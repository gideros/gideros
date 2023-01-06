#include "mouseevent.h"
#include "eventvisitor.h"

MouseEvent::Type MouseEvent::MOUSE_UP("mouseUp");
MouseEvent::Type MouseEvent::MOUSE_DOWN("mouseDown");
MouseEvent::Type MouseEvent::MOUSE_MOVE("mouseMove");
MouseEvent::Type MouseEvent::MOUSE_HOVER("mouseHover");
MouseEvent::Type MouseEvent::MOUSE_WHEEL("mouseWheel");
MouseEvent::Type MouseEvent::MOUSE_ENTER("mouseEnter");
MouseEvent::Type MouseEvent::MOUSE_LEAVE("mouseLeave");

void MouseEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
