#include "mouseevent.h"
#include "eventvisitor.h"

MouseEvent::Type MouseEvent::MOUSE_UP("mouseUp");
MouseEvent::Type MouseEvent::MOUSE_DOWN("mouseDown");
MouseEvent::Type MouseEvent::MOUSE_MOVE("mouseMove");

void MouseEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
