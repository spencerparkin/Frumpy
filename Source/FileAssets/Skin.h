#pragma once

#include "Mesh.h"
#include "Math/Vector3.h"
#include "Skeleton.h"

namespace Frumpy
{
	// Skins are meshes where each vertex has one or more associated bone weights.
	// Also, the procedure of getting the mesh from object to world space requires,
	// not just the object-to-world transform, but also a skeleton in a current pose.
	class FRUMPY_API Skin : public Mesh
	{
	public:
		Skin();
		virtual ~Skin();

		virtual void TransformMeshVertices(Renderer& renderer, const Matrix4x4& objectToWorld) const override;
		virtual Vertex* AllocVertex() override;

		void SetSkeleton(Skeleton* skeleton) { this->skeleton = skeleton; }
		Skeleton* GetSkeleton() { return this->skeleton; }

	protected:

		Skeleton* skeleton;

		struct BoneWeight
		{
			Skeleton::BoneSpace* boneSpace;
			double weight;
		};

		class PosedVertex : public Vertex
		{
		public:
			PosedVertex();
			virtual ~PosedVertex();

			BoneWeight* boneWeightArray;
			unsigned int boneWeightArraySize;

			mutable Vector3 poseSpacePoint;
			mutable Vector3 poseSpaceNormal;
		};
	};
}