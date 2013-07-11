#ifndef TIMEREVENT_H
#define TIMEREVENT_H

#include "event.h"

class TimerEvent : public Event
{
public:
	typedef EventType<TimerEvent> Type;

	TimerEvent(const Type& type) : Event(type.type())
	{

	}

	static Type TIMER;
	static Type TIMER_COMPLETE;

	virtual void apply(EventVisitor* v);
};

#endif
