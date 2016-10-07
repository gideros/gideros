#include "timercontainer.h"
#include "timer.h"
#include "platform.h"
#include <algorithm>
#include <glog.h>
#include "timerevent.h"
#include <application.h>
#include <functional>

TimerContainer::TimerContainer(Application *application) : application_(application)
{

}

TimerContainer::~TimerContainer()
{

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
    void *pool = application_->createAutounrefPool();

	double clock = stopWatch_.clock();

	while (queue_.empty() == false && queue_.begin()->first < clock)
	{
		std::vector<Timer*> timers = queue_.begin()->second;
		double curtime = queue_.begin()->first;

		queue_.erase(queue_.begin());

		for (std::size_t i = 0; i < timers.size(); ++i)
			if (timers[i]->running() == true)
				timers[i]->tick();

		for (std::size_t i = 0; i < timers.size(); ++i)
			if (timers[i]->running() == true)
				queue_[curtime + timers[i]->delay() / 1000].push_back(timers[i]);
	}

    while (!eventQueue_.empty())
    {
        Timer *timer = eventQueue_.front().first;
        int type = eventQueue_.front().second;
        eventQueue_.pop_front();

        timer->ref();
        application_->autounref(timer);

        if (type == 0)
        {
            TimerEvent timerEvent(TimerEvent::TIMER);
            timer->dispatchEvent(&timerEvent);
        }
        else if (type == 1)
        {
            TimerEvent timerEvent(TimerEvent::TIMER_COMPLETE);
            timer->dispatchEvent(&timerEvent);
        }
    }

    application_->deleteAutounrefPool(pool);
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

void TimerContainer::queueTimerEvent(Timer *timer)
{
    eventQueue_.push_back(std::make_pair(timer, 0));
}

void TimerContainer::queueTimerCompleteEvent(Timer *timer)
{
    eventQueue_.push_back(std::make_pair(timer, 1));
}

void TimerContainer::removeEvents(Timer *timer)
{
    eventQueue_.erase(std::remove(eventQueue_.begin(), eventQueue_.end(), std::make_pair(timer, 0)), eventQueue_.end());
    eventQueue_.erase(std::remove(eventQueue_.begin(), eventQueue_.end(), std::make_pair(timer, 1)), eventQueue_.end());
}

void TimerContainer::suspend()
{
	pausedBeforeSuspend_=isAllTimersPaused();
	if (!pausedBeforeSuspend_)
		pauseAllTimers();
	globalTimer_.pause();
}

void TimerContainer::resume()
{
	if (!pausedBeforeSuspend_)
		resumeAllTimers();
	globalTimer_.resume();
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
