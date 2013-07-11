#include "uitouchqueue.h"
#include <stdlib.h>
#include <memory.h>

TouchQueueElement::TouchQueueElement(int type, const std::vector<Touch*>& touches)
{
	this->touches.resize(touches.size());
	for (size_t i = 0; i < this->touches.size(); ++i)
		this->touches[i] = new Touch;
	copy(type, touches);
}

TouchQueueElement::~TouchQueueElement()
{
	for (size_t i = 0; i < touches.size(); ++i)
		delete touches[i];
	touches.clear();
}

void TouchQueueElement::copy(int type, const std::vector<Touch*>& touches)
{
	this->type = type;
	for (size_t i = 0; i < touches.size(); ++i)
		*this->touches[i] = *touches[i];
}


void TouchQueue::push(int type, const std::vector<Touch*>& touches)
{
	int size = touches.size();
	
	if (cache[size].empty())
	{
		queue.push_back(new TouchQueueElement(type, touches));
	}
	else
	{
		TouchQueueElement* element = cache[size].back();
		cache[size].pop_back();
		element->copy(type, touches);
		queue.push_back(element);
	}
}

TouchQueue::~TouchQueue()
{
	for (size_t i = 0; i < queue.size(); ++i)
		delete queue[i];
	queue.clear();

	for (int j = 0; j < 9; ++j)
	{
		for (size_t i = 0; i < cache[j].size(); ++i)
			delete cache[j][i];
		cache[j].clear();
	}
}

TouchQueueElement* TouchQueue::front() const
{
	return queue.front();
}

void TouchQueue::pop()
{
	TouchQueueElement* element = queue.front();
	queue.pop_front();
	cache[element->touches.size()].push_back(element);
}

bool TouchQueue::empty() const
{
	return queue.empty();
}
