#include "Renderer.h"
#include "Math/Vector3.h"
#include "FileAssets/Image.h"
#include "LightSource.h"
#include "Vertex.h"
#include "Math/Triangle.h"
#include "Math/Plane.h"
#include "Scene.h"
#include "Camera.h"
#include <math.h>

using namespace Frumpy;

Renderer::Renderer()
{
	this->frameBuffer = nullptr;
	this->depthBuffer = nullptr;
	this->shadowBuffer = nullptr;
	this->jobCount = new std::atomic<unsigned int>(0);
	this->lightSource = &this->defaultLightSource;
	this->renderPass = RenderPass::UNKNOWN_PASS;
	this->clearPixel.color = 0;
	this->totalRenderJobsPerformedPerFrame = 0;
}

/*virtual*/ Renderer::~Renderer()
{
	delete this->jobCount;
}

bool Renderer::Startup(unsigned int threadCount)
{
	if (!this->frameBuffer || !this->depthBuffer)
		return false;

	if (this->frameBuffer->GetWidth() != this->depthBuffer->GetWidth())
		return false;

	if (this->frameBuffer->GetHeight() != this->depthBuffer->GetHeight())
		return false;

	// Launch our worker threads that do all the shading.
	for (unsigned int i = 0; i < threadCount; i++)
	{
		Thread* thread = new Thread(this);
		this->threadList.AddTail(thread);
	}

	return true;
}

void Renderer::Shutdown()
{
	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		thread->SubmitJob(new ExitRenderJob());
	}

	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		thread->thread->join();
		delete thread;
	}

	this->threadList.Clear();
}

void Renderer::DistributeWorkloadForImage(const Image* image)
{
	// Evenly distribute the scan-line work-load across all the threads.
	unsigned int scanlinesPerThread = image->GetHeight() / this->threadList.GetCount();
	unsigned int minScanline = 0;
	unsigned int maxScanline = scanlinesPerThread - 1;
	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		thread->minScanline = minScanline;
		thread->maxScanline = node->GetNext() ? maxScanline : (image->GetHeight() - 1);
		minScanline = maxScanline + 1;
		maxScanline = minScanline + scanlinesPerThread - 1;
	}
}

void GraphicsMatrices::Calculate(const Camera& camera, const Image& target)
{
	camera.frustum.CalcProjectionMatrix(this->cameraToProjection);
	this->worldToCamera.Invert(camera.worldTransform);
	target.CalcImageMatrix(this->projectionToImage);
	this->worldToImage = this->projectionToImage * this->cameraToProjection * this->worldToCamera;
	this->imageToCamera.Invert(this->projectionToImage * this->cameraToProjection);
}

void Renderer::RenderScene(const Scene* scene, const Camera* camera)
{
	// First, resolve the object-to-world transform for each object in the scene hierarchy.
	Matrix4x4 parentToWorld;
	parentToWorld.Identity();
	for (const Scene::ObjectList::Node* node = scene->objectList.GetHead(); node; node = node->GetNext())
	{
		const Scene::Object* object = node->value;
		object->CalculateWorldTransform(parentToWorld);
	}

	Scene::ObjectList visibleObjectList;
	Image::Pixel pixel;

	// Render the scene from the perspective of the light source into the shadow buffer.
	// The shadow buffer can then be used in the remainder of the render to determine
	// if a shaded pixel is in light or shadow.
	if (this->shadowBuffer && this->lightSource)
	{
		Camera shadowCamera;
		if (this->lightSource->CalcShadowCamera(shadowCamera))
		{
			this->graphicsMatrices.Calculate(shadowCamera, *this->shadowBuffer);
			scene->GenerateVisibleObjectsList(&shadowCamera, visibleObjectList, this->graphicsMatrices.worldToCamera);
			this->renderPass = RenderPass::SHADOW_PASS;
			pixel.depth = -(float)shadowCamera.frustum._far;
			this->shadowBuffer->Clear(pixel);
			this->DistributeWorkloadForImage(this->shadowBuffer);

			for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
			{
				const Scene::Object* object = node->value;
				if (object->GetRenderFlag(FRUMPY_RENDER_FLAG_CASTS_SHADOW))
					object->Render(*this, &shadowCamera);
			}

			this->WaitForAllJobCompletion();
		}
	}

	visibleObjectList.Clear();
	this->graphicsMatrices.Calculate(*camera, *this->frameBuffer);
	scene->GenerateVisibleObjectsList(camera, visibleObjectList, this->graphicsMatrices.worldToCamera);

	// Clear buffers and get the light source ready for lighting calculations.
	pixel.depth = -(float)camera->frustum._far;
	this->depthBuffer->Clear(pixel);
	this->frameBuffer->Clear(this->clearPixel);
	this->lightSource->PrepareForRender(this->graphicsMatrices);

	// Render all the opaque stuff first.
	this->DistributeWorkloadForImage(this->frameBuffer);
	this->renderPass = RenderPass::OPAQUE_PASS;
	for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Scene::Object* object = node->value;
		if (!object->GetRenderFlag(FRUMPY_RENDER_FLAG_HAS_TRANSLUCENCY))
			object->Render(*this, camera);
	}
	
	this->WaitForAllJobCompletion();

	// Now render all the transparent stuff so that it can blend with what's already in the frame-buffer.
	this->renderPass = RenderPass::TRANSPARENT_PASS;
	for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Scene::Object* object = node->value;
		if (object->GetRenderFlag(FRUMPY_RENDER_FLAG_HAS_TRANSLUCENCY))
			object->Render(*this, camera);
	}
	
	this->WaitForAllJobCompletion();

	this->totalRenderJobsPerformedPerFrame = this->submittedJobsList.GetCount();
	this->submittedJobsList.Delete();
}

