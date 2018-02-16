#ifndef LAYOUTEVENT_H
#define LAYOUTEVENT_H

#include "event.h"

class LayoutEvent : public Event
{
public:
    typedef EventType<LayoutEvent> Type;

    LayoutEvent(const Type& type, float w,float h) : Event(type.type()), width(w), height(h)
	{

	}
    float width,height;

    static Type RESIZED;

	virtual void apply(EventVisitor* v);
};

#endif
