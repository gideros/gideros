#include "errorevent.h"
#include "eventvisitor.h"

ErrorEvent::Type ErrorEvent::ERROR("error");

void ErrorEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
