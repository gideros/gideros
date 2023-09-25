#include <ginput.h>
#include <ginput-ios.h>
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
#import <CoreMotion/CoreMotion.h>
#endif
#if TARGET_OS_OSX
//TODO Use this for touchscreen/touchpad on OSX
#include <Carbon/Carbon.h>
@interface UITouch: NSObject
@end
#endif
#include <map>
#include <vector>
#include <gevent.h>
extern float gdr_ScaleFactor;

class GGInputManager;

#if (!TARGET_OS_TV && !TARGET_OS_OSX)
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
#endif
class GGInputManager
{
public:
    GGInputManager()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        accelerometer_ = [[GGAccelerometer alloc] init];

		if (NSClassFromString(@"CMMotionManager") != nil)
			motionManager_ = [[CMMotionManager alloc] init];
#endif
		accelerometerStartCount_ = 0;
		gyroscopeStartCount_ = 0;

        isMouseToTouchEnabled_ = 0;
        isTouchToMouseEnabled_ = 0;
        mouseTouchOrder_= 0;
        initKeyMap();
		
        touchPoolMutex_ = [[NSLock alloc] init];
        mousePoolMutex_ = [[NSLock alloc] init];
        keyPoolMutex_ = [[NSLock alloc] init];
        
        gevent_AddCallback(posttick_s, this);
        
        gid_ = g_NextId();
    }
    
    ~GGInputManager()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        [accelerometer_ release];
		if (motionManager_)
			[motionManager_ release];		
#endif
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
        
        [keyPoolMutex_ lock];
        for (size_t i = 0; i < keyPool1_.size(); ++i)
            delete keyPool1_[i];
        for (size_t i = 0; i < keyPool2_.size(); ++i)
            delete keyPool2_[i];
        [keyPoolMutex_ unlock];
        
        [keyPoolMutex_ release];
    }
    
    bool isAccelerometerAvailable()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        return true;
#else
        return false;
#endif
    }
    
    void startAccelerometer()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
		accelerometerStartCount_++;
		if (accelerometerStartCount_ == 1)
			[accelerometer_ start];
#endif
    }
    
    void stopAccelerometer()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
		if (accelerometerStartCount_ > 0)
		{
			accelerometerStartCount_--;
			if (accelerometerStartCount_ == 0)
				[accelerometer_ stop];
		}
#endif
    }
    
    void getAcceleration(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;

#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        if (accelerometerStartCount_ > 0)
        {
            x2 = accelerometer_.x;
            y2 = accelerometer_.y;
            z2 = accelerometer_.z;
        }
#endif
        
        if (x)
            *x = x2;
        if (y)
            *y = y2;
        if (z)
            *z = z2;
    }
	
	bool isGyroscopeAvailable()
    {
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        return motionManager_ && [motionManager_ isGyroAvailable];
#else
        return false;
#endif
    }
	
	void startGyroscope()
    {
		if (!isGyroscopeAvailable())
			return;
		
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
		gyroscopeStartCount_++;
		if (gyroscopeStartCount_ == 1)
			[motionManager_ startGyroUpdates];
#endif
    }
    
    void stopGyroscope()
    {
		if (!isGyroscopeAvailable())
			return;

#if (!TARGET_OS_TV && !TARGET_OS_OSX)
		if (gyroscopeStartCount_ > 0)
		{
			gyroscopeStartCount_--;
			if (gyroscopeStartCount_ == 0)
				[motionManager_ stopGyroUpdates];
		}
#endif
    }
	
	void getGyroscopeRotationRate(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;
		
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
        if (gyroscopeStartCount_ > 0)
        {
			CMRotationRate rotationRate = motionManager_.gyroData.rotationRate;
            x2 = rotationRate.x;
            y2 = rotationRate.y;
            z2 = rotationRate.z;
        }
#endif
        
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

        [keyPoolMutex_ lock];
        for (size_t i = 0; i < keyPool2_.size(); ++i)
            keyPool1_.push_back(keyPool2_[i]);
        keyPool2_.clear();
        [keyPoolMutex_ unlock];
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
#if !TARGET_OS_OSX
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
#else
        contentScaleFactor=gdr_ScaleFactor;
#endif
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
            touchEvent->touch.modifiers = 0;
            touchEvent->touch.mouseButton = 0;
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
                touchEvent->allTouches[i].modifiers = 0;
                touchEvent->allTouches[i].mouseButton = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0) {
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);
				mouseEvent->mouseType = 2; //Finger (assumed)
			}
            
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
#if !TARGET_OS_OSX
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
#else
        contentScaleFactor=gdr_ScaleFactor;
