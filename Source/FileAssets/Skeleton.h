#pragma once

#include "../Defines.h"
#include "../Math/Matrix4x4.h"

namespace Frumpy
{
	// These are hierarchies of transforms or bone spaces that can be
	// used to manipulate a triangle mesh that has vertrex weights.
	// Each vertex is weighted to one or more bones.  The skeleton can
	// be driven or posed by an animation each frame over time, or
	// manipulated manually by user code.
	class FRUMPY_API Skeleton
	{
	public:
		Skeleton();
		virtual ~Skeleton();

		// TODO: How do we define this?
	};
}