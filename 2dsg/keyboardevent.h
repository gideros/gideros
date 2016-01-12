#ifndef KEYBOARDEVENT_H
#define KEYBOARDEVENT_H

#include "event.h"
#include <string>

class KeyboardEvent : public Event
{
public:
	typedef EventType<KeyboardEvent> Type;

    KeyboardEvent(const Type& type, int keyCode, int realCode, std::string charCode) : Event(type.type()), keyCode(keyCode), realCode(realCode),charCode(charCode)
	{

	}

	static Type KEY_DOWN;
	static Type KEY_UP;
	static Type KEY_CHAR;

	int keyCode;
    int realCode;
    std::string charCode;

	virtual void apply(EventVisitor* v);
};

#endif
