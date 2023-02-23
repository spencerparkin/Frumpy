#include "Mesh.h"
#include "Renderer.h"

using namespace Frumpy;

Mesh::Mesh()
{
	this->vertexBuffer = nullptr;
	this->vertexBufferSize = 0;
	this->indexBuffer = nullptr;
	this->indexBufferSize = 0;
}

/*virtual*/ Mesh::~Mesh()
{
	this->SetVertexBufferSize(0);
	this->SetIndexBufferSize(0);
}

/*virtual*/ Vertex* Mesh::AllocVertex()
{
	return new Vertex();
}

/*virtual*/ void Mesh::DeallocVertex(Vertex* vertex)
{
	delete vertex;
}

/*virtual*/ void Mesh::TransformMeshVertices(Renderer& renderer, const Matrix4x4& objectToWorld) const
{
	Matrix4x4 objectToImage = renderer.GetGraphicsMatrices().worldToImage * objectToWorld;
	Matrix4x4 objectToCamera = renderer.GetGraphicsMatrices().worldToCamera * objectToWorld;

	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
	{
		const Vertex& vertex = *this->vertexBuffer[i];

		objectToImage.TransformPoint(vertex.objectSpacePoint, vertex.imageSpacePoint);
		objectToCamera.TransformPoint(vertex.objectSpacePoint, vertex.cameraSpacePoint);
		objectToCamera.TransformVector(vertex.objectSpaceNormal, vertex.cameraSpaceNormal);
	}
}

void Mesh::SetVertexBufferSize(unsigned int vertexBufferSize)
{
	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
		this->DeallocVertex(this->vertexBuffer[i]);

	delete[] this->vertexBuffer;
	this->vertexBufferSize = 0;

	if (vertexBufferSize > 0)
	{
		this->vertexBuffer = new Vertex*[vertexBufferSize];
		this->vertexBufferSize = vertexBufferSize;

		for (unsigned int i = 0; i < this->vertexBufferSize; i++)
			this->vertexBuffer[i] = this->AllocVertex();
	}
}

unsigned int Mesh::GetVertexBufferSize() const
{
	return this->vertexBufferSize;
}

void Mesh::SetIndexBufferSize(unsigned int indexBufferSize)
{
	delete[] this->indexBuffer;
	this->indexBufferSize = indexBufferSize;

	if (indexBufferSize > 0)
	{
		this->indexBuffer = new unsigned int[indexBufferSize];
		this->indexBufferSize = indexBufferSize;
	}
}

unsigned int Mesh::GetIndexBufferSize() const
{
	return this->indexBufferSize;
}

Vertex* Mesh::GetVertex(unsigned int i)
{
	if (i >= this->vertexBufferSize)
		return nullptr;

	return this->vertexBuffer[i];
}

const Vertex* Mesh::GetVertex(unsigned int i) const
{
	return const_cast<Mesh*>(this)->GetVertex(i);
}

unsigned int Mesh::GetIndex(unsigned int i) const
{
	if (i >= this->indexBufferSize)
		return -1;

	return this->indexBuffer[i];
}

bool Mesh::SetIndex(unsigned int i, unsigned int index)
{
	if (i >= this->indexBufferSize)
		return false;

	this->indexBuffer[i] = index;
	return true;
}

void Mesh::SetColor(const Vector4& color)
{
	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
		this->vertexBuffer[i]->color = color;
}