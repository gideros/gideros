#ifndef KEYBOARDEVENT_H
#define KEYBOARDEVENT_H

#include "event.h"
#include <string>

class KeyboardEvent : public Event
{
public:
	typedef EventType<KeyboardEvent> Type;

    KeyboardEvent(const Type& type, int keyCode, int realCode, std::string charCode, int modifiers) : Event(type.type()), keyCode(keyCode), realCode(realCode),charCode(charCode),modifiers(modifiers)
	{

	}

	static Type KEY_DOWN;
	static Type KEY_UP;
	static Type KEY_CHAR;

	int keyCode;
    int realCode;
    std::string charCode;
    int modifiers;

	virtual void apply(EventVisitor* v);
};

#endif
