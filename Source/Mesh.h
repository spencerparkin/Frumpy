#pragma once

#include "Scene.h"
#include "Vector.h"

namespace Frumpy
{
	class Vertex;
	class Image;

	// These are triangle meshes.
	class FRUMPY_API Mesh : public Scene::Object
	{
	public:
		Mesh();
		virtual ~Mesh();

		// TODO: Add load/save for .OBJ file format?

		virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const override;
		virtual void Render(const Matrix& cameraMatrix, const Matrix& projectionMatrix, Image& image, Image& depthBuffer) const override;

		void SetVertexBufferSize(unsigned int vertexBufferSize);
		unsigned int GetVertexBufferSize() const;

		void SetIndexBufferSize(unsigned int indexBufferSize);
		unsigned int GetIndexBufferSize() const;

	protected:
		Vertex* vertexBuffer;
		unsigned int vertexBufferSize;

		unsigned int* indexBuffer;
		unsigned int indexBufferSize;

		// TODO: Point here to any associated texture.
	};
}