#pragma once

#include "../AssetManager.h"
#include "../Vector.h"
#include "../Vertex.h"

namespace Frumpy
{
	class FRUMPY_API Mesh : public AssetManager::Asset
	{
	public:
		Mesh();
		virtual ~Mesh();

		void SetVertexBufferSize(unsigned int vertexBufferSize);
		unsigned int GetVertexBufferSize() const;

		void SetIndexBufferSize(unsigned int indexBufferSize);
		unsigned int GetIndexBufferSize() const;

		Vertex* GetVertex(unsigned int i);
		const Vertex* GetVertex(unsigned int i) const;

		unsigned int GetIndex(unsigned int i) const;
		bool SetIndex(unsigned int i, unsigned int index);

		void SetColor(const Vector& color);

	protected:
		Vertex* vertexBuffer;
		unsigned int vertexBufferSize;

		unsigned int* indexBuffer;
		unsigned int indexBufferSize;
	};
}