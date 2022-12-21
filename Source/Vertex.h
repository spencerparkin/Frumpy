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

		Vector point;		// TODO: How do we interpolate the depth values with perspective in mind?
		Vector color;		// TODO: How do we interpolate color with perspective in mind?
		Vector texCoords;	// TODO: How do we interpolate these with perspective in mind?

		mutable Vector clipSpacePoint;
	};
}