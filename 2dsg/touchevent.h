#ifndef TOUCHEVENT_H
#define TOUCHEVENT_H

#include "event.h"
#include "touch.h"

#include <vector>

#include <ginput.h>

class TouchEvent : public Event
{
public:
	typedef EventType<TouchEvent> Type;

    TouchEvent(const Type& type, ginput_TouchEvent *event, float sx, float sy, float tx, float ty) :
		Event(type.type()),
        event(event),
        sx(sx), sy(sy), tx(tx), ty(ty)
    {

	}

    ginput_TouchEvent *event;

    float sx, sy, tx, ty;

	static Type TOUCHES_BEGIN;
	static Type TOUCHES_MOVE;
	static Type TOUCHES_END;
	static Type TOUCHES_CANCEL;

	virtual void apply(EventVisitor* v);
};

#endif
