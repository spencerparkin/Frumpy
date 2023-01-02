#include "Renderer.h"
#include "Vector.h"
#include "FileAssets/Image.h"
#include "LightSource.h"
#include "Vertex.h"
#include "Triangle.h"
#include "Plane.h"
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

	// Evenly distribute the scan-line work-load across all the threads.
	unsigned int scanlinesPerThread = this->frameBuffer->GetHeight() / this->threadList.GetCount();
	unsigned int minScanline = 0;
	unsigned int maxScanline = scanlinesPerThread - 1;
	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		thread->minScanline = minScanline;
		thread->maxScanline = node->GetNext() ? maxScanline : (this->frameBuffer->GetHeight() - 1);
		minScanline = maxScanline + 1;
		maxScanline = minScanline + scanlinesPerThread - 1;
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
	Matrix parentToWorld;
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
			scene->GenerateVisibleObjectsList(&shadowCamera, visibleObjectList);
			this->renderPass = RenderPass::SHADOW_PASS;
			pixel.depth = -(float)shadowCamera.frustum._far;
			this->shadowBuffer->Clear(pixel);

			for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
			{
				const Scene::Object* object = node->value;
				if (object->castsShadow)
					object->Render(*this);
			}

			this->WaitForAllJobCompletion();
		}
	}

	visibleObjectList.Clear();
	this->graphicsMatrices.Calculate(*camera, *this->frameBuffer);
	scene->GenerateVisibleObjectsList(camera, visibleObjectList);

	// Clear buffers and get the light source ready for lighting calculations.
	pixel.depth = -(float)camera->frustum._far;
	this->depthBuffer->Clear(pixel);
	this->frameBuffer->Clear(this->clearPixel);
	this->lightSource->PrepareForRender(this->graphicsMatrices);

	// Render all the opaque stuff first.
	this->renderPass = RenderPass::OPAQUE_PASS;
	for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Scene::Object* object = node->value;
		if (object->renderType == Scene::Object::RenderType::RENDER_OPAQUE)
			object->Render(*this);
	}
	
	this->WaitForAllJobCompletion();

	// Now render all the transparent stuff so that it can blend with what's already in the frame-buffer.
	this->renderPass = RenderPass::TRANSPARENT_PASS;
	for (Scene::ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Scene::Object* object = node->value;
		if (object->renderType == Scene::Object::RenderType::RENDER_TRANSPARENT)
			object->Render(*this);
	}
	
	this->WaitForAllJobCompletion();

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

void Renderer::SetClearColor(const Vector& clearColor)
{
	this->clearPixel.color = this->frameBuffer->MakeColor(clearColor);
}

//-------------------------- Renderer::RenderJob --------------------------

Renderer::RenderJob::RenderJob()
{
}

/*virtual*/ Renderer::RenderJob::~RenderJob()
{
}

/*virtual*/ bool Renderer::RenderJob::ShouldRenderOnThread(Thread* thread)
{
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

	int minRow = int(minY);
	int maxRow = int(maxY);

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

/*virtual*/ void Renderer::TriangleRenderJob::Render(Thread* thread)
{
	const GraphicsMatrices& graphicsMatrices = thread->renderer->graphicsMatrices;

	Image* frameBuffer = thread->renderer->frameBuffer;
	Image* depthBuffer = thread->renderer->depthBuffer;
	Image* shadowBuffer = thread->renderer->shadowBuffer;

	const Vertex& vertexA = *this->vertex[0];
	const Vertex& vertexB = *this->vertex[1];
	const Vertex& vertexC = *this->vertex[2];

	struct Edge
	{
		const Vector* vertexA;
		const Vector* vertexB;
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

	int minRow = int(minY);
	int maxRow = int(maxY);
	int midRow = int(midY);

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

		int minCol = int(minX);
		int maxCol = int(maxX);

		minCol = FRUMPY_MAX(minCol, 0);
		maxCol = FRUMPY_MIN(maxCol, int(frameBuffer->GetWidth() - 1));

		for (int col = minCol; col <= maxCol; col++)
		{
			Image::Location location{ unsigned(row), unsigned(col) };

			Vector imagePointA(double(col), double(row), 0.0);
			Vector imagePointB(double(col), double(row), -1.0);

			Vector cameraPointA, cameraPointB;

			graphicsMatrices.imageToCamera.TransformPoint(imagePointA, cameraPointA);
			graphicsMatrices.imageToCamera.TransformPoint(imagePointB, cameraPointB);

			Vector rayDirection = cameraPointB - cameraPointA;
			double lambda = 0.0;
			planeOfTriangle.RayCast(cameraPointA, rayDirection, lambda);
			Vector trianglePoint = cameraPointA + rayDirection * lambda;

			Image::Pixel* pixelZ = depthBuffer->GetPixel(location);
			if (trianglePoint.z <= pixelZ->depth)	// TODO: Depth-testing optional?
				continue;
			
			pixelZ->depth = (float)trianglePoint.z;

			Vector baryCoords;
			if (!triangle.CalcBarycentricCoordinates(trianglePoint, baryCoords))
				continue;

			LightSource::SurfaceProperties surfaceProperties;

			if (this->texture)
			{
				Vector interpolatedTexCoords =
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

			surfaceProperties.normal =
				vertexA.cameraSpaceNormal * baryCoords.x +
				vertexB.cameraSpaceNormal * baryCoords.y +
				vertexC.cameraSpaceNormal * baryCoords.z;

			surfaceProperties.normal.Normalize();

			Vector surfaceColor;
			thread->renderer->lightSource->CalcSurfaceColor(surfaceProperties, surfaceColor);

			// TODO: Support alpha blending.  We'll have to do an opaque pass then an alpha pass.
			uint32_t color = frameBuffer->MakeColor(surfaceColor);
			frameBuffer->SetPixel(location, color);
		}
	}
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