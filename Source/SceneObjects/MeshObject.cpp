#include "MeshObject.h"
#include "../Renderer.h"
#include "../FileAssets/Mesh.h"
#include "../Triangle.h"

using namespace Frumpy;

MeshObject::MeshObject()
{
	this->mesh = nullptr;
	this->texture = nullptr;
	this->sampleMethod = Image::SampleMethod::NEAREST;
}

/*virtual*/ MeshObject::~MeshObject()
{
}

/*virtual*/ bool MeshObject::IntersectsFrustum(const List<Plane>& frustumPlanesList) const
{
	// TODO: Base this on an axis-aligned bounding box combined with our world transform?
	return true;
}

/*virtual*/ void MeshObject::Render(Renderer& renderer) const
{
	if (!this->mesh)
		return;

	Matrix objectToImage = renderer.GetGraphicsMatrices().worldToImage * this->objectToWorld;
	Matrix objectToCamera = renderer.GetGraphicsMatrices().worldToCamera * this->objectToWorld;

	for (unsigned int i = 0; i < this->mesh->GetVertexBufferSize(); i++)
	{
		const Vertex& vertex = *this->mesh->GetVertex(i);
		objectToImage.TransformPoint(vertex.objectSpacePoint, vertex.imageSpacePoint);
		objectToCamera.TransformPoint(vertex.objectSpacePoint, vertex.cameraSpacePoint);
		objectToCamera.TransformVector(vertex.objectSpaceNormal, vertex.cameraSpaceNormal);
	}

	for (unsigned int i = 0; i < this->mesh->GetIndexBufferSize(); i += 3)
	{
		unsigned int indexA = this->mesh->GetIndex(i + 0);
		unsigned int indexB = this->mesh->GetIndex(i + 1);
		unsigned int indexC = this->mesh->GetIndex(i + 2);

		if (indexA >= this->mesh->GetVertexBufferSize() ||
			indexB >= this->mesh->GetVertexBufferSize() ||
			indexC >= this->mesh->GetVertexBufferSize())
		{
			continue;
		}

		const Vertex& vertexA = *this->mesh->GetVertex(indexA);
		const Vertex& vertexB = *this->mesh->GetVertex(indexB);
		const Vertex& vertexC = *this->mesh->GetVertex(indexC);

		const Vector& pointA = vertexA.imageSpacePoint;
		const Vector& pointB = vertexB.imageSpacePoint;
		const Vector& pointC = vertexC.imageSpacePoint;

		// TODO: Frustum-cull individual triangles?

		// Perform back-face culling in image space.
		Vector triangleNorm;
		triangleNorm.Cross(pointB - pointA, pointC - pointA);
		if (triangleNorm.z < 0.0)
			continue;

		// Render by submitting a job to the renderer.
		Renderer::TriangleRenderJob* job = new Renderer::TriangleRenderJob();
		job->vertex[0] = &vertexA;
		job->vertex[1] = &vertexB;
		job->vertex[2] = &vertexC;
		job->texture = this->texture;
		job->sampleMethod = this->sampleMethod;
		job->canBeShadowed = this->canBeShadowed;
		renderer.SubmitJob(job);
	}
}