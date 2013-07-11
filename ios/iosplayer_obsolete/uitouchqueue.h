#ifndef TOUCHQUEUE_H
#define TOUCHQUEUE_H

#include <vector>
#include <queue>
#include <map>
#include "../../2dsg/touch.h"

struct TouchQueueElement
{
public:
	TouchQueueElement(int type, const std::vector<Touch*>& touches);
	~TouchQueueElement();
	void copy(int type, const std::vector<Touch*>& touches);

	int type;
	std::vector<Touch*> touches;

private:
	TouchQueueElement(const TouchQueueElement&);
	TouchQueueElement& operator=(TouchQueueElement&);
};

class TouchQueue
{
public:
	TouchQueue() {}
	~TouchQueue();

	void push(int type, const std::vector<Touch*>& touches);
	TouchQueueElement* front() const;
	void pop();
	bool empty() const;

private:
	std::deque<TouchQueueElement*> queue;
	std::map<int, std::vector<TouchQueueElement*> > cache;
};

#endif // TOUCHQUEUE_H
