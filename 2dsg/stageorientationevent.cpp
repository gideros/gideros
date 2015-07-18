#include "stageorientationevent.h"
#include "eventvisitor.h"

StageOrientationEvent::Type StageOrientationEvent::ORIENTATION_CHANGE("orientationChange");

void StageOrientationEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
