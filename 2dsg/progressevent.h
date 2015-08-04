#ifndef PROGRESSEVENT_H
#define PROGRESSEVENT_H

#include "event.h"

class ProgressEvent : public Event
{
public:
	typedef EventType<ProgressEvent> Type;

	ProgressEvent(const Type& type, size_t bytesLoaded, size_t bytesTotal) : Event(type.type()), bytesLoaded(bytesLoaded), bytesTotal(bytesTotal)
	{

	}

	static Type PROGRESS;

	size_t bytesLoaded;
	size_t bytesTotal;

	virtual void apply(EventVisitor* v);
};

#endif
