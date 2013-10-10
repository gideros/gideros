#ifndef TIMERCONTAINER_H
#define TIMERCONTAINER_H

#include <set>
#include <map>
#include <vector>
#include <deque>
#include "stopwatch.h"

class Timer;

class TimerContainer
{
public:
    TimerContainer();
    ~TimerContainer();

    void addTimer(Timer* timer, double additionalDelay);
	void removeTimer(Timer* timer);
	void removeAllTimers();
	void tick();

    double getAdditionalDelay(const Timer *timer);

	void pauseAllTimers();
	void resumeAllTimers();
	bool isAllTimersPaused() const;

    void queueTimerEvent(Timer *timer);
    void queueTimerCompleteEvent(Timer *timer);
    void removeEvents(Timer *timer);

private:
	std::set<Timer*> timers_;
	std::map<double, std::vector<Timer*> > queue_;
	StopWatch stopWatch_;
    std::deque<std::pair<Timer*, int> > eventQueue_;
};

#endif