#endif
        
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
            touchEvent->touch.modifiers = 0;
            touchEvent->touch.mouseButton = 0;
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
                touchEvent->allTouches[i].modifiers = 0;
                touchEvent->allTouches[i].mouseButton = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }

            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0) {
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_NO_BUTTON);
				mouseEvent->mouseType = 2; //Finger (assumed)
			}
            
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
#if !TARGET_OS_OSX
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
#else
        contentScaleFactor=gdr_ScaleFactor;
#endif
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
            touchEvent->touch.modifiers = 0;
            touchEvent->touch.mouseButton = 0;
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
                touchEvent->allTouches[i].modifiers = 0;
                touchEvent->allTouches[i].mouseButton = 0;
                touchEvent->allTouches[i].touchType = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0) {
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);
				mouseEvent->mouseType = 2; //Finger (assumed)
			}

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
#if !TARGET_OS_OSX
        if ([view respondsToSelector:@selector(contentScaleFactor)] == YES)
            contentScaleFactor = view.contentScaleFactor;
#else
        contentScaleFactor=gdr_ScaleFactor;
#endif
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
            touchEvent->touch.modifiers = 0;
            touchEvent->touch.mouseButton = 0;
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
                touchEvent->allTouches[i].modifiers = 0;
                touchEvent->allTouches[i].mouseButton = 0;
                touchEvent->allTouches[i].id = addTouch(touch2);
                ++i;
            }
            
            ginput_MouseEvent *mouseEvent = NULL;
            if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0) {
                mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);
				mouseEvent->mouseType = 2; //Finger (assumed)
			}

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

    ginput_MouseEvent *newMouseEvent(int x, int y, int button, int mod=0)
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
        event->modifiers=mod;
        event->mouseType=0;
        
        return event;
    }
    
    void deleteMouseEvent(ginput_MouseEvent *event)
    {
        [mousePoolMutex_ lock];
        mousePool2_.push_back(event);
        [mousePoolMutex_ unlock];
    }
