#include "timercontainer.h"
#include "timer.h"
#include "platform.h"
#include <algorithm>

TimerContainer& TimerContainer::instance()
{
    static TimerContainer container;
    return container;
}

void TimerContainer::addTimer(Timer* timer, double additionalDelay)
{
    double clock = stopWatch_.clock() - additionalDelay;

	timers_.insert(timer);
	timer->ref();
	queue_[clock + timer->delay() / 1000].push_back(timer);
}

void TimerContainer::removeTimer(Timer* timer)
{
	timers_.erase(timer);

	for (std::map<double, std::vector<Timer*> >::iterator iter = queue_.begin(); iter != queue_.end(); ++iter)
	{
		std::vector<Timer*>& timers = iter->second;
		timers.erase(
			std::remove_if(
				timers.begin(),
				timers.end(),
				std::bind2nd(std::equal_to<Timer*>(), timer)),
			timers.end());
	}
	timer->unref();
}

void TimerContainer::removeAllTimers()
{
	while (timers_.empty() == false)
		(*timers_.begin())->stop();
}

void TimerContainer::tick()
{
	double clock = stopWatch_.clock();

	while (queue_.empty() == false && queue_.begin()->first < clock)
	{
		std::vector<Timer*> timers = queue_.begin()->second;

		queue_.erase(queue_.begin());

		for (std::size_t i = 0; i < timers.size(); ++i)
			if (timers[i]->running() == true)
				timers[i]->tick();

		for (std::size_t i = 0; i < timers.size(); ++i)
			if (timers[i]->running() == true)
				queue_[clock + timers[i]->delay() / 1000].push_back(timers[i]);
	}
}

double TimerContainer::getAdditionalDelay(const Timer *timer)
{
    std::map<double, std::vector<Timer*> >::iterator iter, e = queue_.end();
    for (iter = queue_.begin(); iter != e; ++iter)
    {
        const std::vector<Timer*> &timers = iter->second;
        if (std::find(timers.begin(), timers.end(), timer) != timers.end())
            return (timer->delay() / 1000) - (iter->first - stopWatch_.clock());
    }

    return 0;
}


void TimerContainer::pauseAllTimers()
{
	stopWatch_.pause();
}

void TimerContainer::resumeAllTimers()
{
	stopWatch_.resume();
}

bool TimerContainer::isAllTimersPaused() const
{
	return stopWatch_.isPaused();
}
