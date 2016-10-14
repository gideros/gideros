#if TARGET_OS_TV == 0
#include <ginput.h>
#include <ginput-ios.h>
#include <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#include <map>
#include <vector>
#include <gevent.h>

class GGInputManager;

@interface GGAccelerometer : NSObject<UIAccelerometerDelegate>
{
    UIAccelerometer *accelerometer_;
	BOOL isRunning_;
}

@property (nonatomic, readonly) UIAccelerationValue x;
@property (nonatomic, readonly) UIAccelerationValue y;
@property (nonatomic, readonly) UIAccelerationValue z;

@end

@implementation GGAccelerometer

@synthesize x = x_;
@synthesize y = y_;
@synthesize z = z_;

- (id)init
{
    if (self = [super init])
    {
        x_ = y_ = z_ = 0;
        accelerometer_ = [UIAccelerometer sharedAccelerometer];
		isRunning_ = NO;
    }
    
    return self;
}

- (void)start
{
	if (isRunning_)
		return;
	
	accelerometer_.delegate = self;
	accelerometer_.updateInterval = 1.0 / 60.0;
	isRunning_ = YES;
}

- (void)stop
{
	if (!isRunning_)
		return;

	accelerometer_.updateInterval = 0.1;
	accelerometer_.delegate = nil;
	isRunning_ = NO;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
    x_ = acceleration.x;
    y_ = acceleration.y;
    z_ = acceleration.z;
}

- (void)dealloc
{
	[self stop];
    [super dealloc];
}

@end


class GGInputManager
{
public:
    GGInputManager()
    {
        accelerometer_ = [[GGAccelerometer alloc] init];

		if (NSClassFromString(@"CMMotionManager") != nil)
			motionManager_ = [[CMMotionManager alloc] init];
		
		accelerometerStartCount_ = 0;
		gyroscopeStartCount_ = 0;

        isMouseToTouchEnabled_ = 0;
        isTouchToMouseEnabled_ = 0;
        mouseTouchOrder_= 0;
		
        touchPoolMutex_ = [[NSLock alloc] init];
		mousePoolMutex_ = [[NSLock alloc] init];
        
        gevent_AddCallback(posttick_s, this);
        
        gid_ = g_NextId();
    }
    
    ~GGInputManager()
    {
        [accelerometer_ release];
		if (motionManager_)
			[motionManager_ release];		

        gevent_RemoveCallbackWithGid(gid_);
        
        gevent_RemoveCallback(posttick_s, this);
		
		[touchPoolMutex_ lock];
		std::map<size_t, std::vector<ginput_TouchEvent*> >::iterator iter;
		for (iter = touchPool1_.begin(); iter != touchPool1_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
			{
				delete [] v[i]->allTouches;
				delete v[i];
			}
		}
		for (iter = touchPool2_.begin(); iter != touchPool2_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
			{
				delete [] v[i]->allTouches;
				delete v[i];
			}
		}
		[touchPoolMutex_ unlock];
        
        [touchPoolMutex_ release];
	
        
        [mousePoolMutex_ lock];
        for (size_t i = 0; i < mousePool1_.size(); ++i)
            delete mousePool1_[i];
        for (size_t i = 0; i < mousePool2_.size(); ++i)
            delete mousePool2_[i];
		[mousePoolMutex_ unlock];
        
        [mousePoolMutex_ release];
    }
    
    bool isAccelerometerAvailable()
    {
        return true;
    }
    
    void startAccelerometer()
    {
		accelerometerStartCount_++;
		if (accelerometerStartCount_ == 1)
			[accelerometer_ start];
    }
    
    void stopAccelerometer()
    {
		if (accelerometerStartCount_ > 0)
		{
			accelerometerStartCount_--;
			if (accelerometerStartCount_ == 0)
				[accelerometer_ stop];
		}
    }
    
    void getAcceleration(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;

        if (accelerometerStartCount_ > 0)
        {
            x2 = accelerometer_.x;
            y2 = accelerometer_.y;
            z2 = accelerometer_.z;
        }
        
        if (x)
            *x = x2;
        if (y)
            *y = y2;
        if (z)
            *z = z2;
    }
	
	bool isGyroscopeAvailable()
    {
        return motionManager_ && [motionManager_ isGyroAvailable];
    }
	
