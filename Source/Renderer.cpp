#include "Renderer.h"
#include "Vector.h"
#include "Image.h"
#include "Vertex.h"
#include "Triangle.h"
#include "Plane.h"
#include <math.h>

using namespace Frumpy;

Renderer::Renderer()
{
	this->image = nullptr;
	this->depthBuffer = nullptr;
	this->jobCount = new std::atomic<unsigned int>(0);
}

/*virtual*/ Renderer::~Renderer()
{
	delete this->jobCount;
}

bool Renderer::Startup(unsigned int threadCount)
{
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

void Renderer::SubmitJob(RenderJob* job)
{
	Thread* bestThread = nullptr;
	for (List<Thread*>::Node* node = this->threadList.GetHead(); node; node = node->GetNext())
	{
		Thread* thread = node->value;
		if (!bestThread || bestThread->renderJobQueue.GetCount() > thread->renderJobQueue.GetCount())
			bestThread = thread;
	}

	if (bestThread)
		bestThread->SubmitJob(job);
}

void Renderer::WaitForAllJobCompletion()
{
	// TODO: How can we do this without busy-waiting?
	while (*this->jobCount > 0)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1ms);
	}
}

//-------------------------- Renderer::RenderJob --------------------------

Renderer::RenderJob::RenderJob()
{
}

/*virtual*/ Renderer::RenderJob::~RenderJob()
{
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
}

/*virtual*/ Renderer::TriangleRenderJob::~TriangleRenderJob()
{
}

/*virtual*/ void Renderer::TriangleRenderJob::Render(Thread* thread)
{
	const PipelineMatrices& pipelineMatrices = thread->renderer->pipelineMatrices;

	Image* image = thread->renderer->image;
	Image* depthBuffer = thread->renderer->depthBuffer;
	
	if (!image || !depthBuffer)
		return;

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

	int minRow = int(floor(minY));
	int maxRow = int(ceil(maxY));
	int midRow = int(round(midY));

	minRow = FRUMPY_MAX(minRow, 0);
	maxRow = FRUMPY_MIN(maxRow, signed(image->GetHeight() - 1));

	if (minRow >= maxRow)
		return;

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
		if (d0 == 0.0)
			continue;

		double t0 = (y - edges[0].vertexB->y) / d0;
		if (isinf(t0) || isnan(t0))
			continue;

		double d1 = edges[1].vertexA->y - edges[1].vertexB->y;
		if (d1 == 0.0)
			continue;

		double t1 = (y - edges[1].vertexB->y) / d1;
		if (isinf(t1) || isnan(t1))
			continue;

		t0 = FRUMPY_CLAMP(t0, 0.0, 1.0);
		t1 = FRUMPY_CLAMP(t1, 0.0, 1.0);

		double x0 = edges[0].vertexA->x * t0 + edges[0].vertexB->x * (1.0 - t0);
		double x1 = edges[1].vertexA->x * t1 + edges[1].vertexB->x * (1.0 - t1);

		double minX = FRUMPY_MIN(x0, x1);
		double maxX = FRUMPY_MAX(x0, x1);

		int minCol = int(floor(minX));
		int maxCol = int(ceil(maxX));

		minCol = FRUMPY_MAX(minCol, 0);
		maxCol = FRUMPY_MIN(maxCol, signed(image->GetWidth() - 1));

		for (int col = minCol; col <= maxCol; col++)
		{
			Image::Location location{ unsigned(row), unsigned(col) };

			Vector imagePointA(double(col), double(row), 0.0);
			Vector imagePointB(double(col), double(row), -1.0);

			Vector cameraPointA, cameraPointB;

			pipelineMatrices.imageToCamera.TransformPoint(imagePointA, cameraPointA);
			pipelineMatrices.imageToCamera.TransformPoint(imagePointB, cameraPointB);

			Vector rayDirection = cameraPointB - cameraPointA;
			double lambda = 0.0;
			planeOfTriangle.RayCast(cameraPointA, rayDirection, lambda);
			Vector trianglePoint = cameraPointA + rayDirection * lambda;

			// TODO: How do we do atomic read-compare-writes to the depth buffer?  Would that be too expensive?
			Image::Pixel* pixelZ = depthBuffer->GetPixel(location);
			if (pixelZ && trianglePoint.z < pixelZ->depth)	// TODO: Depth-testing optional?
			{
				pixelZ->depth = (float)trianglePoint.z;
				Image::Pixel* pixel = image->GetPixel(location);

				Vector baryCoords;
				triangle.CalcBarycentricCoordinates(trianglePoint, baryCoords);

				Vector interpolatedColor =
					vertexA.color * baryCoords.x +
					vertexB.color * baryCoords.y +
					vertexC.color * baryCoords.z;

				Vector interpolatedTexCoords =
					vertexA.texCoords * baryCoords.x +
					vertexB.texCoords * baryCoords.y +
					vertexC.texCoords * baryCoords.z;	// TODO: Do something with this.  Will it look correct?

				pixel->color.SetColor(
					unsigned char(interpolatedColor.x * 255.0),
					unsigned char(interpolatedColor.y * 255.0),
					unsigned char(interpolatedColor.z * 255.0),
					0);	// TODO: What about alpha here?
			}
		}
	}
}

//-------------------------- Renderer::Thread --------------------------

Renderer::Thread::Thread(Renderer* renderer) : renderJobQueueSemaphore(0)
{
	this->renderer = renderer;
	this->exitSignaled = false;
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
			delete renderJob;
			(*this->renderer->jobCount)--;
		}
	}
}