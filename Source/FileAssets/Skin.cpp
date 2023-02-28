#include "Skin.h"
#include "Renderer.h"
#include <list>

using namespace Frumpy;

Skin::Skin()
{
	this->skeleton = nullptr;
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

// Under almost all circumstances, the results of this subroutine would not be
// favourable over bone weights authored by an artist in a modeling program.
// This routine is here for debugging purposes.
bool Skin::AutoGenerateBoneWeights(unsigned int numBonesPerVertex)
{
	if (!this->skeleton || numBonesPerVertex == 0)
		return false;

	this->skeleton->rootSpace->ResolveBindPoseTransforms();

	struct Bone
	{
		Skeleton::BoneSpace* boneSpace;
		Vector3 origin;
		mutable double distance;
	};

	std::list<Bone> boneList;

	this->skeleton->ForAllBoneSpaces([&boneList](Skeleton::BoneSpace* boneSpace) -> bool {
		Bone bone;
		bone.boneSpace = boneSpace;
		boneSpace->bindPoseBoneToObject.GetCol(3, bone.origin);
		bone.distance = 0.0;
		boneList.push_back(bone);
		return true;
	});

	if (numBonesPerVertex > boneList.size())
		return false;

	for (unsigned int i = 0; i < this->vertexBufferSize; i++)
	{
		PosedVertex* vertex = (PosedVertex*)this->vertexBuffer[i];

		vertex->boneWeightArray.clear();

		for (Bone& bone : boneList)
			bone.distance = (bone.origin - vertex->objectSpacePoint).Length();

		boneList.sort([vertex](const Bone& boneA, const Bone& boneB) -> int {
			return boneA.distance < boneB.distance;
		});

		double totalDistance = 0.0;
		unsigned int j = 0;
		for (Bone& bone : boneList)
		{
			totalDistance += bone.distance;
			if (++j >= numBonesPerVertex)
				break;
		}

		j = 0;
		for (Bone& bone : boneList)
		{
			vertex->boneWeightArray.push_back(BoneWeight{ bone.boneSpace, totalDistance / bone.distance });
			if (++j == numBonesPerVertex)
				break;
		}

		vertex->NormalizeWeights();
	}

	return true;
}

void Skin::PosedVertex::NormalizeWeights()
{
	double totalWeight = 0.0;
	for (unsigned int i = 0; i < this->boneWeightArray.size(); i++)
		totalWeight += this->boneWeightArray[i].weight;

	if (totalWeight != 0.0)
	{
		for (unsigned int i = 0; i < this->boneWeightArray.size(); i++)
			this->boneWeightArray[i].weight /= totalWeight;
	}
	else
	{
		for (unsigned int i = 0; i < this->boneWeightArray.size(); i++)
			this->boneWeightArray[i].weight = 1.0 / double(this->boneWeightArray.size());
	}
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
		const Skin::BoneWeight& boneWeight = posedVertex->boneWeightArray[i];

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

	outputVertex->texCoords = posedVertex->texCoords;
	outputVertex->color = posedVertex->color;
}