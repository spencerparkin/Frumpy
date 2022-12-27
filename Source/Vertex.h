#pragma once

#include "Defines.h"
#include "Vector.h"
#include <string>

namespace Frumpy
{
	class FRUMPY_API Vertex
	{
	public:
		Vertex();
		virtual ~Vertex();

		Vector objectSpacePoint;
		Vector objectSpaceNormal;
		Vector color;
		Vector texCoords;

		mutable Vector cameraSpacePoint;
		mutable Vector imageSpacePoint;
		mutable Vector cameraSpaceNormal;

		std::string MakeKey() const;
	};
}