#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "Plane.h"
#include "List.h"

namespace Frumpy
{
	// Camera space is centered at the origin with the +Y-axis up, +X-axis right, and looking down the -Z-axis.
	class FRUMPY_API Camera
	{
	public:
		Camera();
		virtual ~Camera();

		struct FRUMPY_API Frustum
		{
			Frustum();

			double hfovi;
			double vfovi;
			double _near;
			double _far;

			void AdjustVFoviForAspectRatio(double aspectRatio);
			void AdjustHfoviForAspectRatio(double aspectRatio);

			void GeneratePlanes(List<Plane>& frustumPlanesList) const;
			void CalcProjectionMatrix(Matrix& projectionMatrix) const;
		};

		bool LookAt(const Vector& eyePoint, const Vector& eyeTarget, const Vector& upDirection);

		Frustum frustum;
		Matrix worldTransform;
	};
}