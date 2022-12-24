#include "ProfileBlock.h"

TimingStat::TimingStat()
{
	this->maxSamples = 100;
}

/*virtual*/ TimingStat::~TimingStat()
{
}

double TimingStat::GetAverageMilliseconds() const
{
	if (this->timeSamplesList.size() == 0)
		return 0.0;

	double totalMilliseconds = 0.0;
	for (const TimePoint& timePoint : this->timeSamplesList)
		totalMilliseconds += timePoint.GetToMilliseconds();

	double averageMilliseconds = totalMilliseconds / double(this->timeSamplesList.size());
	return averageMilliseconds;
}

ProfileBlock::ProfileBlock(TimingStat* timingStat)
{
	this->timingStat = timingStat;
	this->startTime.SetFromCurrentSystemTime();
}

/*virtual*/ ProfileBlock::~ProfileBlock()
{
	this->stopTime.SetFromCurrentSystemTime();
	TimePoint deltaTime = this->stopTime - this->startTime;
	this->timingStat->timeSamplesList.push_back(deltaTime);
	while (this->timingStat->timeSamplesList.size() > this->timingStat->maxSamples)
		this->timingStat->timeSamplesList.pop_front();
}