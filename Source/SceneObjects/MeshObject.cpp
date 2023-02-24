#include "MeshObject.h"
#include "Renderer.h"
#include "FileAssets/Mesh.h"
#include "Math/Triangle.h"
#include "Math/Aabb.h"
#include "Math/Edge.h"
#include "Camera.h"

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

void MeshObject::SetColor(const Vector4& color)
{
	if (this->mesh)
	{
		this->mesh->SetColor(color);
		this->SetRenderFlag(FRUMPY_RENDER_FLAG_HAS_TRANSLUCENCY, color.a != 0.0);
	}
}

/*virtual*/ bool MeshObject::IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const
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

	Matrix4x4 objectToCamera = worldToCamera * this->objectToWorld;

	ConvexHull cameraSpaceBoundingHull(this->objectSpaceBoundingHull);
	cameraSpaceBoundingHull.Transform(objectToCamera);

	return cameraSpaceBoundingHull.OverlapsWith(frustumHull);
}

/*virtual*/ void MeshObject::Render(Renderer& renderer, const Camera* camera) const
{
	if (!this->mesh)
		return;

	this->mesh->TransformMeshVertices(renderer, this->objectToWorld);

	std::set<Edge>* edgeSet = nullptr;
	if (0 != (this->renderFlags & FRUMPY_RENDER_FLAG_WIRE_FRAME))
		edgeSet = new std::set<Edge>();

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

		const Vector3& pointA = vertexA.imageSpacePoint;
		const Vector3& pointB = vertexB.imageSpacePoint;
		const Vector3& pointC = vertexC.imageSpacePoint;

		// Perform back-face culling in image space.
		Vector3 triangleNorm;
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
		if (0 == (this->renderFlags & FRUMPY_RENDER_FLAG_WIRE_FRAME))
		{
			Renderer::TriangleRenderJob* job = new Renderer::TriangleRenderJob();
			job->vertex[0] = &vertexA;
			job->vertex[1] = &vertexB;
			job->vertex[2] = &vertexC;
			job->texture = this->texture;
			job->sampleMethod = this->sampleMethod;
			job->renderFlags = this->renderFlags;
			renderer.SubmitJob(job);
		}
		else
		{
			Edge edgeArray[3];

			edgeArray[0].i = indexA;
			edgeArray[0].j = indexB;

			edgeArray[1].i = indexB;
			edgeArray[1].j = indexC;

			edgeArray[2].i = indexC;
			edgeArray[2].j = indexA;

			for (int j = 0; j < 3; j++)
			{
				const Edge& edge = edgeArray[j];

				if (edgeSet->end() == edgeSet->find(edge))
				{
					Renderer::LineRenderJob* job = new Renderer::LineRenderJob();
					job->vertex[0] = this->mesh->GetVertex(edge.i);
					job->vertex[1] = this->mesh->GetVertex(edge.j);
					job->renderFlags = this->renderFlags;
					renderer.SubmitJob(job);

					edgeSet->insert(edge);
				}
			}
		}
	}

	delete edgeSet;
}