#pragma once

#include "Defines.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include <string>

namespace Frumpy
{
	class FRUMPY_API Vertex
	{
	public:
		Vertex();
		virtual ~Vertex();

		Vector3 objectSpacePoint;
		Vector3 objectSpaceNormal;
		Vector4 color;
		Vector3 texCoords;

		mutable Vector3 cameraSpacePoint;
		mutable Vector3 imageSpacePoint;
		mutable Vector3 cameraSpaceNormal;

		std::string MakeKey() const;
	};
}