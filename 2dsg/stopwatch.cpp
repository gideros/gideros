#include "stopwatch.h"
#include "platform.h"

StopWatch::StopWatch()
{
	isRunning_ = true;
	last_ = iclock();
	total_ = 0;
}

double StopWatch::clock()
{
	double current = iclock();
	if (isRunning_ == true)
		total_ += current - last_;
	last_ = current;

	return total_;
}

void StopWatch::pause()
{
	clock();
	isRunning_ = false;
	clock();
}

void StopWatch::resume()
{
	clock();
	isRunning_ = true;
	clock();
}

bool StopWatch::isPaused() const
{
	return !isRunning_;
}
