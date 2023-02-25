#include "SkeletonObject.h"
#include "Renderer.h"

using namespace Frumpy;

SkeletonObject::SkeletonObject()
{
	this->skeleton = nullptr;
	this->color.SetComponents(1.0, 1.0, 1.0, 1.0);
}

/*virtual*/ SkeletonObject::~SkeletonObject()
{
}

/*virtual*/ bool SkeletonObject::IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const
{
	return true;
}

/*virtual*/ void SkeletonObject::Render(Renderer& renderer, const Camera* camera) const
{
	if (!this->skeleton)
		return;

	this->skeleton->rootSpace->ResolveCurrentPoseTransforms();

	this->renderVertexBuffer->clear();

	this->objectToImage = renderer.GetGraphicsMatrices().worldToImage * this->objectToWorld;
	this->objectToCamera = renderer.GetGraphicsMatrices().worldToCamera * this->objectToWorld;

	Matrix4x4 identity;
	this->GenerateVertexBuffer(identity, this->skeleton->rootSpace);

	for (unsigned int i = 0; i < this->renderVertexBuffer->size(); i += 2)
	{
		Renderer::LineRenderJob* job = new Renderer::LineRenderJob();
		job->vertex[0] = &(*this->renderVertexBuffer)[i];
		job->vertex[1] = &(*this->renderVertexBuffer)[i + 1];
		job->renderFlags = this->renderFlags;
		renderer.SubmitJob(job);
	}
}

void SkeletonObject::GenerateVertexBuffer(const Matrix4x4& parentToObject, const Skeleton::BoneSpace* childSpace) const
{
	Matrix4x4 childToObject = parentToObject * childSpace->currentPoseChildToParent;

	Vector3 pointA(0.0, 0.0, 0.0);
	Vector3 pointB(0.0, 0.0, 0.0);

	parentToObject.TransformPoint(pointA, pointA);
	childToObject.TransformPoint(pointB, pointB);

	// Bones are only drawn between bone spaces.
	if (childSpace != this->skeleton->rootSpace)
	{
		this->AddVertex(pointA, this->color);
		this->AddVertex(pointB, this->color);

		Vector3 axis = pointB - pointA;
		axis.Normalize();

		Vector3 pointC;
		pointC.Lerp(pointA, pointB, 0.3);

		double length = (pointB - pointA).Length();
		double radius = length / 10.0;

		Vector3 pointArray[4];
		int pointArraySize = sizeof(pointArray) / sizeof(Vector3);
		double angle = 2.0 * FRUMPY_PI / double(pointArraySize);

		pointArray[0].MakeOrthogonalTo(axis);
		pointArray[0].Scale(radius / pointArray[0].Length());
		for (int i = 1; i < pointArraySize; i++)
			pointArray[i].Rotation(pointArray[i - 1], axis, angle);

		for (int i = 0; i < pointArraySize; i++)
			pointArray[i] += pointC;

		for (int i = 0; i < pointArraySize; i++)
		{
			int j = (i + 1) % pointArraySize;

			this->AddVertex(pointArray[i], this->color);
			this->AddVertex(pointArray[j], this->color);

			this->AddVertex(pointArray[i], this->color);
			this->AddVertex(pointA, this->color);

			this->AddVertex(pointArray[i], this->color);
			this->AddVertex(pointB, this->color);
		}
	}

	for (unsigned int i = 0; i < childSpace->childSpaceArray->size(); i++)
		this->GenerateVertexBuffer(childToObject, (*childSpace->childSpaceArray)[i]);
}

void SkeletonObject::AddVertex(const Vector3& vertexPoint, const Vector4& vertexColor) const
{
	Renderer::Vertex vertex;

	this->objectToImage.TransformPoint(vertexPoint, vertex.imageSpacePoint);
	this->objectToCamera.TransformPoint(vertexPoint, vertex.cameraSpacePoint);

	vertex.color = vertexColor;

	this->renderVertexBuffer->push_back(vertex);
}