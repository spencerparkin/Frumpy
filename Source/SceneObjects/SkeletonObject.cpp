#include "SkeletonObject.h"
#include "Renderer.h"

using namespace Frumpy;

SkeletonObject::SkeletonObject()
{
	this->skeleton = nullptr;
	this->color.SetComponents(1.0, 1.0, 1.0, 1.0);
	this->vertexBuffer = new std::vector<Vertex>();
}

/*virtual*/ SkeletonObject::~SkeletonObject()
{
	delete this->vertexBuffer;
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

	this->vertexBuffer->clear();

	Matrix4x4 identity;
	this->GenerateVertexBuffer(identity, this->skeleton->rootSpace);

	Matrix4x4 objectToImage = renderer.GetGraphicsMatrices().worldToImage * this->objectToWorld;
	Matrix4x4 objectToCamera = renderer.GetGraphicsMatrices().worldToCamera * this->objectToWorld;

	for (unsigned int i = 0; i < this->vertexBuffer->size(); i++)
	{
		const Vertex& vertex = (*this->vertexBuffer)[i];

		objectToImage.TransformPoint(vertex.objectSpacePoint, vertex.imageSpacePoint);
		objectToCamera.TransformPoint(vertex.objectSpacePoint, vertex.cameraSpacePoint);
	}

	for (unsigned int i = 0; i < this->vertexBuffer->size(); i += 2)
	{
		Renderer::LineRenderJob* job = new Renderer::LineRenderJob();
		job->vertex[0] = &(*this->vertexBuffer)[i];
		job->vertex[1] = &(*this->vertexBuffer)[i + 1];
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

	this->AddVertex(pointA, this->color);
	this->AddVertex(pointB, this->color);

	Vector3 axis = pointB - pointA;
	axis.Normalize();

	Vector3 pointC;
	pointC.Lerp(pointA, pointB, 0.3);

	double radius = (pointB - pointA).Length() / 10.0;
		
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

	for (unsigned int i = 0; i < childSpace->childSpaceArray->size(); i++)
		this->GenerateVertexBuffer(childToObject, (*childSpace->childSpaceArray)[i]);
}

void SkeletonObject::AddVertex(const Vector3& vertexPoint, const Vector4& vertexColor) const
{
	Vertex vertex;
	vertex.objectSpacePoint = vertexPoint;
	vertex.color = vertexColor;
	this->vertexBuffer->push_back(vertex);
}