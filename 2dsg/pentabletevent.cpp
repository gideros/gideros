#include "pentabletevent.h"
#include "eventvisitor.h"

PenTabletEvent::Type PenTabletEvent::PENTABLET_PRESS("pentabletPress");
PenTabletEvent::Type PenTabletEvent::PENTABLET_MOVE("pentabletMove");
PenTabletEvent::Type PenTabletEvent::PENTABLET_RELEASE("pentabletRelease");

void PenTabletEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
