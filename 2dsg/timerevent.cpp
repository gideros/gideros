#include "timerevent.h"
#include "eventvisitor.h"

TimerEvent::Type TimerEvent::TIMER("timer");
TimerEvent::Type TimerEvent::TIMER_COMPLETE("timerComplete");

void TimerEvent::apply(EventVisitor* v)
{
	v->visit(this);
}
