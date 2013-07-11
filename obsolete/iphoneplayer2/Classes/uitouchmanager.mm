#include "uitouchmanager.h"
#include <vector>
#include <algorithm>

UITouchManager::UITouchManager()
{

}

UITouchManager::~UITouchManager()
{
	// TODO: touchMap_'i silme. hem touchMap_'i hem touchPool_'i silersen Touch*'lari double delete yapmis olursun.
	
	while (touchPool_.empty() == false)
	{
		Touch* touch = touchPool_.top();
		delete touch;
		touchPool_.pop();
	}
}


void UITouchManager::update(	const std::vector<UITouch*>& uiTouches,
								const std::vector<UITouch*>& uiAllTouches,
								std::vector<Touch*>* touches,
								std::vector<Touch*>* allTouches)
{

	// collect uiTouches and uiAllTouches
	static std::vector<UITouch*> touchSet;
	
	touchSet.clear();

	touchSet.insert(touchSet.end(), uiAllTouches.begin(), uiAllTouches.end());
	touchSet.insert(touchSet.end(), uiTouches.begin(), uiTouches.end());
	std::sort(touchSet.begin(), touchSet.end());
	touchSet.erase(std::unique(touchSet.begin(), touchSet.end()), touchSet.end());


	// delete the Touch'es that doesn't in touchSet
	static std::vector<UITouch*> toBeDeleted;
	toBeDeleted.clear();
	for (std::map<UITouch*, Touch*>::iterator iter = touchMap_.begin(); iter != touchMap_.end(); ++iter)
	{
		UITouch* uitouch = iter->first;

		if (std::binary_search(touchSet.begin(), touchSet.end(), uitouch) == false)
			toBeDeleted.push_back(uitouch);
	}

	for (std::size_t i = 0; i < toBeDeleted.size(); ++i)
	{
		UITouch* uitouch = toBeDeleted[i];
		Touch* touch = touchMap_[uitouch];

		touchMap_.erase(uitouch);

		deleteTouch(touch);
	}


    // create and update Touch'es for the new ones
    for (std::vector<UITouch*>::const_iterator iter = touchSet.begin(); iter != touchSet.end(); ++iter)
    {
        UITouch* uitouch = *iter;
        Touch* touch;

        {
            std::map<UITouch*, Touch*>::iterator iter = touchMap_.find(uitouch);

            if (iter == touchMap_.end())
            {
                touch = newTouch();
                touchMap_[uitouch] = touch;
            }
            else
            {
                touch = iter->second;
            }
        }
		
		// transfer uitouch to touch
		CGPoint location = [uitouch locationInView:nil];
//		touch->location = Point2i(location.x, location.y);
		touch->x = location.x;
		touch->y = location.y;

//		CGPoint previousLocation = [uitouch previousLocationInView:nil];
//		touch->previousLocation = Point2i(previousLocation.x, previousLocation.y);
		
		touch->tapCount = [uitouch tapCount];
		touch->timestamp = [uitouch timestamp];
		
/*		switch ([uitouch phase])
		{
			case UITouchPhaseBegan:
				touch->phase = eTouchPhaseBegan;
			case UITouchPhaseMoved:
				touch->phase = eTouchPhaseMoved;
				break;
			case UITouchPhaseStationary:
				touch->phase = eTouchPhaseStationary;
				break;
			case UITouchPhaseEnded:
				touch->phase = eTouchPhaseEnded;
				break;
			case UITouchPhaseCancelled:
				touch->phase = eTouchPhaseCancelled;
				break;
		} */
    } 
	
	touches->clear();
    for (std::vector<UITouch*>::const_iterator iter = uiTouches.begin(); iter != uiTouches.end(); ++iter)
		touches->push_back(touchMap_[*iter]);

	allTouches->clear();
    for (std::vector<UITouch*>::const_iterator iter = uiAllTouches.begin(); iter != uiAllTouches.end(); ++iter)
		allTouches->push_back(touchMap_[*iter]);
}


Touch* UITouchManager::newTouch()
{
	if (touchPool_.empty() == false)
	{
		Touch* touch = touchPool_.top();
		touchPool_.pop();
		return touch;
	}

	Touch* touch = new Touch;

	return touch;
}

void UITouchManager::deleteTouch(Touch* touch)
{
	touchPool_.push(touch);
}

