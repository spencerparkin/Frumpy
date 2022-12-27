#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "Vector.h"

namespace Frumpy
{
	class FRUMPY_API LightSource
	{
	public:
		LightSource();
		virtual ~LightSource();

		struct SurfaceProperties
		{
			Vector diffuseColor;
			Vector normal;
		};

		virtual void PrepareForRender(const PipelineMatrices& pipelineMatrices) const = 0;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const = 0;

		double mainIntensity;
		double ambientIntensity;
	};
}