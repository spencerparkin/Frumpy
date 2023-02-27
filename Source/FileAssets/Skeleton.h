#pragma once

#include "Defines.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include <vector>
#include <functional>

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

		class FRUMPY_API BoneSpace
		{
		public:
			BoneSpace();
			virtual ~BoneSpace();

			void ResetToBindPose();

			void ResolveBindPoseTransforms(const BoneSpace* parentSpace = nullptr) const;
			void ResolveCurrentPoseTransforms(const BoneSpace* parentSpace = nullptr) const;

			void ResolveTransforms(const BoneSpace* parentSpace, Matrix4x4 BoneSpace::* childToParent, Matrix4x4 BoneSpace::* objectToBone, Matrix4x4 BoneSpace::* boneToObject);

			void SetName(const char* givenName);
			const char* GetName() const { return this->name; }

			Matrix4x4 bindPoseChildToParent;
			Matrix4x4 currentPoseChildToParent;

			mutable Matrix4x4 bindPoseObjectToBone;
			mutable Matrix4x4 bindPoseBoneToObject;

			mutable Matrix4x4 currentPoseObjectToBone;
			mutable Matrix4x4 currentPoseBoneToObject;

			std::vector<BoneSpace*>* childSpaceArray;

			char name[128];
		};

		void ForAllBoneSpaces(std::function<bool(BoneSpace*)> lambda);

		BoneSpace* FindBoneSpaceByName(const char* givenName);

		BoneSpace* rootSpace;
	};
}