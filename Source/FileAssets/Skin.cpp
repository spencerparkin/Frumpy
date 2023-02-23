#include "Skin.h"
#include "Renderer.h"

using namespace Frumpy;

Skin::Skin()
{
}

/*virtual*/ Skin::~Skin()
{
}

/*virtual*/ Vertex* Skin::AllocVertex()
{
	return new PosedVertex();
}

/*virtual*/ void Skin::TransformMeshVertices(Renderer& renderer, const Matrix4x4& objectToWorld) const
{
	if (!this->skeleton)
		return;

	this->skeleton->rootSpace->ResolveCurrentPoseTransforms();

	Matrix4x4 objectToImage = renderer.GetGraphicsMatrices().worldToImage * objectToWorld;
	Matrix4x4 objectToCamera = renderer.GetGraphicsMatrices().worldToCamera * objectToWorld;

	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
	{
		const PosedVertex* vertex = (const PosedVertex*)this->vertexBuffer[i];

		vertex->poseSpacePoint.SetComponents(0.0, 0.0, 0.0);

		for (unsigned int j = 0; j < vertex->boneWeightArraySize; j++)
		{
			const BoneWeight& boneWeight = vertex->boneWeightArray[j];

			Vector3 boneSpacePoint;
			boneWeight.boneSpace->bindPoseObjectToBone.TransformPoint(vertex->objectSpacePoint, boneSpacePoint);

			Vector3 objectSpacePoint;
			boneWeight.boneSpace->currentPoseBoneToObject.TransformPoint(boneSpacePoint, objectSpacePoint);

			vertex->poseSpacePoint += objectSpacePoint * boneWeight.weight;
		}

		objectToImage.TransformPoint(vertex->poseSpacePoint, vertex->imageSpacePoint);
		objectToCamera.TransformPoint(vertex->poseSpacePoint, vertex->cameraSpacePoint);
		objectToCamera.TransformVector(vertex->poseSpaceNormal, vertex->cameraSpaceNormal);
	}
}

Skin::PosedVertex::PosedVertex()
{
	this->boneWeightArray = nullptr;
	this->boneWeightArraySize = 0;
}

/*virtual*/ Skin::PosedVertex::~PosedVertex()
{
	delete[] this->boneWeightArray;
}