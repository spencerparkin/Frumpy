#pragma once

#include "Defines.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"

namespace Frumpy
{
	class Camera;
	class Image;
	struct GraphicsMatrices;

	class FRUMPY_API LightSource
	{
	public:
		LightSource();
		virtual ~LightSource();

		struct SurfaceProperties
		{
			Vector3 diffuseColor;
			Vector3 cameraSpacePoint;
			Vector3 cameraSpaceNormal;
		};

		virtual void PrepareForRender(const GraphicsMatrices& graphicsMatrices) const = 0;
		virtual void CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector3& surfaceColor, const Image* shadowBuffer) const = 0;
		virtual bool CalcShadowCamera(Camera& shadowCamera) const;

		double mainIntensity;
		double ambientIntensity;
	};
}