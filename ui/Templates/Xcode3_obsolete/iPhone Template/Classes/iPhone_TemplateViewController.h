//
//  iPhone_TemplateViewController.h
//  iPhone Template
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "EAGLView.h"

@interface iPhone_TemplateViewController : UIViewController<UIAccelerometerDelegate>
{
    EAGLContext *context;
    
    BOOL animating;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;
	
	EAGLView* glView;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, assign) EAGLView* glView;

- (void)startAnimation;
- (void)stopAnimation;

@end
