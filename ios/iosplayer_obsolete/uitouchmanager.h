#ifndef UITOUCHMANAGER_H
#define UITOUCHMANAGER_H

#include "../../2dsg/touch.h"
#include <map>
#include <set>
#include <vector>
#include <stack>

#import <UIKit/UIKit.h>

class UITouchManager
{
public:
	UITouchManager();
	~UITouchManager();

	void update(UIView* view,
				const std::vector<UITouch*>& uiTouches,
				const std::vector<UITouch*>& uiAllTouches,
				std::vector<Touch*>* touches,
				std::vector<Touch*>* allTouches);

	int poolSize() const
	{
		return touchPool_.size();
	}

private:
	Touch* newTouch();
	void deleteTouch(Touch* touch);
	
private:
	std::map<UITouch*, Touch*> touchMap_;
	std::map<int, Touch*> touchPool_;
	int nextid_;
	std::vector<Touch*> deleteList_;
};

#endif