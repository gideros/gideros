#ifndef UITOUCHMANAGER_H
#define UITOUCHMANAGER_H

#include "../2dsg/touch.h"
#include <map>
#include <set>
#include <vector>
#include <stack>

class UITouchManager
{
public:
	UITouchManager();
	~UITouchManager();

	void update(const std::set<UITouch*>& uiTouches,
				const std::set<UITouch*>& uiAllTouches,
				std::vector<Touch*>* touches,
				std::vector<Touch*>* allTouches);

private:
	Touch* newTouch();
	void deleteTouch(Touch* touch);
	
private:
	std::map<UITouch*, Touch*> touchMap_;
	std::stack<Touch*> touchPool_;
};

#endif