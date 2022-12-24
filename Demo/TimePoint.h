#pragma once

#include <Windows.h>
#include <stdint.h>

class TimePoint
{
public:

	TimePoint();
	TimePoint(const TimePoint& timePoint);
	virtual ~TimePoint();

	void operator=(const TimePoint& timePoint);
	void operator+=(const TimePoint& timePoint);
	void operator-=(const TimePoint& timePoint);
	TimePoint operator+(const TimePoint& timePoint) const;
	TimePoint operator-(const TimePoint& timePoint) const;
	bool operator==(const TimePoint& timePoint) const;
	bool operator!=(const TimePoint& timePoint) const;
	bool operator<(const TimePoint& timePoint) const;
	bool operator>(const TimePoint& timePoint) const;
	bool operator<=(const TimePoint& timePoint) const;
	bool operator>=(const TimePoint& timePoint) const;

	TimePoint CalcDistanceTo(const TimePoint& timePoint) const;
	TimePoint& SetFromFileTime(const FILETIME& ft);
	void GetToFileTime(FILETIME& ft) const;
	TimePoint& SetFromCurrentSystemTime();
	TimePoint& SetFromFileTimeStamp(const char* file);
	TimePoint& SetFromMillseconds(double milliseconds);
	double GetToMilliseconds(void) const;
	void GetToSystemTime(SYSTEMTIME& st) const;
	void SetTime(uint64_t givenTime);
	uint64_t GetTime(void) const;

protected:

	uint64_t time;
};