public:
    void mouseDown(int x, int y, int button,int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, button, mod);
        
        ginput_TouchEvent *touchEvent = NULL;
        if (isMouseToTouchEnabled_)
        {
            touchEvent = newTouchEvent(1);
            touchEvent->touch.x = x*gdr_ScaleFactor;
            touchEvent->touch.y = y*gdr_ScaleFactor;
            touchEvent->touch.id = 0;
            touchEvent->touch.pressure = 0;
            touchEvent->touch.touchType = 2;
            touchEvent->touch.modifiers = mod;
            touchEvent->touch.mouseButton=button;
            touchEvent->allTouches[0].x = x*gdr_ScaleFactor;
            touchEvent->allTouches[0].y = y*gdr_ScaleFactor;
            touchEvent->allTouches[0].id = 0;
            touchEvent->allTouches[0].pressure = 0;
            touchEvent->allTouches[0].touchType = 2;
            touchEvent->allTouches[0].modifiers = mod;
            touchEvent->allTouches[0].mouseButton=button;
        }
        
        if (mouseTouchOrder_ == 0)
        {
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            
        }
        else
        {
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
        }
    }
    
    void mouseMove(int x, int y, int button, int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, button, mod);
        
        ginput_TouchEvent *touchEvent = NULL;
        if (isMouseToTouchEnabled_)
        {
            touchEvent = newTouchEvent(1);
            touchEvent->touch.x = x*gdr_ScaleFactor;
            touchEvent->touch.y = y*gdr_ScaleFactor;
            touchEvent->touch.id = 0;
            touchEvent->touch.pressure = 0;
            touchEvent->touch.touchType = 2;
            touchEvent->touch.modifiers = mod;
            touchEvent->touch.mouseButton=button;
            touchEvent->allTouches[0].x = x*gdr_ScaleFactor;
            touchEvent->allTouches[0].y = y*gdr_ScaleFactor;
            touchEvent->allTouches[0].id = 0;
            touchEvent->allTouches[0].pressure = 0;
            touchEvent->allTouches[0].touchType = 2;
            touchEvent->allTouches[0].modifiers = mod;
            touchEvent->allTouches[0].mouseButton=button;
        }
        
        if (mouseTouchOrder_ == 0)
        {
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            
        }
        else
        {
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
        }
    }
    
    void mouseHover(int x, int y, int button, int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, button, mod);
        
        gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_HOVER_EVENT, mouseEvent, 0, this);
        deleteMouseEvent(mouseEvent);
    }
    
    void mouseUp(int x, int y, int button, int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, button, mod);
        
        ginput_TouchEvent *touchEvent = NULL;
        if (isMouseToTouchEnabled_)
        {
            touchEvent = newTouchEvent(1);
            touchEvent->touch.x = x*gdr_ScaleFactor;
            touchEvent->touch.y = y*gdr_ScaleFactor;
            touchEvent->touch.id = 0;
            touchEvent->touch.pressure = 0;
            touchEvent->touch.touchType = 2;
            touchEvent->touch.modifiers = mod;
            touchEvent->touch.mouseButton=button;
            touchEvent->allTouches[0].x = x*gdr_ScaleFactor;
            touchEvent->allTouches[0].y = y*gdr_ScaleFactor;
            touchEvent->allTouches[0].id = 0;
            touchEvent->allTouches[0].pressure = 0;
            touchEvent->allTouches[0].touchType = 2;
            touchEvent->allTouches[0].modifiers = mod;
            touchEvent->allTouches[0].mouseButton=button;
        }
        
        if (mouseTouchOrder_ == 0)
        {
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            
        }
        else
        {
            if (touchEvent)
            {
                gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
                deleteTouchEvent(touchEvent);
            }
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
            deleteMouseEvent(mouseEvent);
        }
    }
    
    void mouseWheel(int x, int y, int buttons,int delta, int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, buttons, mod);
        mouseEvent->wheel=delta;
        gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_WHEEL_EVENT, mouseEvent, 0, this);
        deleteMouseEvent(mouseEvent);
    }

    void mouseEnter(int x, int y, int buttons,int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor, y*gdr_ScaleFactor, buttons, mod);
        gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_ENTER_EVENT, mouseEvent, 0, this);
        deleteMouseEvent(mouseEvent);
    }

    void mouseLeave(int x, int y, int mod)
    {
        ginput_MouseEvent *mouseEvent = newMouseEvent(x*gdr_ScaleFactor,y*gdr_ScaleFactor,0,mod);
        gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_LEAVE_EVENT, mouseEvent, 0, this);
        deleteMouseEvent(mouseEvent);
    }

private:
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
    int keyDown(int realCode, int mods, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);
        
        if (repeatCount == 0)
        {
            ginput_KeyEvent *event = newKeyEvent(keyCode, realCode, mods);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_DOWN_EVENT, event, 0, this);
            deleteKeyEvent(event);
        }
        
        return 1;
    }
    
    int keyUp(int realCode, int mods, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);
        
        if (repeatCount == 0)
        {
            ginput_KeyEvent *event = newKeyEvent(keyCode, realCode, mods);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_UP_EVENT, event, 0, this);
            deleteKeyEvent(event);
        }
        
        return 1;
    }
    
    void keyChar(const char *keychar)
    {
        ginput_KeyEvent *event = newKeyEvent(0,0,-1);
        if (strlen(keychar)<(sizeof(event->charCode)))
        {
            strcpy(event->charCode,keychar);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_CHAR_EVENT, event, 0, this);
        }
        deleteKeyEvent(event);
    }
    
    
