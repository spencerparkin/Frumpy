#include "Mesh.h"
#include "Vertex.h"

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

/*virtual*/ bool Mesh::IntersectsFrustum(const List<Plane>& frustumPlanesList) const
{
	// TODO: Base this on an axis-aligned bounding box combined with our world transform?
	return true;
}

/*virtual*/ void Mesh::Render(const Matrix& cameraMatrix, const Matrix& projectionMatrix, Image& image, Image& depthBuffer) const
{
	Matrix viewMatrix = cameraMatrix * this->worldTransform;
	Matrix transformMatrix = projectionMatrix * viewMatrix;

	for (unsigned int i = 0; i < this->indexBufferSize; i += 3)
	{
		unsigned int indexA = this->indexBuffer[i + 0];
		unsigned int indexB = this->indexBuffer[i + 1];
		unsigned int indexC = this->indexBuffer[i + 2];

		if (indexA >= this->vertexBufferSize ||
			indexB >= this->vertexBufferSize ||
			indexC >= this->vertexBufferSize)
		{
			continue;
		}

		const Vertex& vertexA = this->vertexBuffer[indexA];
		const Vertex& vertexB = this->vertexBuffer[indexB];
		const Vertex& vertexC = this->vertexBuffer[indexC];

		Vector pointA, pointB, pointC;

		viewMatrix.TransformPoint(vertexA.point, pointA);
		viewMatrix.TransformPoint(vertexB.point, pointB);
		viewMatrix.TransformPoint(vertexC.point, pointC);

		// Perform back-face culling.
		Vector triangleNorm;
		triangleNorm.Cross(pointB - pointA, pointC - pointA);
		if (triangleNorm.z < 0.0)
			continue;

		image.RenderTriangle(vertexA, vertexB, vertexC, transformMatrix, depthBuffer);
	}
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