	void startGyroscope()
    {
		if (!isGyroscopeAvailable())
			return;
		
		gyroscopeStartCount_++;
		if (gyroscopeStartCount_ == 1)
			[motionManager_ startGyroUpdates];
    }
    
    void stopGyroscope()
    {
		if (!isGyroscopeAvailable())
			return;

		if (gyroscopeStartCount_ > 0)
		{
			gyroscopeStartCount_--;
			if (gyroscopeStartCount_ == 0)
				[motionManager_ stopGyroUpdates];
		}
    }
	
	void getGyroscopeRotationRate(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;
		
        if (gyroscopeStartCount_ > 0)
        {
			CMRotationRate rotationRate = motionManager_.gyroData.rotationRate;
            x2 = rotationRate.x;
            y2 = rotationRate.y;
            z2 = rotationRate.z;
        }
        
        if (x)
            *x = x2;
        if (y)
            *y = y2;
        if (z)
            *z = z2;
    }
private:
    static void posttick_s(int type, void *event, void *udata)
    {
        if (type == GEVENT_POST_TICK_EVENT)
            static_cast<GGInputManager*>(udata)->posttick();
    }
    
    void posttick()
    {
		[touchPoolMutex_ lock];
		std::map<size_t, std::vector<ginput_TouchEvent*> >::iterator iter;
		for (iter = touchPool2_.begin(); iter != touchPool2_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
				touchPool1_[iter->first].push_back(v[i]);
		}
		touchPool2_.clear();
		[touchPoolMutex_ unlock];

		[mousePoolMutex_ lock];
        for (size_t i = 0; i < mousePool2_.size(); ++i)
            mousePool1_.push_back(mousePool2_[i]);
        mousePool2_.clear();
		[mousePoolMutex_ unlock];
    }
    
public:
    void setMouseToTouchEnabled(int enabled)
    {
        isMouseToTouchEnabled_ = enabled;
    }
    
    void setTouchToMouseEnabled(int enabled)
    {
        isTouchToMouseEnabled_ = enabled;
    }
    