void Renderer::SubmitJob(RenderJob* job)
{
	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		if (job->ShouldRenderOnThread(thread))
			thread->SubmitJob(job);
	}

	this->submittedJobsList.AddTail(job);
}

void Renderer::WaitForAllJobCompletion()
{
	// TODO: How can we do this without busy-waiting?
	// First, wait for all jobs to complete.
	while (*this->jobCount > 0)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1ms);
	}
}

void Renderer::SetClearColor(const Vector3& clearColor)
{
	this->clearPixel.color = this->frameBuffer->MakeColor(clearColor);
}

//-------------------------- Renderer::RenderJob --------------------------

Renderer::RenderJob::RenderJob()
{
	this->renderFlags = 0;
}

/*virtual*/ Renderer::RenderJob::~RenderJob()
{
}

/*virtual*/ bool Renderer::RenderJob::ShouldRenderOnThread(Thread* thread)
{
	return false;
}

bool Renderer::RenderJob::ThreadOverlapsRange(int minRow, int maxRow, Thread* thread)
{
	if (int(thread->minScanline) <= minRow && minRow <= int(thread->maxScanline))
		return true;

	if (int(thread->minScanline) <= maxRow && maxRow <= int(thread->maxScanline))
		return true;

	if (minRow <= int(thread->minScanline) && int(thread->minScanline) <= maxRow)
		return true;

	if (minRow <= int(thread->maxScanline) && int(thread->maxScanline) <= maxRow)
		return true;

	return false;
}

bool Renderer::RenderJob::InterpolateSurfacePointOfPlanarPrimitive(int row, int col, const Matrix4x4& imageToCamera, const Plane& hitPlane, Vector3& surfacePoint)
{
	Vector3 imagePointA(double(col), double(row), 0.0);
	Vector3 imagePointB(double(col), double(row), -1.0);

	Vector3 cameraPointA, cameraPointB;

	imageToCamera.TransformPoint(imagePointA, cameraPointA);
	imageToCamera.TransformPoint(imagePointB, cameraPointB);

	Vector3 rayDirection = cameraPointB - cameraPointA;	// I'm not sure why we can't just use cameraPointA = <0,0,0>.
	double lambda = 0.0;
	if (hitPlane.RayCast(cameraPointA, rayDirection, lambda))
	{
		surfacePoint = cameraPointA + rayDirection * lambda;
		return true;
	}
	
	return false;
}

//-------------------------- Renderer::ExitRenderJob --------------------------

Renderer::ExitRenderJob::ExitRenderJob()
{
}

/*virtual*/ Renderer::ExitRenderJob::~ExitRenderJob()
{
}

/*virtual*/ void Renderer::ExitRenderJob::Render(Thread* thread)
{
	thread->exitSignaled = true;
}

//-------------------------- Renderer::TriangleRenderJob --------------------------

Renderer::TriangleRenderJob::TriangleRenderJob()
{
	this->vertex[0] = nullptr;
	this->vertex[1] = nullptr;
	this->vertex[2] = nullptr;
	this->texture = nullptr;
	this->sampleMethod = Image::SampleMethod::NEAREST;
}

