#ifndef TIMERCONTAINER_H
#define TIMERCONTAINER_H

#include <set>
#include <map>
#include <vector>
#include "stopwatch.h"

class Timer;

class TimerContainer
{
public:
	static TimerContainer& instance();

	void addTimer(Timer* timer);
	void removeTimer(Timer* timer);
	void removeAllTimers();
	void tick();

	void pauseAllTimers();
	void resumeAllTimers();
	bool isAllTimersPaused() const;

private:
	std::set<Timer*> timers_;
	std::map<double, std::vector<Timer*> > queue_;
	StopWatch stopWatch_;
};

#endif
