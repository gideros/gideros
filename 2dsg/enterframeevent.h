#ifndef ENTERFRAMEEVENT_H
#define ENTERFRAMEEVENT_H

#include "event.h"

class EnterFrameEvent : public Event
{
public:
	typedef EventType<EnterFrameEvent> Type;

	EnterFrameEvent(	const Type& type,
						int frameCount, int deltaFrameCount,
						double time, double deltaTime, double lastFrameRenderTime) : Event(type.type()),
		frameCount_(frameCount),
		deltaFrameCount_(deltaFrameCount),
		time_(time),
		deltaTime_(deltaTime),
		lastFrameRenderTime_(lastFrameRenderTime)
	{

	}

	int frameCount() const
	{
		return frameCount_;
	}

	int deltaFrameCount() const
	{
		return deltaFrameCount_;
	}

	double time() const
	{
		return time_;
	}

	double deltaTime() const
	{
		return deltaTime_;
	}

	double lastFrameRenderTime() const
	{
		return lastFrameRenderTime_;
	}


	virtual void apply(EventVisitor* v);

	static Type ENTER_FRAME;

private:
	int frameCount_;
	int deltaFrameCount_;
	double time_;
	double deltaTime_;
	double lastFrameRenderTime_;
};

#endif
