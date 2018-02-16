#include "layoutevent.h"
#include "eventvisitor.h"

LayoutEvent::Type LayoutEvent::RESIZED("layout_resized");

void LayoutEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
