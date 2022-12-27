#pragma once

#include "../LightSource.h"
#include "../Vector.h"

namespace Frumpy
{
	class FRUMPY_API AmbientLight : public LightSource
	{
	public:
		AmbientLight();
		virtual ~AmbientLight();

		virtual void PrepareForRender(const PipelineMatrices& pipelineMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const override;
	};
}