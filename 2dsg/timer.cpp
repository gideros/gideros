#include "timer.h"
#include "timercontainer.h"
#include "timerevent.h"
#include "platform.h"

Timer::Timer(double delay, int repeatCount)
{
	//debuglog("Timer()");
	delay_ = delay;
	repeatCount_ = repeatCount;
	running_ = false;
	currentCount_ = 0;

	container_ = &TimerContainer::instance();
}

Timer::~Timer()
{
	//debuglog("~Timer()");
	stop();
}

void Timer::start()
{
	if (running_ == true)
		return;
	running_ = true;

	container_->addTimer(this);
}

void Timer::stop()
{
	if (running_ == false)
		return;
	running_ = false;

	container_->removeTimer(this);
}

void Timer::tick()
{
//	printf("Timer::tick()\n");
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
