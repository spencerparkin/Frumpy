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
			double near;
			double far;

			void GeneratePlanes(List<Plane>& frustumPlanesList) const;
			void CalcProjectionMatrix(Matrix& projectionMatrix) const;
		};

		Frustum frustum;
		Matrix worldTransform;
	};
}