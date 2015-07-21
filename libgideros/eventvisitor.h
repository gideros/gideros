#ifndef EVENTVISITOR_H
#define EVENTVISITOR_H

class Event;
class EnterFrameEvent;
class MouseEvent;
class TouchEvent;
class PenTabletEvent;
class TimerEvent;
class AccelerometerEvent;
class StageOrientationEvent;
class ErrorEvent;
class ProgressEvent;
class KeyboardEvent;
class CompleteEvent;

class EventVisitor
{
public:
	virtual void visit(Event* v) = 0;
	virtual void visit(EnterFrameEvent* v) = 0;
	virtual void visit(MouseEvent* v) = 0;
	virtual void visit(TouchEvent* v) = 0;
    virtual void visit(PenTabletEvent* v) = 0;
	virtual void visit(TimerEvent* v) = 0;
	virtual void visit(AccelerometerEvent* v) = 0;
	virtual void visit(StageOrientationEvent* v) = 0;
	virtual void visit(ErrorEvent* v) = 0;
	virtual void visit(ProgressEvent* v) = 0;
	virtual void visit(KeyboardEvent* v) = 0;
    virtual void visit(CompleteEvent* v) = 0;

	virtual void visitOther(Event* v, void* data) {}
};

#endif
