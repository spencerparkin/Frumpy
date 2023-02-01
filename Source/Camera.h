#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "Plane.h"
#include "List.h"
#include "ConvexHull.h"

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

		void CalcProjectionMatrix(Matrix& projectionMatrix) const;

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

		bool LookAt(const Vector& eyePoint, const Vector& eyeTarget, const Vector& upDirection);

		Matrix worldTransform;
		Frustum frustum;
	};
}