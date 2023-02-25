#pragma once

#include "Defines.h"
#include "Renderer.h"

namespace Frumpy
{
	class FRUMPY_API VertexShader
	{
	public:
		VertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices);
		virtual ~VertexShader();

		virtual void ProcessVertex(const void* inputVertex, Renderer::Vertex* outputVertex) = 0;

		Matrix4x4 objectToImage;
		Matrix4x4 objectToCamera;
	};

	// TODO: This class is not yet used, but conceivably it could be used by the low-level renderer to customize shading across triangles.
	class FRUMPY_API PixelShader
	{
	public:
		PixelShader();
		virtual ~PixelShader();

		// TODO: Pin this down.
		virtual void ProcessPixel(const void* inputPixel, const void* outputPixel) = 0;
	};
}