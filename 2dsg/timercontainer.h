#ifndef TIMERCONTAINER_H
#define TIMERCONTAINER_H

#include <set>
#include <map>
#include <vector>
#include <deque>
#include "stopwatch.h"

class Timer;
class Application;

class TimerContainer
{
public:
    TimerContainer(Application *application);
    ~TimerContainer();

    void addTimer(Timer* timer, double additionalDelay);
	void removeTimer(Timer* timer);
	void removeAllTimers();
	void tick();
	void suspend();
	void resume();

    double getAdditionalDelay(const Timer *timer);

	void pauseAllTimers();
	void resumeAllTimers();
	bool isAllTimersPaused() const;

    void queueTimerEvent(Timer *timer);
    void queueTimerCompleteEvent(Timer *timer);
    void removeEvents(Timer *timer);

    double getTimer() { return globalTimer_.clock(); };

private:
    Application *application_;
    std::set<Timer*> timers_;
	std::map<double, std::vector<Timer*> > queue_;
	StopWatch stopWatch_;
	StopWatch globalTimer_;
    std::deque<std::pair<Timer*, int> > eventQueue_;
    bool pausedBeforeSuspend_;
};

#endif
