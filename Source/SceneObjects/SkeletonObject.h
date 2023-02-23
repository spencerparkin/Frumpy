#pragma once

#include "Scene.h"
#include "Math/Vector4.h"

namespace Frumpy
{
	class Skeleton;

	class FRUMPY_API SkeletonObject : public Scene::Object
	{
	public:
		SkeletonObject();
		virtual ~SkeletonObject();

		virtual bool IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const override;
		virtual void Render(Renderer& renderer, const Camera* camera) const override;

		void SetSkeleton(Skeleton* skeleton) { this->skeleton = skeleton; }
		Skeleton* GetSkeleton() { return this->skeleton; }

		void SetColor(const Vector4& color) { this->color; }
		const Vector4& GetColor() const { return this->color; }

	private:

		Skeleton* skeleton;

		Vector4 color;
	};
}