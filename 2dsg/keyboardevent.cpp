#include "keyboardevent.h"
#include "eventvisitor.h"

KeyboardEvent::Type KeyboardEvent::KEY_DOWN("keyDown");
KeyboardEvent::Type KeyboardEvent::KEY_UP("keyUp");
KeyboardEvent::Type KeyboardEvent::KEY_CHAR("keyChar");

void KeyboardEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
