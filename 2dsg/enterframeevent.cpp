#include "enterframeevent.h"
#include "eventvisitor.h"

EnterFrameEvent::Type EnterFrameEvent::ENTER_FRAME("enterFrame");

void EnterFrameEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
