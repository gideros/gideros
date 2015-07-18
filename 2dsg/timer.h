#ifndef TIMER_H_AC
#define TIMER_H_AC

#include "eventdispatcher.h"

class Application;

class TimerContainer;

class Timer : public EventDispatcher
{
public:
    Timer(Application *application, double delay, int repeatCount = 0);
	virtual ~Timer();

	void start();
	void stop();
	void reset()
	{
		stop();
		currentCount_ = 0;
	}
    void pause();

	virtual void tick();

	double delay() const
	{
		return delay_;
	}

	int currentCount() const
	{
		return currentCount_;
	}

	int repeatCount() const
	{
		return repeatCount_;
	}

	bool running() const
	{
		return running_;
	}

	void setDelay(double delay)
	{
		delay_ = delay;

		if (running() == true)
		{
			stop();
			start();
		}
	}

	void setRepeatCount(int repeatCount)
	{
		repeatCount_ = repeatCount;

		if (repeatCount_ != 0 && currentCount_ >= repeatCount_)
			stop();
	}
	
private:
	double delay_;
	int repeatCount_;
	bool running_;
	int currentCount_;
    double additionalDelay_;

	TimerContainer* container_;

    Application *application_;
};

#endif
