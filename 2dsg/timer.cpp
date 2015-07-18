#include "timer.h"
#include "timercontainer.h"
#include "timerevent.h"
#include "platform.h"
#include <glog.h>
#include "application.h"

Timer::Timer(Application *application, double delay, int repeatCount)
{
    application_ = application;
	delay_ = delay;
	repeatCount_ = repeatCount;
	running_ = false;
	currentCount_ = 0;
    additionalDelay_ = 0;

    container_ = application_->getTimerContainer();
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
    container_->removeEvents(this);
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
	currentCount_++;

    container_->queueTimerEvent(this);

	if (repeatCount_ != 0 && currentCount_ >= repeatCount_)
	{
        additionalDelay_ = 0;
        container_->removeTimer(this);
        running_ = false;
        container_->queueTimerCompleteEvent(this);
    }
}