private:
    ginput_KeyEvent *newKeyEvent(int keyCode, int realCode, int modifiers)
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
        event->modifiers = modifiers;
        
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
        std::map<int, int>::const_iterator iter = keyMap_.find(keyCode);
        
        if (iter == keyMap_.end())
            return 0;
        
        return iter->second;
    }	
    std::vector<ginput_KeyEvent*> keyPool1_;
    std::vector<ginput_KeyEvent*> keyPool2_;
    std::map<int, int> keyMap_;
    void initKeyMap() {
        keyMap_[0x33]=8; //BS
#if TARGET_OS_OSX
        keyMap_[kVK_Escape]=GINPUT_KEY_ESC;
        keyMap_[kVK_Shift]=GINPUT_KEY_SHIFT;
        keyMap_[kVK_Space]=GINPUT_KEY_SPACE;
        keyMap_[kVK_Delete]=GINPUT_KEY_BACKSPACE;
        keyMap_[kVK_Control]=GINPUT_KEY_CTRL;
        keyMap_[kVK_Option]=GINPUT_KEY_ALT;
        keyMap_[kVK_Tab]=GINPUT_KEY_TAB;
        keyMap_[kVK_Return]=GINPUT_KEY_ENTER;

        keyMap_[kVK_F1]=GINPUT_KEY_F1;
        keyMap_[kVK_F2]=GINPUT_KEY_F2;
        keyMap_[kVK_F3]=GINPUT_KEY_F3;
        keyMap_[kVK_F4]=GINPUT_KEY_F4;
        keyMap_[kVK_F5]=GINPUT_KEY_F5;
        keyMap_[kVK_F6]=GINPUT_KEY_F6;
        keyMap_[kVK_F7]=GINPUT_KEY_F7;
        keyMap_[kVK_F8]=GINPUT_KEY_F8;
        keyMap_[kVK_F9]=GINPUT_KEY_F9;
        keyMap_[kVK_F10]=GINPUT_KEY_F10;
        keyMap_[kVK_F11]=GINPUT_KEY_F11;
        keyMap_[kVK_F12]=GINPUT_KEY_F12;
        keyMap_[kVK_ANSI_A]=GINPUT_KEY_A;
        keyMap_[kVK_ANSI_B]=GINPUT_KEY_B;
        keyMap_[kVK_ANSI_C]=GINPUT_KEY_C;
        keyMap_[kVK_ANSI_D]=GINPUT_KEY_D;
        keyMap_[kVK_ANSI_E]=GINPUT_KEY_E;
        keyMap_[kVK_ANSI_F]=GINPUT_KEY_F;
        keyMap_[kVK_ANSI_G]=GINPUT_KEY_G;
        keyMap_[kVK_ANSI_H]=GINPUT_KEY_H;
        keyMap_[kVK_ANSI_I]=GINPUT_KEY_I;
        keyMap_[kVK_ANSI_J]=GINPUT_KEY_J;
        keyMap_[kVK_ANSI_K]=GINPUT_KEY_K;
        keyMap_[kVK_ANSI_L]=GINPUT_KEY_L;
        keyMap_[kVK_ANSI_M]=GINPUT_KEY_M;
        keyMap_[kVK_ANSI_N]=GINPUT_KEY_N;
        keyMap_[kVK_ANSI_O]=GINPUT_KEY_O;
        keyMap_[kVK_ANSI_P]=GINPUT_KEY_P;
        keyMap_[kVK_ANSI_Q]=GINPUT_KEY_Q;
        keyMap_[kVK_ANSI_R]=GINPUT_KEY_R;
        keyMap_[kVK_ANSI_S]=GINPUT_KEY_S;
        keyMap_[kVK_ANSI_T]=GINPUT_KEY_T;
        keyMap_[kVK_ANSI_U]=GINPUT_KEY_U;
        keyMap_[kVK_ANSI_V]=GINPUT_KEY_V;
        keyMap_[kVK_ANSI_W]=GINPUT_KEY_W;
        keyMap_[kVK_ANSI_X]=GINPUT_KEY_X;
        keyMap_[kVK_ANSI_Y]=GINPUT_KEY_Y;
        keyMap_[kVK_ANSI_Z]=GINPUT_KEY_Z;
        keyMap_[kVK_ANSI_1]=GINPUT_KEY_1;
        keyMap_[kVK_ANSI_2]=GINPUT_KEY_2;
        keyMap_[kVK_ANSI_3]=GINPUT_KEY_3;
        keyMap_[kVK_ANSI_4]=GINPUT_KEY_4;
        keyMap_[kVK_ANSI_5]=GINPUT_KEY_5;
        keyMap_[kVK_ANSI_6]=GINPUT_KEY_6;
        keyMap_[kVK_ANSI_7]=GINPUT_KEY_7;
        keyMap_[kVK_ANSI_8]=GINPUT_KEY_8;
        keyMap_[kVK_ANSI_9]=GINPUT_KEY_9;
        keyMap_[kVK_ANSI_0]=GINPUT_KEY_0;
        
        keyMap_[kVK_Home]=GINPUT_KEY_HOME;
        keyMap_[kVK_End]=GINPUT_KEY_END;
        //keyMap_[kVK_Help]=GINPUT_KEY_INSERT;
        keyMap_[kVK_ForwardDelete]=GINPUT_KEY_DELETE;
        keyMap_[kVK_PageUp]=GINPUT_KEY_PAGEUP;
        keyMap_[kVK_PageDown]=GINPUT_KEY_PAGEDOWN;

        keyMap_[kVK_LeftArrow]=GINPUT_KEY_LEFT;
        keyMap_[kVK_RightArrow]=GINPUT_KEY_RIGHT;
        keyMap_[kVK_UpArrow]=GINPUT_KEY_UP;
        keyMap_[kVK_DownArrow]=GINPUT_KEY_DOWN;

	    keyMap_[kVK_ANSI_Keypad0] = GINPUT_KEY_NUM0;
	    keyMap_[kVK_ANSI_Keypad1] = GINPUT_KEY_NUM1;
	    keyMap_[kVK_ANSI_Keypad2] = GINPUT_KEY_NUM2;
	    keyMap_[kVK_ANSI_Keypad3] = GINPUT_KEY_NUM3;
	    keyMap_[kVK_ANSI_Keypad4] = GINPUT_KEY_NUM4;
	    keyMap_[kVK_ANSI_Keypad5] = GINPUT_KEY_NUM5;
	    keyMap_[kVK_ANSI_Keypad6] = GINPUT_KEY_NUM6;
	    keyMap_[kVK_ANSI_Keypad7] = GINPUT_KEY_NUM7;
	    keyMap_[kVK_ANSI_Keypad8] = GINPUT_KEY_NUM8;
	    keyMap_[kVK_ANSI_Keypad9] = GINPUT_KEY_NUM9;
	    keyMap_[kVK_ANSI_KeypadDivide] = GINPUT_KEY_NUMDIV;
	    keyMap_[kVK_ANSI_KeypadMultiply] = GINPUT_KEY_NUMMUL;
	    keyMap_[kVK_ANSI_KeypadMinus] = GINPUT_KEY_NUMSUB;
	    keyMap_[kVK_ANSI_KeypadPlus] = GINPUT_KEY_NUMADD;
	    keyMap_[kVK_ANSI_KeypadDecimal] = GINPUT_KEY_NUMDOT;
	    keyMap_[kVK_ANSI_KeypadEnter] = GINPUT_KEY_NUMENTER;
#endif
    }
