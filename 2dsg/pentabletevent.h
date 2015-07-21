#ifndef PENTABLETEVENT_H
#define PENTABLETEVENT_H

#include "event.h"
#include "ginput.h"

class PenTabletEvent : public Event
{
public:
    typedef EventType<PenTabletEvent> Type;

    PenTabletEvent(const Type& type, int x, int y, float sx, float sy, float tx, float ty) :
        Event(type.type()),
        x(x), y(y),pointerType(0),pressure(0),
        tiltx(0),tilty(0),
        tangentialPressure(0),
        sx(sx), sy(sy), tx(tx), ty(ty)
    {

    }

    int x, y;
    int pointerType, pressure, tiltx, tilty, tangentialPressure;

    float sx, sy, tx, ty;


    static Type PENTABLET_PRESS;
    static Type PENTABLET_MOVE;
    static Type PENTABLET_RELEASE;

    virtual void apply(EventVisitor* v);
};

#endif
