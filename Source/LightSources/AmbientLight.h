#pragma once

#include "LightSource.h"
#include "Math/Vector4.h"

namespace Frumpy
{
	class FRUMPY_API AmbientLight : public LightSource
	{
	public:
		AmbientLight();
		virtual ~AmbientLight();

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector4& surfaceColor, const Image* shadowBuffer) const override;
	};
}