    void setMouseTouchOrder(int order)
    {
        mouseTouchOrder_ = order;
    }

public:
    void touchesBegan(NSSet *touches, NSSet *allTouches, UIView *view)
    {
        float contentScaleFactor = 1;
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
            
        bool has3Dtouch = false;
        #if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
        if([[view traitCollection] forceTouchCapability] == UIForceTouchCapabilityAvailable)
            has3Dtouch = true;
        #endif
        for (UITouch *touch in touches)
        {
            ginput_TouchEvent *touchEvent = newTouchEvent(allTouches.count);
            
            CGPoint location = [touch locationInView:view];
            touchEvent->touch.x = location.x * contentScaleFactor;
            touchEvent->touch.y = location.y * contentScaleFactor;
            touchEvent->touch.pressure = 0;
            
            #if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
            if(has3Dtouch)
                touchEvent->touch.pressure = touch.force/touch.maximumPossibleForce;
            #endif

            touchEvent->touch.touchType = 0;
            touchEvent->touch.id = addTouch(touch);
            
            int i = 0;
            for (UITouch *touch2 in allTouches)
            {
                CGPoint location = [touch2 locationInView:view];
                touchEvent->allTouches[i].x = location.x * contentScaleFactor;
                touchEvent->allTouches[i].y = location.y * contentScaleFactor;
                touchEvent->allTouches[i].pressure = 0;
                #if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
                if(has3Dtouch)
                    touchEvent->allTouches[i].pressure = touch2.force/touch2.maximumPossibleForce;
                #endif

                touchEvent->allTouches[i].touchType = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);
            
            if (mouseTouchOrder_ == 0)
            {
                if (mouseEvent)
                {
                    gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
                    deleteMouseEvent(mouseEvent);
                }
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            else
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
                if (mouseEvent)
                {
                    gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
                    deleteMouseEvent(mouseEvent);
                }
            }
        }
    }
    void touchesMoved(NSSet *touches, NSSet *allTouches, UIView *view)
    {
        float contentScaleFactor = 1;
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
        
        bool has3Dtouch = false;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
        if([[view traitCollection] forceTouchCapability] == UIForceTouchCapabilityAvailable)
            has3Dtouch = true;
#endif

        
        for (UITouch *touch in touches)
        {
            ginput_TouchEvent *touchEvent = newTouchEvent(allTouches.count);
            
            CGPoint location = [touch locationInView:view];
            touchEvent->touch.x = location.x * contentScaleFactor;
            touchEvent->touch.y = location.y * contentScaleFactor;
            touchEvent->touch.pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
            if(has3Dtouch)
                touchEvent->touch.pressure = touch.force/touch.maximumPossibleForce;
#endif
            touchEvent->touch.touchType = 0;
            touchEvent->touch.id = addTouch(touch);
            
            int i = 0;
            for (UITouch *touch2 in allTouches)
            {
                CGPoint location = [touch2 locationInView:view];
                touchEvent->allTouches[i].x = location.x * contentScaleFactor;
                touchEvent->allTouches[i].y = location.y * contentScaleFactor;
                touchEvent->allTouches[i].pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
                if(has3Dtouch)
                    touchEvent->allTouches[i].pressure = touch2.force/touch2.maximumPossibleForce;
#endif
                touchEvent->allTouches[i].touchType = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }

            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_NO_BUTTON);
            
			if (mouseTouchOrder_ == 0)
			{
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
			}
			else
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
			}
        }
    }
    void touchesEnded(NSSet *touches, NSSet *allTouches, UIView *view)
    {
        float contentScaleFactor = 1;
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
        
        bool has3Dtouch = false;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
        if([[view traitCollection] forceTouchCapability] == UIForceTouchCapabilityAvailable)
            has3Dtouch = true;
#endif

        
        for (UITouch *touch in touches)
        {
            ginput_TouchEvent *touchEvent = newTouchEvent(allTouches.count);
            
            CGPoint location = [touch locationInView:view];
            touchEvent->touch.x = location.x * contentScaleFactor;
            touchEvent->touch.y = location.y * contentScaleFactor;
            touchEvent->touch.pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
            if(has3Dtouch)
                touchEvent->touch.pressure = touch.force/touch.maximumPossibleForce;
#endif
            touchEvent->touch.touchType = 0;
            touchEvent->touch.id = addTouch(touch);
            
            int i = 0;
            for (UITouch *touch2 in allTouches)
            {
                CGPoint location = [touch2 locationInView:view];
                touchEvent->allTouches[i].x = location.x * contentScaleFactor;
                touchEvent->allTouches[i].y = location.y * contentScaleFactor;
                touchEvent->allTouches[i].pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
                if(has3Dtouch)
                    touchEvent->allTouches[i].pressure = touch2.force/touch2.maximumPossibleForce;
#endif
                touchEvent->allTouches[i].touchType = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);

            if (mouseTouchOrder_ == 0)
            {
                if (mouseEvent)
                {
                    gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
                    deleteMouseEvent(mouseEvent);
                }
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            else
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
                if (mouseEvent)
                {
                    gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
                    deleteMouseEvent(mouseEvent);
                }
            }
        }
        
        for (UITouch *touch in touches)
            removeTouch(touch);    
    }

    void touchesCancelled(NSSet *touches, NSSet *allTouches, UIView *view)
    {
        float contentScaleFactor = 1;
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
        
        bool has3Dtouch = false;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
        if([[view traitCollection] forceTouchCapability] == UIForceTouchCapabilityAvailable)
            has3Dtouch = true;
#endif

        
        for (UITouch *touch in touches)
        {
            ginput_TouchEvent *touchEvent = newTouchEvent(allTouches.count);
            
            CGPoint location = [touch locationInView:view];
            touchEvent->touch.x = location.x * contentScaleFactor;
            touchEvent->touch.y = location.y * contentScaleFactor;
            touchEvent->touch.pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
            if(has3Dtouch)
                touchEvent->touch.pressure = touch.force/touch.maximumPossibleForce;
#endif
            touchEvent->touch.touchType = 0;
            touchEvent->touch.id = addTouch(touch);
            
            int i = 0;
            for (UITouch *touch2 in allTouches)
            {
                CGPoint location = [touch2 locationInView:view];
                touchEvent->allTouches[i].x = location.x * contentScaleFactor;
                touchEvent->allTouches[i].y = location.y * contentScaleFactor;
                touchEvent->allTouches[i].pressure = 0;
#if __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_8_2
                if(has3Dtouch)
                    touchEvent->allTouches[i].pressure = touch2.force/touch2.maximumPossibleForce;
#endif
                touchEvent->allTouches[i].touchType = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);

			if (mouseTouchOrder_ == 0)
			{
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_CANCEL_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
			}
			else
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_CANCEL_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
			}
        }
        
        for (UITouch *touch in touches)
            removeTouch(touch);
    }

