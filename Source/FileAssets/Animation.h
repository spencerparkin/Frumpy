#pragma once

#include "../Defines.h"

namespace Frumpy
{
	// Animations are sequences of key-frames.  Each key-frame is a set of bone transforms that
	// can be applied to a compatible skeleton.
	class FRUMPY_API Animation
	{
	public:
		Animation();
		virtual ~Animation();

		// TODO: How do we define this?
	};
}