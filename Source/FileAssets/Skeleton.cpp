#include "Skeleton.h"
#include "Containers/List.h"
#include <string.h>

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
	this->name[0] = '\0';
	this->childSpaceArray = new std::vector<BoneSpace*>();
}

void Skeleton::BoneSpace::SetName(const char* givenName)
{
	strcpy_s(this->name, sizeof(this->name), givenName);
}

/*virtual*/ Skeleton::BoneSpace::~BoneSpace()
{
	for (unsigned int i = 0; i < this->childSpaceArray->size(); i++)
		delete (*this->childSpaceArray)[i];

	delete this->childSpaceArray;
}

void Skeleton::BoneSpace::ResolveBindPoseTransforms(const BoneSpace* parentSpace /*= nullptr*/) const
{
	const_cast<BoneSpace*>(this)->ResolveTransforms(parentSpace, &BoneSpace::bindPoseChildToParent, &BoneSpace::bindPoseObjectToBone, &BoneSpace::bindPoseBoneToObject);
}

void Skeleton::BoneSpace::ResolveCurrentPoseTransforms(const BoneSpace* parentSpace /*= nullptr*/) const
{
	const_cast<BoneSpace*>(this)->ResolveTransforms(parentSpace, &BoneSpace::currentPoseChildToParent, &BoneSpace::currentPoseObjectToBone, &BoneSpace::currentPoseBoneToObject);
}

void Skeleton::BoneSpace::ResolveTransforms(const BoneSpace* parentSpace, Matrix4x4 BoneSpace::* childToParent, Matrix4x4 BoneSpace::* objectToBone, Matrix4x4 BoneSpace::* boneToObject)
{
	Matrix4x4 parentToObject;
	if (!parentSpace)
		parentToObject.Identity();
	else
		parentToObject = parentSpace->*boneToObject;

	this->*boneToObject = parentToObject * this->*childToParent;
	(this->*objectToBone).Invert(this->*boneToObject);

	for (unsigned int i = 0; i < this->childSpaceArray->size(); i++)
		(*this->childSpaceArray)[i]->ResolveTransforms(this, childToParent, objectToBone, boneToObject);
}

void Skeleton::BoneSpace::ResetToBindPose()
{
	this->currentPoseChildToParent = this->bindPoseChildToParent;

	for (unsigned int i = 0; i < this->childSpaceArray->size(); i++)
		(*this->childSpaceArray)[i]->ResetToBindPose();
}

Skeleton::BoneSpace* Skeleton::FindBoneSpaceByName(const char* givenName)
{
	if (!this->rootSpace)
		return nullptr;

	List<BoneSpace*> boneSpaceQueue;
	boneSpaceQueue.AddTail(this->rootSpace);

	while (boneSpaceQueue.GetCount() > 0)
	{
		List<BoneSpace*>::Node* node = boneSpaceQueue.GetHead();
		BoneSpace* boneSpace = node->value;
		boneSpaceQueue.Remove(node);

		if (0 == strcmp(boneSpace->name, givenName))
			return boneSpace;

		for (unsigned int i = 0; i < boneSpace->childSpaceArray->size(); i++)
			boneSpaceQueue.AddTail((*boneSpace->childSpaceArray)[i]);
	}

	return nullptr;
}