#include "MeshObject.h"
#include "../Renderer.h"
#include "../FileAssets/Mesh.h"
#include "../Triangle.h"
#include "../Aabb.h"
#include "../Camera.h"

using namespace Frumpy;

MeshObject::MeshObject()
{
	this->mesh = nullptr;
	this->texture = nullptr;
	this->sampleMethod = Image::SampleMethod::NEAREST;
	this->objectSpaceBoundingHullValid = false;
}

/*virtual*/ MeshObject::~MeshObject()
{
}

/*virtual*/ bool MeshObject::IntersectsFrustum(const ConvexHull& frustumHull, const Matrix& worldToCamera) const
{
	if (!this->objectSpaceBoundingHullValid)
	{
		AxisAlignedBoundingBox aabb;
		for (unsigned int i = 0; i < this->mesh->GetVertexBufferSize(); i++)
			aabb.MinimallyExpandToContainPoint(this->mesh->GetVertex(i)->objectSpacePoint);

		if (aabb.Width() < FRUMPY_EPS)
		{
			aabb.min.x -= 0.5;
			aabb.max.x += 0.5;
		}

		if (aabb.Height() < FRUMPY_EPS)
		{
			aabb.min.y -= 0.5;
			aabb.max.y += 0.5;
		}

		if (aabb.Depth() < FRUMPY_EPS)
		{
			aabb.min.z -= 0.5;
			aabb.max.z += 0.5f;
		}

		this->objectSpaceBoundingHull.Generate(aabb);
		this->objectSpaceBoundingHull.RegenerateEdgeSetIfNecessary();
		this->objectSpaceBoundingHullValid = true;
	}

	Matrix objectToCamera = worldToCamera * this->objectToWorld;

	ConvexHull cameraSpaceBoundingHull(this->objectSpaceBoundingHull);
	cameraSpaceBoundingHull.Transform(objectToCamera);

	return cameraSpaceBoundingHull.OverlapsWith(frustumHull);
}

/*virtual*/ void MeshObject::Render(Renderer& renderer, const Camera* camera) const
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

		// Perform back-face culling in image space.
		Vector triangleNorm;
		triangleNorm.Cross(pointB - pointA, pointC - pointA);
		if (triangleNorm.z < 0.0)
			continue;

		// Cull against the camera frustum.  (Clipping is done in image space.)
		const ConvexHull& frustumHull = camera->frustum.GetFrustumHull();
		Triangle triangle;
		triangle.vertex[0] = vertexA.cameraSpacePoint;
		triangle.vertex[1] = vertexB.cameraSpacePoint;
		triangle.vertex[2] = vertexC.cameraSpacePoint;
		if (!frustumHull.OverlapsWith(triangle))
			continue;

		// Render by submitting a job to the renderer.
		Renderer::TriangleRenderJob* job = new Renderer::TriangleRenderJob();
		job->vertex[0] = &vertexA;
		job->vertex[1] = &vertexB;
		job->vertex[2] = &vertexC;
		job->texture = this->texture;
		job->sampleMethod = this->sampleMethod;
		job->canBeShadowed = this->GetRenderFlag(Scene::Object::CAN_BE_SHADOWED);
		renderer.SubmitJob(job);
	}
}