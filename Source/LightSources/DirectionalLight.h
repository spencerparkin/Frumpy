#pragma once

#include "LightSource.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"

namespace Frumpy
{
	class FRUMPY_API DirectionalLight : public LightSource
	{
	public:
		DirectionalLight();
		virtual ~DirectionalLight();

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector4& surfaceColor, const Image* shadowBuffer) const override;

		Frumpy::Vector3 worldSpaceDirection;
		mutable Frumpy::Vector3 cameraSpaceDirection;
	};
}