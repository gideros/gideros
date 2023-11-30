#ifndef EVENTVISITOR_H
#define EVENTVISITOR_H

#ifndef G_UNUSED
#define G_UNUSED(x) (void)(x)
#endif

class Event;
class EnterFrameEvent;
class MouseEvent;
class TouchEvent;
class TimerEvent;
class AccelerometerEvent;
class StageOrientationEvent;
class ErrorEvent;
class ProgressEvent;
class KeyboardEvent;
class CompleteEvent;
class LayoutEvent;
class OpenUrlEvent;
class TextInputEvent;
class PermissionEvent;

class EventVisitor
{
public:
	virtual void visit(Event* v) = 0;
	virtual void visit(EnterFrameEvent* v) = 0;
	virtual void visit(MouseEvent* v) = 0;
	virtual void visit(TouchEvent* v) = 0;
	virtual void visit(TimerEvent* v) = 0;
	virtual void visit(AccelerometerEvent* v) = 0;
	virtual void visit(StageOrientationEvent* v) = 0;
	virtual void visit(ErrorEvent* v) = 0;
	virtual void visit(ProgressEvent* v) = 0;
	virtual void visit(KeyboardEvent* v) = 0;
    virtual void visit(CompleteEvent* v) = 0;
    virtual void visit(LayoutEvent* v) = 0;
    virtual void visit(OpenUrlEvent* v) = 0;
    virtual void visit(TextInputEvent* v) = 0;
    virtual void visit(PermissionEvent* v) = 0;

    virtual void visitOther(Event* v, void* data) { G_UNUSED(v); G_UNUSED(data); }
    virtual ~EventVisitor()  {};
};

#endif
