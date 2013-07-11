#ifndef KEYBOARDEVENT_H
#define KEYBOARDEVENT_H

#include "event.h"

class KeyboardEvent : public Event
{
public:
	typedef EventType<KeyboardEvent> Type;

	KeyboardEvent(const Type& type, int keyCode) : Event(type.type()), keyCode(keyCode)
	{

	}

	static Type KEY_DOWN;
	static Type KEY_UP;

	int keyCode;

	virtual void apply(EventVisitor* v);
};

#endif
