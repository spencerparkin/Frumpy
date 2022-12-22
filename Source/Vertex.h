#pragma once

#include "Defines.h"
#include "Vector.h"

namespace Frumpy
{
	class FRUMPY_API Vertex
	{
	public:
		Vertex();
		virtual ~Vertex();

		Vector objectSpacePoint;
		Vector color;
		Vector texCoords;

		mutable Vector cameraSpacePoint;
		mutable Vector imageSpacePoint;
	};
}