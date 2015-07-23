#include "progressevent.h"
#include "eventvisitor.h"

ProgressEvent::Type ProgressEvent::PROGRESS("progress");

void ProgressEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