private:
    int addTouch(UITouch *touch)
    {
        for (int i = 0; i < touches_.size(); ++i)
            if (touches_[i] == touch)
                return i;

        for (int i = 0; i < touches_.size(); ++i)
            if (touches_[i] == NULL)
            {
                touches_[i] = touch;
                return i;
            }

        touches_.push_back(touch);
        
        return touches_.size() - 1;
    }

    void removeTouch(UITouch *touch)
    {
        for (int i = 0; i < touches_.size(); ++i)
            if (touches_[i] == touch)
            {
                touches_[i] = NULL;
                break;
            }        
    }

    ginput_TouchEvent *newTouchEvent(size_t allTouchesCount)
	{
		[touchPoolMutex_ lock];
		std::vector<ginput_TouchEvent*> &pool = touchPool1_[allTouchesCount];
        
        ginput_TouchEvent *event;
        
        if (pool.empty())
        {
            event = new ginput_TouchEvent;
			event->allTouches = new ginput_Touch[allTouchesCount];
        }
        else
        {
            event = pool.back();
            pool.pop_back();
        }
		[touchPoolMutex_ unlock];
        
        event->allTouchesCount = allTouchesCount;
        
        return event;
	}
    
	void deleteTouchEvent(ginput_TouchEvent *event)
	{
		[touchPoolMutex_ lock];
		touchPool2_[event->allTouchesCount].push_back(event);
		[touchPoolMutex_ unlock];
	}

    ginput_MouseEvent *newMouseEvent(int x, int y, int button)
    {
        [mousePoolMutex_ lock];
        ginput_MouseEvent *event;
        
        if (mousePool1_.empty())
        {
            event = new ginput_MouseEvent;
        }
        else
        {
            event = mousePool1_.back();
            mousePool1_.pop_back();
        }
        [mousePoolMutex_ unlock];
        
        event->x = x;
        event->y = y;
        event->button = button;
        event->wheel = 0;        
        
        return event;
    }
    
    void deleteMouseEvent(ginput_MouseEvent *event)
    {
        [mousePoolMutex_ lock];
        mousePool2_.push_back(event);
        [mousePoolMutex_ unlock];
    }
    
    std::vector<UITouch*> touches_;
	std::map<size_t, std::vector<ginput_TouchEvent*> > touchPool1_;
	std::map<size_t, std::vector<ginput_TouchEvent*> > touchPool2_;
    std::vector<ginput_MouseEvent*> mousePool1_;
    std::vector<ginput_MouseEvent*> mousePool2_;
    NSLock *touchPoolMutex_;
    NSLock *mousePoolMutex_;
    NSLock *keyPoolMutex_;
    
    int isMouseToTouchEnabled_;
    int isTouchToMouseEnabled_;
    int mouseTouchOrder_;

public:
    int keyDown(int realCode, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);
        
        if (repeatCount == 0)
        {
            ginput_KeyEvent *event = newKeyEvent(keyCode, realCode);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_DOWN_EVENT, event, 0, this);
            deleteKeyEvent(event);
        }
        
        return 1;
    }
    
    int keyUp(int realCode, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);
        
        if (repeatCount == 0)
        {
            ginput_KeyEvent *event = newKeyEvent(keyCode, realCode);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_UP_EVENT, event, 0, this);
            deleteKeyEvent(event);
        }
        
        return 1;
    }
    
    void keyChar(const char *keychar)
    {
        ginput_KeyEvent *event = newKeyEvent(0,0);
        if (strlen(keychar)<(sizeof(event->charCode)))
        {
            strcpy(event->charCode,keychar);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_CHAR_EVENT, event, 0, this);
        }
        deleteKeyEvent(event);
    }
    
    