private:
#if (!TARGET_OS_TV && !TARGET_OS_OSX)
    GGAccelerometer *accelerometer_;
    CMMotionManager *motionManager_;
#endif
	int accelerometerStartCount_;
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

g_bool ginputp_keyDown(int keyCode, int mods, int repeatCount)
{
    if (s_manager)
        return s_manager->keyDown(keyCode, mods, repeatCount);
    return g_false;
}
    
g_bool ginputp_keyUp(int keyCode, int mods, int repeatCount)
{
    if (s_manager)
        return s_manager->keyUp(keyCode, mods, repeatCount);
     return g_false;
}
    
void ginputp_keyChar(const char *keyChar)
{
     if (s_manager)
     s_manager->keyChar(keyChar);
}
    
    void ginputp_mouseDown(int x, int y, int button, int mod)
    {
        if (s_manager)
            s_manager->mouseDown(x, y, button, mod);
    }
    
    void ginputp_mouseMove(int x, int y, int button, int mod)
    {
        if (s_manager)
            s_manager->mouseMove(x, y, button, mod);
    }
    
    void ginputp_mouseHover(int x, int y, int button, int mod)
    {
        if (s_manager)
            s_manager->mouseHover(x, y, button, mod);
    }
    
    void ginputp_mouseUp(int x, int y, int button, int mod)
    {
        if (s_manager)
            s_manager->mouseUp(x, y, button, mod);
    }
    
    void ginputp_mouseWheel(int x, int y, int buttons, int delta, int mod)
    {
        if (s_manager)
            s_manager->mouseWheel(x, y, buttons,delta, mod);
    }

    void ginputp_mouseEnter(int x, int y, int buttons, int mod)
    {
        if (s_manager)
            s_manager->mouseEnter(x, y, buttons, mod);
    }

    void ginputp_mouseLeave(int x, int y, int mod)
    {
        if (s_manager)
            s_manager->mouseLeave(x,y,mod);
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