/*virtual*/ Renderer::TriangleRenderJob::~TriangleRenderJob()
{
}

/*virtual*/ bool Renderer::TriangleRenderJob::ShouldRenderOnThread(Thread* thread)
{
	const Vertex& vertexA = *this->vertex[0];
	const Vertex& vertexB = *this->vertex[1];
	const Vertex& vertexC = *this->vertex[2];

	double minY = FRUMPY_MIN(FRUMPY_MIN(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);
	double maxY = FRUMPY_MAX(FRUMPY_MAX(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);

	return this->ThreadOverlapsRange(int(minY), int(maxY), thread);
}

/*virtual*/ void Renderer::TriangleRenderJob::Render(Thread* thread)
{
	const GraphicsMatrices& graphicsMatrices = thread->renderer->graphicsMatrices;

	Image* frameBuffer = nullptr;
	Image* depthBuffer = nullptr;

	if (thread->renderer->renderPass == RenderPass::OPAQUE_PASS || thread->renderer->renderPass == RenderPass::TRANSPARENT_PASS)
	{
		frameBuffer = thread->renderer->frameBuffer;
		depthBuffer = thread->renderer->depthBuffer;
	}
	else if (thread->renderer->renderPass == RenderPass::SHADOW_PASS)
	{
		frameBuffer = thread->renderer->shadowBuffer;
		depthBuffer = thread->renderer->shadowBuffer;
	}
	else
		return;

	const Vertex& vertexA = *this->vertex[0];
	const Vertex& vertexB = *this->vertex[1];
	const Vertex& vertexC = *this->vertex[2];

	struct Edge
	{
		const Vector3* vertexA;
		const Vector3* vertexB;
	};

	Edge minEdges[2];
	Edge maxEdges[2];

	double minY = FRUMPY_MIN(FRUMPY_MIN(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);
	double maxY = FRUMPY_MAX(FRUMPY_MAX(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);

	double midY = 0.0;

	if (minY == vertexA.imageSpacePoint.y)
	{
		if (maxY == vertexB.imageSpacePoint.y)
			midY = vertexC.imageSpacePoint.y;
		else
			midY = vertexB.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexA.imageSpacePoint;
		minEdges[0].vertexB = &vertexB.imageSpacePoint;
		minEdges[1].vertexA = &vertexA.imageSpacePoint;
		minEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else if (minY == vertexB.imageSpacePoint.y)
	{
		if (maxY == vertexA.imageSpacePoint.y)
			midY = vertexC.imageSpacePoint.y;
		else
			midY = vertexA.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexB.imageSpacePoint;
		minEdges[0].vertexB = &vertexA.imageSpacePoint;
		minEdges[1].vertexA = &vertexB.imageSpacePoint;
		minEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else
	{
		if (maxY == vertexA.imageSpacePoint.y)
			midY = vertexB.imageSpacePoint.y;
		else
			midY = vertexA.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexC.imageSpacePoint;
		minEdges[0].vertexB = &vertexA.imageSpacePoint;
		minEdges[1].vertexA = &vertexC.imageSpacePoint;
		minEdges[1].vertexB = &vertexB.imageSpacePoint;
	}

	if (maxY == vertexA.imageSpacePoint.y)
	{
		maxEdges[0].vertexA = &vertexA.imageSpacePoint;
		maxEdges[0].vertexB = &vertexB.imageSpacePoint;
		maxEdges[1].vertexA = &vertexA.imageSpacePoint;
		maxEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else if (maxY == vertexB.imageSpacePoint.y)
	{
		maxEdges[0].vertexA = &vertexB.imageSpacePoint;
		maxEdges[0].vertexB = &vertexA.imageSpacePoint;
		maxEdges[1].vertexA = &vertexB.imageSpacePoint;
		maxEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else
	{
		maxEdges[0].vertexA = &vertexC.imageSpacePoint;
		maxEdges[0].vertexB = &vertexA.imageSpacePoint;
		maxEdges[1].vertexA = &vertexC.imageSpacePoint;
		maxEdges[1].vertexB = &vertexB.imageSpacePoint;
	}

	int minRow = int(::ceil(minY));
	int maxRow = int(::floor(maxY));
	int midRow = int(::ceil(midY));

	if (double(maxRow) == maxY)
		maxRow--;

	minRow = FRUMPY_CLAMP(minRow, int(thread->minScanline), int(thread->maxScanline));
	maxRow = FRUMPY_CLAMP(maxRow, int(thread->minScanline), int(thread->maxScanline));

	Triangle triangle;
	triangle.vertex[0] = vertexA.cameraSpacePoint;
	triangle.vertex[1] = vertexB.cameraSpacePoint;
	triangle.vertex[2] = vertexC.cameraSpacePoint;

	Plane planeOfTriangle;
	planeOfTriangle.SetFromTriangle(triangle);

	for (int row = minRow; row <= maxRow; row++)
	{
		Edge* edges = (row < midRow) ? minEdges : maxEdges;

		double y = double(row);

		double d0 = edges[0].vertexA->y - edges[0].vertexB->y;
		//if (d0 == 0.0)
		//	continue;

		double t0 = (y - edges[0].vertexB->y) / d0;
		//if (isinf(t0) || isnan(t0))
		//	continue;

		double d1 = edges[1].vertexA->y - edges[1].vertexB->y;
		//if (d1 == 0.0)
		//	continue;

		double t1 = (y - edges[1].vertexB->y) / d1;
		//if (isinf(t1) || isnan(t1))
		//	continue;

		t0 = FRUMPY_CLAMP(t0, 0.0, 1.0);
		t1 = FRUMPY_CLAMP(t1, 0.0, 1.0);

		double x0 = edges[0].vertexA->x * t0 + edges[0].vertexB->x * (1.0 - t0);
		double x1 = edges[1].vertexA->x * t1 + edges[1].vertexB->x * (1.0 - t1);

		double minX = FRUMPY_MIN(x0, x1);
		double maxX = FRUMPY_MAX(x0, x1);

		int minCol = int(::ceil(minX));
		int maxCol = int(::floor(maxX));

		if (double(maxCol) == maxX)
			maxCol--;

		minCol = FRUMPY_MAX(minCol, 0);
		maxCol = FRUMPY_MIN(maxCol, int(frameBuffer->GetWidth() - 1));

		for (int col = minCol; col <= maxCol; col++)
		{
			LightSource::SurfaceProperties surfaceProperties;
			if(!this->InterpolateSurfacePointOfPlanarPrimitive(row, col, graphicsMatrices.imageToCamera, planeOfTriangle, surfaceProperties.cameraSpacePoint))
				surfaceProperties.cameraSpacePoint = (vertexA.cameraSpacePoint + vertexB.cameraSpacePoint + vertexC.cameraSpacePoint) / 3.0;

			Image::Location location{ unsigned(row), unsigned(col) };
			Image::Pixel* pixelZ = depthBuffer->GetPixel(location);
			if ((0 != (this->renderFlags & FRUMPY_RENDER_FLAG_DEPTH_TEST)) && surfaceProperties.cameraSpacePoint.z <= pixelZ->depth)
				continue;
			
			pixelZ->depth = (float)surfaceProperties.cameraSpacePoint.z;

			if (thread->renderer->renderPass == RenderPass::SHADOW_PASS)
				continue;

			Vector3 baryCoords;
			if (!triangle.CalcBarycentricCoordinates(surfaceProperties.cameraSpacePoint, baryCoords))
				continue;

			if (this->texture)
			{
				Vector3 interpolatedTexCoords =
					vertexA.texCoords * baryCoords.x +
					vertexB.texCoords * baryCoords.y +
					vertexC.texCoords * baryCoords.z;

				interpolatedTexCoords.x = FRUMPY_CLAMP(interpolatedTexCoords.x, 0.0, 1.0);
				interpolatedTexCoords.y = FRUMPY_CLAMP(interpolatedTexCoords.y, 0.0, 1.0);
				interpolatedTexCoords.z = FRUMPY_CLAMP(interpolatedTexCoords.z, 0.0, 1.0);

				texture->SampleColorVector(surfaceProperties.diffuseColor, interpolatedTexCoords, this->sampleMethod);
			}
			else
			{
				surfaceProperties.diffuseColor =
					vertexA.color * baryCoords.x +
					vertexB.color * baryCoords.y +
					vertexC.color * baryCoords.z;
			}

			surfaceProperties.cameraSpaceNormal =
				vertexA.cameraSpaceNormal * baryCoords.x +
				vertexB.cameraSpaceNormal * baryCoords.y +
				vertexC.cameraSpaceNormal * baryCoords.z;

			surfaceProperties.cameraSpaceNormal.Normalize();

			Vector4 surfaceColor;

			if (0 != (this->renderFlags & FRUMPY_RENDER_FLAG_IS_LIT))
				thread->renderer->lightSource->CalcSurfaceColor(surfaceProperties, surfaceColor, (0 != (this->renderFlags & FRUMPY_RENDER_FLAG_CAN_BE_SHADOWED)) ? thread->renderer->shadowBuffer : nullptr);
			else
				surfaceColor = surfaceProperties.diffuseColor;

			uint32_t color = 0;

			if (thread->renderer->renderPass == RenderPass::TRANSPARENT_PASS)
			{
				Vector4 existingColor;
				frameBuffer->GetPixel(location)->MakeColorVector(existingColor, frameBuffer);

				Vector4 blentColor;
				blentColor.r = existingColor.r * (1.0 - surfaceColor.a) + surfaceColor.r * surfaceColor.a;
				blentColor.g = existingColor.g * (1.0 - surfaceColor.a) + surfaceColor.g * surfaceColor.a;
				blentColor.b = existingColor.b * (1.0 - surfaceColor.a) + surfaceColor.b * surfaceColor.a;
				blentColor.a = 0.0;

				color = frameBuffer->MakeColor(blentColor);
			}
			else
			{
				color = frameBuffer->MakeColor(surfaceColor);
			}

			frameBuffer->SetPixel(location, color);
		}
	}
}

//-------------------------- Renderer::LineRenderJob --------------------------

Renderer::LineRenderJob::LineRenderJob()
{
	this->vertex[0] = nullptr;
	this->vertex[1] = nullptr;
}

/*virtual*/ Renderer::LineRenderJob::~LineRenderJob()
{
}

/*virtual*/ bool Renderer::LineRenderJob::ShouldRenderOnThread(Thread* thread)
{
	const Vertex& vertexA = *this->vertex[0];
	const Vertex& vertexB = *this->vertex[1];

	double minY = FRUMPY_MIN(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y);
	double maxY = FRUMPY_MAX(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y);

	return this->ThreadOverlapsRange(int(minY), int(maxY), thread);
}

/*virtual*/ void Renderer::LineRenderJob::Render(Thread* thread)
{
	const GraphicsMatrices& graphicsMatrices = thread->renderer->graphicsMatrices;

	Image* frameBuffer = nullptr;
	Image* depthBuffer = nullptr;

	if (thread->renderer->renderPass == RenderPass::OPAQUE_PASS || thread->renderer->renderPass == RenderPass::TRANSPARENT_PASS)
	{
		frameBuffer = thread->renderer->frameBuffer;
		depthBuffer = thread->renderer->depthBuffer;
	}
	else if (thread->renderer->renderPass == RenderPass::SHADOW_PASS)
	{
		frameBuffer = thread->renderer->shadowBuffer;
		depthBuffer = thread->renderer->shadowBuffer;
	}
	else
		return;

	const Vertex& vertexA = *this->vertex[0];
	const Vertex& vertexB = *this->vertex[1];

	Vector3 vectorA = vertexB.cameraSpacePoint - vertexA.cameraSpacePoint;
	double lengthA = vectorA.Length();
	Vector3 vectorB;
	vectorB.Cross(Vector3(0.0, 0.0, 1.0), vectorA);
	double lengthB = vectorB.Length();
	if (lengthB == 0.0)
		return;
	vectorB *= lengthA / lengthB;

	Triangle triangle;
	triangle.vertex[0] = vertexA.cameraSpacePoint;
	triangle.vertex[1] = vertexB.cameraSpacePoint;
	triangle.vertex[2] = vertexA.cameraSpacePoint + vectorB;

	Plane planeOfLine;
	planeOfLine.SetFromTriangle(triangle);

	double minY = FRUMPY_MIN(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y);
	double maxY = FRUMPY_MAX(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y);

	double minX = FRUMPY_MIN(vertexA.imageSpacePoint.x, vertexB.imageSpacePoint.x);
	double maxX = FRUMPY_MAX(vertexA.imageSpacePoint.x, vertexB.imageSpacePoint.x);

	if (maxX - minX > maxY - minY)
	{
		int minCol = int(::ceil(minX));
		int maxCol = int(::floor(maxX));

		if (double(maxCol) == maxX)
			maxCol--;

		minCol = FRUMPY_MAX(minCol, 0);
		maxCol = FRUMPY_MIN(maxCol, int(frameBuffer->GetWidth() - 1));

		for (int col = minCol; col <= maxCol; col++)
		{
			double x = double(col);
			double tb = (x - vertexA.imageSpacePoint.x) / (vertexB.imageSpacePoint.x - vertexA.imageSpacePoint.x);
			double ta = 1.0 - tb;
			double y = vertexA.imageSpacePoint.y * ta + vertexB.imageSpacePoint.y * tb;

			int row = int(::round(y));

			if (int(thread->minScanline) <= row && row <= int(thread->maxScanline))
			{
				this->Rasterize(row, col, vertexA, vertexB, ta, tb, graphicsMatrices.imageToCamera, planeOfLine, frameBuffer, depthBuffer);
			}
		}
	}
	else
	{
		int minRow = int(::ceil(minY));
		int maxRow = int(::floor(maxY));

		if (double(maxRow) == maxY)
			maxRow--;

		minRow = FRUMPY_CLAMP(minRow, int(thread->minScanline), int(thread->maxScanline));
		maxRow = FRUMPY_CLAMP(maxRow, int(thread->minScanline), int(thread->maxScanline));

		for (int row = minRow; row <= maxRow; row++)
		{
			double y = double(row);
			double tb = (y - vertexA.imageSpacePoint.y) / (vertexB.imageSpacePoint.y - vertexA.imageSpacePoint.y);
			double ta = 1.0 - tb;
			double x = vertexA.imageSpacePoint.x * ta + vertexB.imageSpacePoint.x * tb;

			int col = int(::round(x));

			if (0 <= col && col < int(frameBuffer->GetWidth()))
			{
				this->Rasterize(row, col, vertexA, vertexB, ta, tb, graphicsMatrices.imageToCamera, planeOfLine, frameBuffer, depthBuffer);
			}
		}
	}
}

void Renderer::LineRenderJob::Rasterize(int row, int col, const Vertex& vertexA, const Vertex& vertexB, double ta, double tb,
								const Matrix4x4& imageToCamera, const Plane& planeOfLine, Image* frameBuffer, Image* depthBuffer)
{
	Vector3 surfacePoint;
	if (!this->InterpolateSurfacePointOfPlanarPrimitive(row, col, imageToCamera, planeOfLine, surfacePoint))
		surfacePoint = (vertexA.cameraSpacePoint + vertexB.cameraSpacePoint) / 2.0;

	Image::Location location{ unsigned(row), unsigned(col) };
	Image::Pixel* pixelZ = depthBuffer->GetPixel(location);
	if ((0 != (this->renderFlags & FRUMPY_RENDER_FLAG_DEPTH_TEST)) && surfacePoint.z <= pixelZ->depth)
		return;

	Vector4 surfaceColor = vertexA.color * ta + vertexB.color * tb;
	uint32_t color = frameBuffer->MakeColor(surfaceColor);
	frameBuffer->SetPixel(location, color);
}

//-------------------------- Renderer::Thread --------------------------

Renderer::Thread::Thread(Renderer* renderer) : renderJobQueueSemaphore(0)
{
	this->renderer = renderer;
	this->exitSignaled = false;
	this->minScanline = 0;
	this->maxScanline = 0;
	this->thread = new std::thread([=]() { this->Run(); });
}

/*virtual*/ Renderer::Thread::~Thread()
{
	delete this->thread;
}

void Renderer::Thread::SubmitJob(RenderJob* job)
{
	(*this->renderer->jobCount)++;
	this->renderJobQueueMutex.lock();
	this->renderJobQueue.AddTail(job);
	this->renderJobQueueMutex.unlock();
	this->renderJobQueueSemaphore.release();
}

void Renderer::Thread::Run()
{
	while (!this->exitSignaled)
	{
		this->renderJobQueueSemaphore.acquire();

		if (renderJobQueue.GetCount() > 0)
		{
			// Lock only long enough to grab the next job off the queue.
			this->renderJobQueueMutex.lock();
			List<RenderJob*>::Node* node = this->renderJobQueue.GetHead();
			RenderJob* renderJob = node->value;
			this->renderJobQueue.Remove(node);
			this->renderJobQueueMutex.unlock();

			// Process the job.
			renderJob->Render(this);
			(*this->renderer->jobCount)--;
		}
	}
}