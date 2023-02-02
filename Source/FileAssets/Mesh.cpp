#include "Mesh.h"

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

void Mesh::SetVertexBufferSize(unsigned int vertexBufferSize)
{
	delete[] this->vertexBuffer;
	this->vertexBufferSize = 0;

	if (vertexBufferSize > 0)
	{
		this->vertexBuffer = new Vertex[vertexBufferSize];
		this->vertexBufferSize = vertexBufferSize;
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

	return &this->vertexBuffer[i];
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

void Mesh::SetColor(const Vector3& color)
{
	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
		this->vertexBuffer[i].color = color;
}