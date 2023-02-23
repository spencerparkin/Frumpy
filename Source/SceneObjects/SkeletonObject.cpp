#include "SkeletonObject.h"

using namespace Frumpy;

SkeletonObject::SkeletonObject()
{
	this->skeleton = nullptr;
	this->color.SetComponents(1.0, 1.0, 1.0, 1.0);
}

/*virtual*/ SkeletonObject::~SkeletonObject()
{
}

/*virtual*/ bool SkeletonObject::IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const
{
	return true;
}

/*virtual*/ void SkeletonObject::Render(Renderer& renderer, const Camera* camera) const
{
	// TODO: Send line render jobs to the renderer.
}