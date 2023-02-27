#include "Animation.h"
#include "Skeleton.h"

using namespace Frumpy;

Animation::Animation()
{
	this->time = 0.0;
	this->minTime = 0.0;
	this->maxTime = 0.0;
	this->playRate = 0.0;
	this->boundSkeleton = nullptr;
	this->wrap = true;
	this->sequenceArray = new std::vector<Sequence>();
}

/*virtual*/ Animation::~Animation()
{
	delete this->sequenceArray;
}

bool Animation::BindTo(Skeleton* skeleton)
{
	if (this->boundSkeleton)
		return false;

	this->minTime = DBL_MAX;
	this->maxTime = -DBL_MAX;

	for(Sequence& sequence : *this->sequenceArray)
	{
		Skeleton::BoneSpace* boneSpace = skeleton->FindBoneSpaceByName(sequence.name.c_str());
		if (!boneSpace)
		{
			this->Unbind();
			return false;
		}

		sequence.boundTransform = &boneSpace->currentPoseChildToParent;

		if (sequence.keyFrameArray.size() > 0)
		{
			if (this->minTime > sequence.keyFrameArray[0].time)
				this->minTime = sequence.keyFrameArray[0].time;

			if (this->maxTime < sequence.keyFrameArray[sequence.keyFrameArray.size() - 1].time)
				this->maxTime = sequence.keyFrameArray[sequence.keyFrameArray.size() - 1].time;
		}
	}

	this->time = this->minTime;
	this->boundSkeleton = skeleton;
	return true;
}

bool Animation::Unbind()
{
	for (Sequence& sequence : *this->sequenceArray)
		sequence.boundTransform = nullptr;

	this->boundSkeleton = nullptr;
	return true;
}

void Animation::Animate(double deltaTime)
{
	double newTime = this->time + this->playRate * deltaTime;

	if (this->wrap)
	{
		if (newTime > this->maxTime)
			newTime -= this->maxTime - this->minTime;

		if (newTime < this->minTime)
			newTime += this->maxTime - this->minTime;
	}

	this->SetTime(newTime);
}

void Animation::SetTime(double givenTime)
{
	this->time = givenTime;

	if (this->time > this->maxTime)
		this->time = this->maxTime;

	if (this->time < this->minTime)
		this->time = this->minTime;

	if (this->boundSkeleton)
	{
		for (Sequence& sequence : *this->sequenceArray)
		{
			// This is a really silly way to interpolate between key-frames, but it's something for now.
			// Note that we don't have continuity of derivatives or anything like that.  I.e., it's not "smooth."
			for (unsigned int i = 0; i < sequence.keyFrameArray.size() - 1; i++)
			{
				int j = i + 1;

				const KeyFrame& keyFrameA = sequence.keyFrameArray[i];
				const KeyFrame& keyFrameB = sequence.keyFrameArray[j];

				if (keyFrameA.time <= this->time && this->time < keyFrameB.time)
				{
					double alpha = (time - keyFrameA.time) / (keyFrameB.time - keyFrameA.time);

					Matrix3x3 interpolatedOrientation;
					interpolatedOrientation.InterpolateRotations(keyFrameA.orientation, keyFrameB.orientation, alpha);

					// This only overwrites the upper-left 3x3 submatrix of the 4x4 matrix.
					*sequence.boundTransform = interpolatedOrientation;

					break;
				}
			}
		}

		this->boundSkeleton->rootSpace->ResolveCurrentPoseTransforms();
	}
}

void Animation::SetPlayRate(double givenPlayRate)
{
	this->playRate = givenPlayRate;
}