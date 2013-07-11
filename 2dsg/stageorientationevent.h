#ifndef STAGE_ORIENTATION_EVENT
#define STAGE_ORIENTATION_EVENT

#include "event.h"
#include "orientation.h"

class EventVisitor;

class StageOrientationEvent : public Event
{
public:
	typedef EventType<StageOrientationEvent> Type;

	StageOrientationEvent(const Type& type, Orientation beforeOrientation, Orientation afterOrientation) : 
		Event(type.type()), beforeOrientation(beforeOrientation), afterOrientation(afterOrientation)
	{

	}

	Orientation beforeOrientation;
	Orientation afterOrientation;

	static Type ORIENTATION_CHANGE;

	virtual void apply(EventVisitor* v);
};


#endif
