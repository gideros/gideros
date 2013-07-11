//
//  GiderosiPadPlayerViewController.h
//  GiderosiPadPlayer
//
//  Created by Atilim Cetin on 1/18/12.
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "EAGLView.h"

@interface GiderosiPadPlayerViewController : UIViewController<UIAccelerometerDelegate>
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
