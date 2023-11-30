#include "errorevent.h"
#include "eventvisitor.h"
#include "permissionevent.h"

ErrorEvent::Type ErrorEvent::ERROR("error");

void ErrorEvent::apply(EventVisitor* v)
{
	v->visit(this);
}

PermissionEvent::Type PermissionEvent::PERMISSION("permission");
void PermissionEvent::apply(EventVisitor* v)
{
    v->visit(this);
}
