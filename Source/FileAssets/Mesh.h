#pragma once

#include "AssetManager.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Shader.h"

namespace Frumpy
{
	class Renderer;
	class Matrix4x4;
	class VertexShader;

	// These are always triangle meshes.  I should probably rename the class to be more specific.
	class FRUMPY_API Mesh : public AssetManager::Asset
	{
	public:
		Mesh();
		virtual ~Mesh();

		struct Vertex;

		virtual Vertex* CreateVertex();
		virtual VertexShader* CreateVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices);
		virtual void BecomeCopyOf(const Mesh* mesh);

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

		struct Vertex
		{
			Vector3 objectSpacePoint;
			Vector3 objectSpaceNormal;
			Vector4 color;
			Vector3 texCoords;
		};

	protected:
		Vertex** vertexBuffer;
		unsigned int vertexBufferSize;

		unsigned int* indexBuffer;
		unsigned int indexBufferSize;
	};

	class MeshVertexShader : public VertexShader
	{
	public:
		MeshVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices);
		virtual ~MeshVertexShader();

		virtual void ProcessVertex(const void* inputVertex, Renderer::Vertex* outputVertex) override;
	};
}