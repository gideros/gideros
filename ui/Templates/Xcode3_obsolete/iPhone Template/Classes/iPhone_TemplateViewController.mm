//
//  iPhone_TemplateViewController.mm
//  iPhone Template
//

#import <QuartzCore/QuartzCore.h>

#import "iPhone_TemplateViewController.h"
#import "EAGLView.h"

#include "../giderosapi.h"

@interface iPhone_TemplateViewController ()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
@end

@implementation iPhone_TemplateViewController

@synthesize animating, context, displayLink, glView;

- (void)loadView
{
	self.glView = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	self.glView.clearsContextBeforeDrawing = NO;
	self.glView.multipleTouchEnabled = YES;
	self.glView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	
	UIView* rootView = [[UIView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];       
	[rootView addSubview:glView];   
	
	self.view = rootView;
}

- (void)viewDidLoad
{
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
	self.context = aContext;
	[aContext release];
	
    [(EAGLView *)self.glView setContext:context];
    [(EAGLView *)self.glView setFramebuffer];

    animating = FALSE;
    animationFrameInterval = 1;
    self.displayLink = nil;

	UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
	accelerometer.delegate = self;
	accelerometer.updateInterval = 1.0 / 60.0;
}

- (void)dealloc
{
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];
    
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];

    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
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

- (void)startAnimation
{
    if (!animating)
    {
		CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
	gdr_drawFrame();
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
	gdr_didReceiveMemoryWarning();
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	gdr_touchesBegan(touches, [event allTouches]);
}
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	gdr_touchesMoved(touches, [event allTouches]);
}
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	gdr_touchesEnded(touches, [event allTouches]);
}
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	gdr_touchesCancelled(touches, [event allTouches]);
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	gdr_setAccelerometer(acceleration.x, acceleration.y, acceleration.z);
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return gdr_shouldAutorotateToInterfaceOrientation(interfaceOrientation);
}

-(void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	gdr_willRotateToInterfaceOrientation(toInterfaceOrientation);
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	gdr_didRotateFromInterfaceOrientation(fromInterfaceOrientation);
}


@end
