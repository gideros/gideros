#ifndef PERMISSIONEVENT_H
#define PERMISSIONEVENT_H

#include "event.h"
#include <map>
#include <string>

class PermissionEvent : public Event
{
public:
    typedef EventType<PermissionEvent> Type;

    PermissionEvent(const Type& type) : Event(type.type())
	{

	}
    std::map<std::string,int> permissions;

    static Type PERMISSION;

	virtual void apply(EventVisitor* v);
};

#endif
