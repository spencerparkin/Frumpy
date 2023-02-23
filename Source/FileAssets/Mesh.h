#pragma once

#include "AssetManager.h"
#include "Math/Vector3.h"
#include "Vertex.h"

namespace Frumpy
{
	class Renderer;
	class Matrix4x4;

	// These are always triangle meshes.  I should probably rename the class to be more specific.
	class FRUMPY_API Mesh : public AssetManager::Asset
	{
	public:
		Mesh();
		virtual ~Mesh();

		virtual void TransformMeshVertices(Renderer& renderer, const Matrix4x4& objectToWorld) const;
		virtual Vertex* AllocVertex();
		virtual void DeallocVertex(Vertex* vertex);

		void SetVertexBufferSize(unsigned int vertexBufferSize);
		unsigned int GetVertexBufferSize() const;

		void SetIndexBufferSize(unsigned int indexBufferSize);
		unsigned int GetIndexBufferSize() const;

		Vertex* GetVertex(unsigned int i);
		const Vertex* GetVertex(unsigned int i) const;

		unsigned int GetIndex(unsigned int i) const;
		bool SetIndex(unsigned int i, unsigned int index);

		void SetColor(const Vector4& color);

		unsigned int* GetRawIndexBuffer() { return this->indexBuffer; }
		const unsigned int* GetRawIndexBuffer() const { return this->indexBuffer; }

	protected:
		Vertex** vertexBuffer;
		unsigned int vertexBufferSize;

		unsigned int* indexBuffer;
		unsigned int indexBufferSize;
	};
}