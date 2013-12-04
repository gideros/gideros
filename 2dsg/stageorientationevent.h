#ifndef STAGE_ORIENTATION_EVENT
#define STAGE_ORIENTATION_EVENT

#include "event.h"
#include <gapplication.h>

class EventVisitor;

class StageOrientationEvent : public Event
{
public:
	typedef EventType<StageOrientationEvent> Type;

    StageOrientationEvent(const Type& type, gapplication_Orientation orientation) :
        Event(type.type()), orientation(orientation)
	{

	}

    gapplication_Orientation orientation;

	static Type ORIENTATION_CHANGE;

	virtual void apply(EventVisitor* v);
};


#endif
