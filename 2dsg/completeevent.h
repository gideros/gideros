#ifndef COMPLETEEVENT_H
#define COMPLETEEVENT_H

#include "event.h"

class CompleteEvent : public Event
{
public:
    typedef EventType<CompleteEvent> Type;

    CompleteEvent(const Type& type) : Event(type.type())
    {

    }

    static Type COMPLETE;

    virtual void apply(EventVisitor* v);
};

class HeaderEvent : public Event
{
public:
	typedef EventType<HeaderEvent> Type;

	HeaderEvent(const Type& type) : Event(type.type())
	{

	}

	static Type HEADER;

	virtual void apply(EventVisitor* v);
};

#endif
