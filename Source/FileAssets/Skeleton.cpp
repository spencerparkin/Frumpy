#include "Skeleton.h"

using namespace Frumpy;

Skeleton::Skeleton()
{
	this->rootSpace = nullptr;
}

/*virtual*/ Skeleton::~Skeleton()
{
	delete this->rootSpace;
}

Skeleton::BoneSpace::BoneSpace()
{
	this->subSpaceArray = nullptr;
	this->subSpaceArraySize = 0;
}

/*virtual*/ Skeleton::BoneSpace::~BoneSpace()
{
	for (unsigned int i = 0; i < this->subSpaceArraySize; i++)
		delete this->subSpaceArray[i];
}

void Skeleton::BoneSpace::ResolveBindPoseTransforms(const BoneSpace* parentSpace /*= nullptr*/) const
{
	this->ResolveTransforms(parentSpace, &BoneSpace::bindPoseChildToParent, &BoneSpace::bindPoseObjectToBone, &BoneSpace::bindPoseBoneToObject);
}

void Skeleton::BoneSpace::ResolveCurrentPoseTransforms(const BoneSpace* parentSpace /*= nullptr*/) const
{
	this->ResolveTransforms(parentSpace, &BoneSpace::currentPoseChildToParent, &BoneSpace::currentPoseObjectToBone, &BoneSpace::currentPoseBoneToObject);
}

void Skeleton::BoneSpace::ResolveTransforms(const BoneSpace* parentSpace, Matrix4x4 BoneSpace::* childToParent, Matrix4x4 BoneSpace::* objectToBone, Matrix4x4 BoneSpace::* boneToObject) const
{
	Matrix4x4 objectToParent;
	if (!parentSpace)
		objectToParent.Identity();
	else
		objectToParent = parentSpace->*objectToBone;

	*const_cast<Matrix4x4*>(&(this->*objectToBone)) = objectToParent * this->*childToParent;
	const_cast<Matrix4x4*>(&(this->*boneToObject))->Invert(this->*objectToBone);

	for (unsigned int i = 0; i < this->subSpaceArraySize; i++)
		this->subSpaceArray[i]->ResolveTransforms(this, childToParent, objectToBone, boneToObject);
}