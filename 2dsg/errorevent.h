#ifndef ERROREVENT_H
#define ERROREVENT_H

#include "event.h"

class ErrorEvent : public Event
{
public:
	typedef EventType<ErrorEvent> Type;

	ErrorEvent(const Type& type) : Event(type.type())
	{

	}

	static Type ERROR;

	virtual void apply(EventVisitor* v);
};

#endif
