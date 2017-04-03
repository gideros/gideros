#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H

#include "event.h"
#include "ginput.h"

class MouseEvent : public Event
{
public:
	typedef EventType<MouseEvent> Type;

    MouseEvent(const Type& type, int x, int y, float sx, float sy, float tx, float ty) :
        Event(type.type()),
        x(x), y(y),wheel(0),button(GINPUT_LEFT_BUTTON),modifiers(GINPUT_NO_MODIFIER),
        sx(sx), sy(sy), tx(tx), ty(ty)
	{

	}

	int x, y;
    int wheel;
    int button;
    int modifiers;

    float sx, sy, tx, ty;


	static Type MOUSE_UP;
	static Type MOUSE_DOWN;
	static Type MOUSE_MOVE;
    static Type MOUSE_HOVER;
	static Type MOUSE_WHEEL;

	virtual void apply(EventVisitor* v);
};

#endif
