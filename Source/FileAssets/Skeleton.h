#pragma once

#include "Defines.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"

namespace Frumpy
{
	// These are hierarchies of transforms or bone spaces that can be
	// used to manipulate a triangle mesh that has vertrex weights.
	// Each vertex is weighted to one or more bones.  The skeleton can
	// be driven or posed by an animation each frame over time, or
	// manipulated manually by user code.
	class FRUMPY_API Skeleton
	{
	public:
		Skeleton();
		virtual ~Skeleton();

		class BoneSpace
		{
		public:
			BoneSpace();
			virtual ~BoneSpace();

			void ResolveBindPoseTransforms(const BoneSpace* parentSpace = nullptr) const;
			void ResolveCurrentPoseTransforms(const BoneSpace* parentSpace = nullptr) const;

			void ResolveTransforms(const BoneSpace* parentSpace, Matrix4x4 BoneSpace::* childToParent, Matrix4x4 BoneSpace::* objectToBone, Matrix4x4 BoneSpace::* boneToObject) const;

			Matrix4x4 bindPoseChildToParent;
			Matrix4x4 currentPoseChildToParent;

			mutable Matrix4x4 bindPoseObjectToBone;
			mutable Matrix4x4 bindPoseBoneToObject;

			mutable Matrix4x4 currentPoseObjectToBone;
			mutable Matrix4x4 currentPoseBoneToObject;

			BoneSpace** subSpaceArray;
			unsigned int subSpaceArraySize;
		};

		BoneSpace* rootSpace;
	};
}