#pragma once

#include "TimePoint.h"
#include <list>

class TimingStat
{
	friend class ProfileBlock;

public:
	TimingStat();
	virtual ~TimingStat();

	double GetAverageMilliseconds() const;

private:
	std::list<TimePoint> timeSamplesList;
	unsigned int maxSamples;
};

class ProfileBlock
{
public:
	ProfileBlock(TimingStat* timingStat);
	virtual ~ProfileBlock();

private:
	TimingStat* timingStat;
	TimePoint startTime, stopTime;
};