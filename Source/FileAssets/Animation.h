#pragma once

#include "Defines.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include <vector>
#include <string>

namespace Frumpy
{
	class Skeleton;

	class FRUMPY_API Animation
	{
	public:
		Animation();
		virtual ~Animation();

		bool BindTo(Skeleton* skeleton);
		bool Unbind();

		void Animate(double deltaTime);
		void SetPlayRate(double givenPlayRate);
		double GetPlayRate() const { return this->playRate; }

		struct KeyFrame
		{
			Quaternion orientation;
			Vector3 translation;
			double time;
		};

		struct Sequence
		{
			std::string name;
			std::vector<KeyFrame> keyFrameArray;
			Matrix4x4* boundTransform;
		};

		std::vector<Sequence>* sequenceArray;
		double time;
		double minTime;
		double maxTime;
		double playRate;
		Skeleton* boundSkeleton;
		bool wrap;
	};
}