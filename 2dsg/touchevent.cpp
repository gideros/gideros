#include "touchevent.h"
#include "eventvisitor.h"

TouchEvent::Type TouchEvent::TOUCHES_BEGIN("touchesBegin");
TouchEvent::Type TouchEvent::TOUCHES_MOVE("touchesMove");
TouchEvent::Type TouchEvent::TOUCHES_END("touchesEnd");
TouchEvent::Type TouchEvent::TOUCHES_CANCEL("touchesCancel");

void TouchEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
