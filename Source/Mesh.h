#pragma once

#include "Scene.h"
#include "Vector.h"
#include "FileFormat.h"

namespace Frumpy
{
	class Vertex;
	class Image;

	// These are triangle meshes.
	class FRUMPY_API Mesh : public Scene::Object, public FileFormat::Asset
	{
	public:
		Mesh();
		virtual ~Mesh();

		virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const override;
		virtual void Render(Renderer& renderer) const override;

		void SetVertexBufferSize(unsigned int vertexBufferSize);
		unsigned int GetVertexBufferSize() const;

		void SetIndexBufferSize(unsigned int indexBufferSize);
		unsigned int GetIndexBufferSize() const;

		Vertex* GetVertex(unsigned int i);
		const Vertex* GetVertex(unsigned int i) const;

		unsigned int GetIndex(unsigned int i) const;
		bool SetIndex(unsigned int i, unsigned int index);

	protected:
		Vertex* vertexBuffer;
		unsigned int vertexBufferSize;

		unsigned int* indexBuffer;
		unsigned int indexBufferSize;

		// TODO: Point here to any associated texture.  Or material?  This would combine textures with shaders.
	};
}