#pragma once

#include "Defines.h"
#include "List.h"
#include "Matrix.h"
#include "LightSources/AmbientLight.h"
#include <mutex>
#include <thread>
#include <semaphore>
#include <atomic>

namespace Frumpy
{
	class Vertex;
	class Image;
	class LightSource;

	class FRUMPY_API Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		bool Startup(unsigned int threadCount);
		void Shutdown();

		class RenderJob;
		class Thread;

		void SubmitJob(RenderJob* job);
		void BeginRenderPass();
		void EndRenderPass();

		class RenderJob
		{
		public:
			RenderJob();
			virtual ~RenderJob();

			virtual void Render(Thread* thread) = 0;
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

			const Vertex* vertex[3];
		};

		void SetFramebuffer(Image* frameBuffer) { this->frameBuffer = frameBuffer; }
		Image* GetFramebuffer() { return this->frameBuffer; }

		void SetDepthBuffer(Image* depthBuffer) { this->depthBuffer = depthBuffer; }
		Image* GetDepthBuffer() { return this->depthBuffer; }

		PipelineMatrices pipelineMatrices;

		class Thread
		{
		public:
			Thread(Renderer* renderer);
			virtual ~Thread();

			void Run();
			void SubmitJob(RenderJob* job);

			List<RenderJob*> renderJobQueue;
			std::mutex renderJobQueueMutex;
			std::counting_semaphore<7000> renderJobQueueSemaphore;
			std::thread* thread;
			bool exitSignaled;
			Renderer* renderer;
			Image* frameBuffer;
			Image* depthBuffer;
		};

		void SetLightSource(LightSource* lightSource) { this->lightSource = lightSource; }
		LightSource* GeLightSource() { return this->lightSource; }

	private:

		void WaitForAllJobCompletion();

		List<Thread*> threadList;
		Image* frameBuffer;
		Image* depthBuffer;
		std::atomic<unsigned int>* jobCount;
		LightSource* lightSource;
		AmbientLight defaultLightSource;
	};
}