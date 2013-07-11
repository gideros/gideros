#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H

#include "event.h"

class MouseEvent : public Event
{
public:
	typedef EventType<MouseEvent> Type;

    MouseEvent(const Type& type, int x, int y, float sx, float sy, float tx, float ty) :
        Event(type.type()),
        x(x), y(y),
        sx(sx), sy(sy), tx(tx), ty(ty)
	{

	}

	int x, y;

    float sx, sy, tx, ty;

	static Type MOUSE_UP;
	static Type MOUSE_DOWN;
	static Type MOUSE_MOVE;

	virtual void apply(EventVisitor* v);
};

#endif
