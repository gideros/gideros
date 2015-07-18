#include "keyboardevent.h"
#include "eventvisitor.h"

KeyboardEvent::Type KeyboardEvent::KEY_DOWN("keyDown");
KeyboardEvent::Type KeyboardEvent::KEY_UP("keyUp");

void KeyboardEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
