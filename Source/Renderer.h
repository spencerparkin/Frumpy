#pragma once

#include "Defines.h"
#include "List.h"
#include "Matrix.h"
#include <mutex>
#include <thread>
#include <semaphore>
#include <atomic>

namespace Frumpy
{
	class Vertex;
	class Image;

	class FRUMPY_API Renderer
	{
		friend class Thread;

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

		void SetImage(Image* image) { this->image = image; }
		Image* GetImage() { return this->image; }

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
			std::counting_semaphore<1000> renderJobQueueSemaphore;
			std::thread* thread;
			bool exitSignaled;
			Renderer* renderer;
		};

	private:

		List<Thread*> threadList;
		Image* image;
		Image* depthBuffer;
		std::atomic<unsigned int>* jobCount;
	};
}