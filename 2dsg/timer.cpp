#include "timer.h"
#include "timercontainer.h"
#include "timerevent.h"
#include "platform.h"
#include <glog.h>

Timer::Timer(double delay, int repeatCount)
{
	delay_ = delay;
	repeatCount_ = repeatCount;
	running_ = false;
	currentCount_ = 0;
    additionalDelay_ = 0;

	container_ = &TimerContainer::instance();
}

Timer::~Timer()
{
    stop();
}

void Timer::start()
{
    if (!running_)
    {
        container_->addTimer(this, additionalDelay_);
        additionalDelay_ = 0;
        running_ = true;
    }
}

void Timer::stop()
{
    additionalDelay_ = 0;
    if (running_)
    {
        container_->removeTimer(this);
        running_ = false;
    }
}

void Timer::pause()
{
    if (running_)
    {
        additionalDelay_ = container_->getAdditionalDelay(this);
        container_->removeTimer(this);
        running_ = false;
    }
}

void Timer::tick()
{
	ref();

	currentCount_++;

	try
	{
		TimerEvent timerEvent(TimerEvent::TIMER);
		dispatchEvent(&timerEvent);
	}
	catch(...)
	{
		unref();
		throw;
	}
	
	if (repeatCount_ != 0 && currentCount_ >= repeatCount_)
	{
		try
		{
			TimerEvent timerEvent(TimerEvent::TIMER_COMPLETE);
			dispatchEvent(&timerEvent);
		}
		catch(...)
		{
			unref();
			throw;
		}

		stop();
	}

	unref();
}


void Timer::pauseAllTimers()
{
	TimerContainer::instance().pauseAllTimers();
}

void Timer::resumeAllTimers()
{
	TimerContainer::instance().resumeAllTimers();
}

void Timer::stopAllTimers()
{
	TimerContainer::instance().removeAllTimers();
}
