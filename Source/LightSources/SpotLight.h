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

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor, const Image* shadowBuffer) const override;
		virtual bool CalcShadowCamera(Camera& shadowCamera) const override;

		Frumpy::Vector worldSpaceLocation;
		Frumpy::Vector worldSpaceDirection;

		double innerConeAngle;
		double outerConeAngle;
		//double attenuationRadius;		TODO: Support this.  For now, the radius is infinite.

		mutable Frumpy::Vector cameraSpaceLocation;
		mutable Frumpy::Vector cameraSpaceDirection;

		mutable Frumpy::Vector shadowMapXAxis;
		mutable Frumpy::Vector shadowMapYAxis;
		mutable Frumpy::Vector shadowMapZAxis;

		mutable double shadowMapExtent;
	};
}