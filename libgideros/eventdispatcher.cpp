#include "eventdispatcher.h"

std::set<EventDispatcher*> EventDispatcher::allEventDispatchers_;
void EventDispatcher::broadcastEvent(Event* event)
{
	std::vector<EventDispatcher*> v(allEventDispatchers_.begin(), allEventDispatchers_.end());	// NOTE: bunu static yapma. broadcastEvent icindeyken broadcastEvent'in cagirilma ihtimali var.

	for (std::size_t i = 0; i < v.size(); ++i)
		v[i]->ref();

	try
	{
		for (std::size_t i = 0; i < v.size(); ++i)
			v[i]->dispatchEvent(event);
	}
	catch(...)
	{
		for (std::size_t i = 0; i < v.size(); ++i)
			v[i]->unref();
		throw;
	}

	for (std::size_t i = 0; i < v.size(); ++i)
		v[i]->unref();
}
