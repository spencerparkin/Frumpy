#pragma once

#include "../LightSource.h"
#include "../Vector.h"

namespace Frumpy
{
	class FRUMPY_API SpotLight : public LightSource
	{
	public:
		SpotLight();
		virtual ~SpotLight();

		virtual void PrepareForRender(const PipelineMatrices& pipelineMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const override;

		Frumpy::Vector locationWorldSpace;
		Frumpy::Vector directionWorldSpace;
		double attenuationRadius;

		mutable Frumpy::Vector locationCameraSpace;
		mutable Frumpy::Vector directionCameraSpace;
	};
}