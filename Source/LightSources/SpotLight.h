#pragma once

#include "../LightSource.h"
#include "../Vector3.h"

namespace Frumpy
{
	class FRUMPY_API SpotLight : public LightSource
	{
	public:
		SpotLight();
		virtual ~SpotLight();

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const override;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector3& surfaceColor, const Image* shadowBuffer) const override;
		virtual bool CalcShadowCamera(Camera& shadowCamera) const override;

		Frumpy::Vector3 worldSpaceLocation;
		Frumpy::Vector3 worldSpaceDirection;

		double innerConeAngle;
		double outerConeAngle;
		//double attenuationRadius;		TODO: Support this.  For now, the radius is infinite.

		mutable Frumpy::Vector3 cameraSpaceLocation;
		mutable Frumpy::Vector3 cameraSpaceDirection;

		mutable Frumpy::Vector3 shadowMapXAxis;
		mutable Frumpy::Vector3 shadowMapYAxis;
		mutable Frumpy::Vector3 shadowMapZAxis;

		mutable double shadowMapExtent;
	};
}