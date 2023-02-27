#include "Skin.h"
#include "Renderer.h"

using namespace Frumpy;

Skin::Skin()
{
}

/*virtual*/ Skin::~Skin()
{
}

/*virtual*/ Mesh::Vertex* Skin::CreateVertex()
{
	return new PosedVertex();
}

/*virtual*/ VertexShader* Skin::CreateVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices)
{
	return new SkinVertexShader(objectToWorld, graphicsMatrices);
}

SkinVertexShader::SkinVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices) : VertexShader(objectToWorld, graphicsMatrices)
{
}

/*virtual*/ SkinVertexShader::~SkinVertexShader()
{
}

/*virtual*/ void SkinVertexShader::ProcessVertex(const void* inputVertex, Renderer::Vertex* outputVertex)
{
	const Skin::PosedVertex* posedVertex = (const Skin::PosedVertex*)inputVertex;

	Vector3 poseSpacePoint(0.0, 0.0, 0.0);
	Vector3 poseSpaceNormal(0.0, 0.0, 0.0);

	for (unsigned int i = 0; i < posedVertex->boneWeightArray.size(); i++)
	{
		const Skin::BoneWeight& boneWeight = *posedVertex->boneWeightArray[i];

		Vector3 boneSpacePoint;
		boneWeight.boneSpace->bindPoseObjectToBone.TransformPoint(posedVertex->objectSpacePoint, boneSpacePoint);

		Vector3 objectSpacePoint;
		boneWeight.boneSpace->currentPoseBoneToObject.TransformPoint(boneSpacePoint, objectSpacePoint);

		poseSpacePoint += objectSpacePoint * boneWeight.weight;

		Vector3 boneSpaceNormal;
		boneWeight.boneSpace->bindPoseObjectToBone.TransformVector(posedVertex->objectSpaceNormal, boneSpaceNormal);

		Vector3 objectSpaceNormal;
		boneWeight.boneSpace->currentPoseBoneToObject.TransformVector(boneSpaceNormal, objectSpaceNormal);

		poseSpaceNormal += objectSpaceNormal * boneWeight.weight;
	}

	poseSpaceNormal.Normalize();

	objectToImage.TransformPoint(poseSpacePoint, outputVertex->imageSpacePoint);
	objectToCamera.TransformPoint(poseSpacePoint, outputVertex->cameraSpacePoint);
	objectToCamera.TransformVector(poseSpaceNormal, outputVertex->cameraSpaceNormal);
}