//
//  iphoneplayer2ViewController.h
//  iphoneplayer2
//
//  Created by Atilim Cetin on 12/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


class UITouchManager;
class LuaApplication;
class Server;
#include <vector>
#include <string>
class SoundImplementation;


@interface iphoneplayer2ViewController : UIViewController <UIAccelerometerDelegate>
{
    EAGLContext *context;
    GLuint program;
    
    BOOL animating;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;

	UITouchManager* uiTouchManager_;
	LuaApplication* application_;
	Server* server_;
	std::vector<std::string> fileList_;
	SoundImplementation* soundImplementation_;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawFrame;
- (void)exitGameLoop;

@end
