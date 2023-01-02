#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "Vector.h"

namespace Frumpy
{
	class Camera;
	struct GraphicsMatrices;

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

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const = 0;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const = 0;
		virtual bool CalcShadowCamera(Camera& shadowCamera) const;

		double mainIntensity;
		double ambientIntensity;
	};
}