private:
    ginput_KeyEvent *newKeyEvent(int keyCode, int realCode)
    {
        [keyPoolMutex_ lock];
        ginput_KeyEvent *event;
        
        if (keyPool1_.empty())
        {
            event = new ginput_KeyEvent;
        }
        else
        {
            event = keyPool1_.back();
            keyPool1_.pop_back();
        }
        [keyPoolMutex_ unlock];
        
        event->keyCode = keyCode;
        event->realCode = realCode;
        
        return event;
    }
    
    void deleteKeyEvent(ginput_KeyEvent *event)
    {
        [keyPoolMutex_ lock];
        keyPool2_.push_back(event);
        [keyPoolMutex_ unlock];
    }
    
    int convertKeyCode(int keyCode)
    {
        return keyCode;
/*        std::map<int, int>::const_iterator iter = keyMap_.find(keyCode);
        
        if (iter == keyMap_.end())
            return 0;
        
        return iter->second;*/
    }	
    std::vector<ginput_KeyEvent*> keyPool1_;
    std::vector<ginput_KeyEvent*> keyPool2_;
    //std::map<int, int> keyMap_;

private:
    GGAccelerometer *accelerometer_;
	int accelerometerStartCount_;
	CMMotionManager *motionManager_;
	int gyroscopeStartCount_;

public:
    g_id addCallback(gevent_Callback callback, void *udata)
    {
        return callbackList_.addCallback(callback, udata);
    }
    
    void removeCallback(gevent_Callback callback, void *udata)
    {
        callbackList_.removeCallback(callback, udata);
    }
    
    void removeCallbackWithGid(g_id gid)
    {
        callbackList_.removeCallbackWithGid(gid);
    }
    
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGInputManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};

static GGInputManager *s_manager = NULL;

extern "C" {

void ginput_init()
{
    s_manager = new GGInputManager;    
}

void ginput_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

int ginput_isAccelerometerAvailable()
{
    return s_manager->isAccelerometerAvailable();
}

void ginput_startAccelerometer()
{
    s_manager->startAccelerometer();
}

void ginput_stopAccelerometer()
{
    s_manager->stopAccelerometer();    
}

void ginput_getAcceleration(double *x, double *y, double *z)
{
    s_manager->getAcceleration(x, y, z);
}

int ginput_isGyroscopeAvailable()
{
    return s_manager->isGyroscopeAvailable();
}

void ginput_startGyroscope()
{
    s_manager->startGyroscope();
}

void ginput_stopGyroscope()
{
    s_manager->stopGyroscope();    
}

void ginput_getGyroscopeRotationRate(double *x, double *y, double *z)
{
    s_manager->getGyroscopeRotationRate(x, y, z);
}

void ginputp_touchesBegan(NSSet *touches, NSSet *allTouches, UIView *view)
{
    if (s_manager)
        s_manager->touchesBegan(touches, allTouches, view);
}

void ginputp_touchesMoved(NSSet *touches, NSSet *allTouches, UIView *view)
{
    if (s_manager)
        s_manager->touchesMoved(touches, allTouches, view);
}

void ginputp_touchesEnded(NSSet *touches, NSSet *allTouches, UIView *view)
{
    if (s_manager)
        s_manager->touchesEnded(touches, allTouches, view);
}

void ginputp_touchesCancelled(NSSet *touches, NSSet *allTouches, UIView *view)
{
    if (s_manager)
        s_manager->touchesCancelled(touches, allTouches, view);
}

g_bool ginputp_keyDown(int keyCode, int repeatCount)
{
    if (s_manager)
        return s_manager->keyDown(keyCode, repeatCount);
    return g_false;
}
    
g_bool ginputp_keyUp(int keyCode, int repeatCount)
{
    if (s_manager)
        return s_manager->keyUp(keyCode, repeatCount);
     return g_false;
}
    
void ginputp_keyChar(const char *keyChar)
{
     if (s_manager)
     s_manager->keyChar(keyChar);
}
    

void ginput_setMouseToTouchEnabled(int enabled)
{
    s_manager->setMouseToTouchEnabled(enabled);
}
    
void ginput_setTouchToMouseEnabled(int enabled)
{
    s_manager->setTouchToMouseEnabled(enabled);
}
    
void ginput_setMouseTouchOrder(int order)
{
    s_manager->setMouseTouchOrder(order);
}

g_id ginput_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void ginput_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void ginput_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

}
#endif
