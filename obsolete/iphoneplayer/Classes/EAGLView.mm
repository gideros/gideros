//
//  EAGLView.m
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "EAGLView.h"

#import "ES1Renderer.h"
#import "ES2Renderer.h"
#include "uitouchmanager.h"
#include "luaapplication.h"
#include "platform.h"
#include "binder.h"
#include <pthread.h>
#include <semaphore.h>


static pthread_mutex_t eventMutex;
static pthread_t gameThreadHandle;
static sem_t* sem;
//static EAGLContext* context;
//static LuaApplication* application;
static id <ESRenderer> g_renderer;
static bool gameLoopActive = true;


void* gameThread(void* args)
{
	printf("starting game thread\n");
	
	[NSThread setThreadPriority:0.5];

	sem_wait(sem);

	printf("starting game loop\n");
	
	while (gameLoopActive)
	{
		int deltaFrame = 1;

		sem_wait(sem);

		// drain any extra values in the semaphore
		while (sem_trywait(sem) != -1)
		{
			deltaFrame += 1;
		}
		
		pthread_mutex_lock(&eventMutex);
		[g_renderer render];
		pthread_mutex_unlock(&eventMutex);
	}

	printf("ending game loop\n");
	
	return NULL;
}


@implementation EAGLView

@synthesize animating;
@dynamic animationFrameInterval;

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id) initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		//renderer = [[ES2Renderer alloc] init];
		renderer = nil;
		
		if (!renderer)
		{
			renderer = [[ES1Renderer alloc] init];
			
			if (!renderer)
			{
				[self release];
				return nil;
			}
		}
		
		
		
						   
		Binder::disableTypeChecking();
		
		animating = FALSE;
		displayLinkSupported = FALSE;
		animationFrameInterval = 1;
		displayLink = nil;
		animationTimer = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;
		
		application_ = [renderer getLuaApplication];
		uiTouchManager_ = new UITouchManager;
		
		UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
		accelerometer.delegate = self;
		accelerometer.updateInterval = 1.0 / 60.0;
		
		g_renderer = renderer;
		if (pthread_mutex_init(&eventMutex, NULL) == -1)
		{
			perror("pthread_mutex_init");
		}
		[NSThread setThreadPriority:1.0];
		sem = sem_open("gameLoopSemaphore", O_CREAT, S_IRWXU, 0);
		pthread_create(&gameThreadHandle, NULL, gameThread, NULL);
		
    }
	
    return self;
}

- (void) drawView:(id)sender
{
//    [renderer render];
	sem_post(sem);
}

- (void) layoutSubviews
{
	[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void) startAnimation
{
	printf("startAnimation\n");
	if (!animating)
	{
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.

			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	printf("stopAnimation\n");
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		animating = FALSE;
	}
}

-(void)exitGameLoop
{
	gameLoopActive = false;
	sem_post(sem);
	void* thread_return = 0;
	pthread_join(gameThreadHandle, &thread_return);
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	pthread_mutex_lock(&eventMutex);

	NSArray* touchesArray = [touches allObjects];
	std::set<UITouch*> uiTouches;
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.insert(touch);
	}

	NSArray* allTouchesArray = [[event allTouches] allObjects];
	std::set<UITouch*> uiAllTouches;
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.insert(touch);
	}
	
	
	std::vector<Touch*> touchesSet, allTouchesSet;
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);

	try
	{
		application_->touchesBegan(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}

//    [self drawView];
	pthread_mutex_unlock(&eventMutex);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	pthread_mutex_lock(&eventMutex);

	NSArray* touchesArray = [touches allObjects];
	std::set<UITouch*> uiTouches;
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.insert(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	std::set<UITouch*> uiAllTouches;
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.insert(touch);
	}
	
	
	std::vector<Touch*> touchesSet, allTouchesSet;
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);

	try	
	{
		application_->touchesMoved(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
//    [self drawView];
	pthread_mutex_unlock(&eventMutex);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	pthread_mutex_lock(&eventMutex);

	NSArray* touchesArray = [touches allObjects];
	std::set<UITouch*> uiTouches;
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.insert(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	std::set<UITouch*> uiAllTouches;
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.insert(touch);
	}
	
	
	std::vector<Touch*> touchesSet, allTouchesSet;
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);

	try
	{
		application_->touchesEnded(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
//    [self drawView];
	pthread_mutex_unlock(&eventMutex);
}


- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	pthread_mutex_lock(&eventMutex);
	
	NSArray* touchesArray = [touches allObjects];
	std::set<UITouch*> uiTouches;
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.insert(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	std::set<UITouch*> uiAllTouches;
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.insert(touch);
	}
	
	
	std::vector<Touch*> touchesSet, allTouchesSet;
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
		
	try
	{
		application_->touchesCancelled(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
//    [self drawView];
	pthread_mutex_unlock(&eventMutex);
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	::setAccelerometer(acceleration.x, acceleration.y, acceleration.z);
}

- (void) dealloc
{
    [renderer release];

	delete uiTouchManager_;
	
    [super dealloc];
}

@end
