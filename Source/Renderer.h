#pragma once

#include "Defines.h"
#include "Containers/List.h"
#include "Math/Matrix4x4.h"
#include "LightSources/AmbientLight.h"
#include "FileAssets/Image.h"
#include <mutex>
#include <thread>
#include <semaphore>
#include <atomic>

namespace Frumpy
{
	class Vertex;
	class Image;
	class LightSource;
	class Scene;
	class Plane;

	struct GraphicsMatrices
	{
		void Calculate(const Camera& camera, const Image& target);

		Matrix4x4 worldToCamera;
		Matrix4x4 cameraToProjection;
		Matrix4x4 projectionToImage;
		Matrix4x4 worldToImage;
		Matrix4x4 imageToCamera;
	};

	// Here we essentially support a fixed-function pipeline, but conceivably, we could
	// support a kind of vertex and pixel shader abstraction and call that here across the
	// given vertex buffers and interpolated primitives.  I'm fine without this kind of
	// abstraction for now, though.
	class FRUMPY_API Renderer
	{
	private:
		class Thread;

	public:
		Renderer();
		virtual ~Renderer();

		bool Startup(unsigned int threadCount);
		void Shutdown();
		void RenderScene(const Scene* scene, const Camera* camera);

		void SetLightSource(LightSource* lightSource) { this->lightSource = lightSource; }
		LightSource* GetLightSource() { return this->lightSource; }

		void SetFramebuffer(Image* frameBuffer) { this->frameBuffer = frameBuffer; }
		Image* GetFramebuffer() { return this->frameBuffer; }

		void SetDepthBuffer(Image* depthBuffer) { this->depthBuffer = depthBuffer; }
		Image* GetDepthBuffer() { return this->depthBuffer; }

		void SetShadowBuffer(Image* shadowBuffer) { this->shadowBuffer = shadowBuffer; }
		Image* GetShadowBuffer() { return this->shadowBuffer; }

		void SetClearColor(const Vector3& clearColor);

		const GraphicsMatrices& GetGraphicsMatrices() const { return this->graphicsMatrices; }

		class RenderJob;

		void SubmitJob(RenderJob* job);

		class RenderJob
		{
		public:
			RenderJob();
			virtual ~RenderJob();

			virtual void Render(Thread* thread) = 0;
			virtual bool ShouldRenderOnThread(Thread* thread);

			bool ThreadOverlapsRange(int minRow, int maxRow, Thread* thread);
			bool InterpolateSurfacePointOfPlanarPrimitive(int row, int col, const Matrix4x4& imageToCamera, const Plane& hitPlane, Vector3& surfacePoint);

			uint32_t renderFlags;
		};

		class ExitRenderJob : public RenderJob
		{
		public:
			ExitRenderJob();
			virtual ~ExitRenderJob();

			virtual void Render(Thread* thread) override;
		};

		class TriangleRenderJob : public RenderJob
		{
		public:
			TriangleRenderJob();
			virtual ~TriangleRenderJob();

			virtual void Render(Thread* thread) override;
			virtual bool ShouldRenderOnThread(Thread* thread) override;

			const Vertex* vertex[3];
			const Image* texture;
			Image::SampleMethod sampleMethod;
		};

		class LineRenderJob : public RenderJob
		{
		public:
			LineRenderJob();
			virtual ~LineRenderJob();

			virtual void Render(Thread* thread) override;
			virtual bool ShouldRenderOnThread(Thread* thread) override;

			void Rasterize(int row, int col, const Vertex& vertexA, const Vertex& vertexB, double ta, double tb,
							const Matrix4x4& imageToCamera, const Plane& planeOfLine, Image* frameBuffer, Image* depthBuffer);

			const Vertex* vertex[2];
		};

		enum RenderPass
		{
			UNKNOWN_PASS,
			OPAQUE_PASS,
			TRANSPARENT_PASS,
			SHADOW_PASS
		};

		RenderPass GetRenderPass() { return this->renderPass; }

		unsigned int GetThreadCount() { return this->threadList.GetCount(); }

		uint32_t totalRenderJobsPerformedPerFrame;

	private:

		class Thread
		{
		public:
			Thread(Renderer* renderer);
			virtual ~Thread();

			void Run();
			void SubmitJob(RenderJob* job);

			List<RenderJob*> renderJobQueue;
			std::mutex renderJobQueueMutex;
			std::counting_semaphore<10000> renderJobQueueSemaphore;
			std::thread* thread;
			bool exitSignaled;
			Renderer* renderer;
			unsigned int minScanline;
			unsigned int maxScanline;
		};

		void DistributeWorkloadForImage(const Image* image);
		void WaitForAllJobCompletion();

		List<Thread*> threadList;
		Image* frameBuffer;
		Image* depthBuffer;
		Image* shadowBuffer;
		std::atomic<unsigned int>* jobCount;
		LightSource* lightSource;
		AmbientLight defaultLightSource;
		List<RenderJob*> submittedJobsList;
		GraphicsMatrices graphicsMatrices;
		Image::Pixel clearPixel;
		RenderPass renderPass;
	};
}