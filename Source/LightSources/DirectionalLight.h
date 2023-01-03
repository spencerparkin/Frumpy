#pragma once

#include "../LightSource.h"
#include "../Vector.h"

namespace Frumpy
{
	class FRUMPY_API DirectionalLight : public LightSource
	{
	public:
		DirectionalLight();
		virtual ~DirectionalLight();

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const override;

		Frumpy::Vector worldSpaceDirection;
		mutable Frumpy::Vector cameraSpaceDirection;
	};
}