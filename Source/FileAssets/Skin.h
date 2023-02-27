#pragma once

#include "Mesh.h"
#include "Math/Vector3.h"
#include "Skeleton.h"
#include <vector>

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

		virtual Vertex* CreateVertex() override;
		virtual VertexShader* CreateVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices) override;

		void SetSkeleton(Skeleton* skeleton) { this->skeleton = skeleton; }
		Skeleton* GetSkeleton() { return this->skeleton; }

		struct BoneWeight
		{
			Skeleton::BoneSpace* boneSpace;
			double weight;
		};

		struct PosedVertex : public Vertex
		{
			std::vector<BoneWeight*> boneWeightArray;
		};

	protected:

		Skeleton* skeleton;
	};

	class SkinVertexShader : public VertexShader
	{
	public:
		SkinVertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices);
		virtual ~SkinVertexShader();

		virtual void ProcessVertex(const void* inputVertex, Renderer::Vertex* outputVertex) override;
	};
}