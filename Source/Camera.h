#pragma once

#include "Defines.h"
#include "Math/Matrix4x4.h"
#include "Math/Plane.h"
#include "Containers/List.h"
#include "Math/ConvexHull.h"

namespace Frumpy
{
	class FRUMPY_API Frustum
	{
	public:
		Frustum();
		virtual ~Frustum();

		double hfovi;
		double vfovi;
		double _near;
		double _far;

		void AdjustVFoviForAspectRatio(double aspectRatio);
		void AdjustHfoviForAspectRatio(double aspectRatio);

		void CalcProjectionMatrix(Matrix4x4& projectionMatrix) const;

		const ConvexHull& GetFrustumHull() const;

	private:

		mutable ConvexHull frustumHull;
		mutable bool frustumHullValid;
	};

	// Camera space is centered at the origin with the +Y-axis up, +X-axis right, and looking down the -Z-axis.
	class FRUMPY_API Camera
	{
	public:
		Camera();
		virtual ~Camera();

		bool LookAt(const Vector3& eyePoint, const Vector3& eyeTarget, const Vector3& upDirection);

		Matrix4x4 worldTransform;
		Frustum frustum;
	};
}