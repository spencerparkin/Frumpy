#include "TimePoint.h"
#include <sysinfoapi.h>

TimePoint::TimePoint()
{
	this->time = 0;
}

TimePoint::TimePoint(const TimePoint& timePoint)
{
	this->time = timePoint.time;
}

/*virtual*/ TimePoint::~TimePoint()
{
}

void TimePoint::operator=(const TimePoint& timePoint)
{
	this->time = timePoint.time;
}

void TimePoint::operator+=(const TimePoint& timePoint)
{
	this->time += timePoint.time;
}

void TimePoint::operator-=(const TimePoint& timePoint)
{
	this->time -= timePoint.time;
}

TimePoint TimePoint::operator+(const TimePoint& timePoint) const
{
	TimePoint result;
	result.time = this->time + timePoint.time;
	return result;
}

TimePoint TimePoint::operator-(const TimePoint& timePoint) const
{
	TimePoint result;
	result.time = this->time - timePoint.time;
	return result;
}

bool TimePoint::operator==(const TimePoint& timePoint) const
{
	return this->time == timePoint.time;
}

bool TimePoint::operator!=(const TimePoint& timePoint) const
{
	return this->time != timePoint.time;
}

bool TimePoint::operator<(const TimePoint& timePoint) const
{
	return this->time < timePoint.time;
}

bool TimePoint::operator>(const TimePoint& timePoint) const
{
	return this->time > timePoint.time;
}

bool TimePoint::operator<=(const TimePoint& timePoint) const
{
	return this->time <= timePoint.time;
}

bool TimePoint::operator>=(const TimePoint& timePoint) const
{
	return this->time >= timePoint.time;
}

TimePoint TimePoint::CalcDistanceTo(const TimePoint& timePoint) const
{
	TimePoint result;
	result.time = (this->time > timePoint.time) ? (this->time - timePoint.time) : (timePoint.time - this->time);
	return result;
}

TimePoint& TimePoint::SetFromFileTime(const FILETIME& ft)
{
	ULARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	this->time = li.QuadPart;
	return *this;
}

void TimePoint::GetToFileTime(FILETIME& ft) const
{
	ULARGE_INTEGER li;
	li.QuadPart = this->time;
	ft.dwLowDateTime = li.LowPart;
	ft.dwHighDateTime = li.HighPart;
}

TimePoint& TimePoint::SetFromCurrentSystemTime()
{
	// Note that the following windows call should not only be very accurate,
	// but it should also be synchronized across machines, because it is
	// based on UTC time.
	FILETIME ft;
	::GetSystemTimePreciseAsFileTime(&ft);
	this->SetFromFileTime(ft);
	return *this;
}

TimePoint& TimePoint::SetFromFileTimeStamp(const char* file)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	::GetFileAttributesEx(file, GetFileExInfoStandard, &data);
	this->SetFromFileTime(data.ftLastWriteTime);
	return *this;
}

TimePoint& TimePoint::SetFromMillseconds(double milliseconds)
{
	long double oneHundredNanoSecondTicks = milliseconds * 10000.0;
	this->time = (uint64_t)oneHundredNanoSecondTicks;
	return *this;
}

double TimePoint::GetToMilliseconds(void) const
{
	long double oneHundredNanoSecondTicks = (long double)this->time;
	long double milliseconds = oneHundredNanoSecondTicks / 10000.0;
	return double(milliseconds);
}

void TimePoint::GetToSystemTime(SYSTEMTIME& st) const
{
	FILETIME ft;
	this->GetToFileTime(ft);
	::FileTimeToSystemTime(&ft, &st);
}

void TimePoint::SetTime(uint64_t givenTime)
{
	this->time = givenTime;
}

uint64_t TimePoint::GetTime(void) const
{
	return this->time;
}