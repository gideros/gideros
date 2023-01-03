//
//  EAGLView.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import "Metal/Metal.h"
extern id<MTLDevice> metalDevice;
extern MTLRenderPassDescriptor *metalFramebuffer;

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : NSView
{
@private
    NSUInteger framebufferWidth;
    NSUInteger framebufferHeight;
 	BOOL framebufferDirty;
	BOOL retinaDisplay;
    CAMetalLayer *metalLayer;
    id<CAMetalDrawable> metalDrawable;
    CGRect safeArea;
    id<MTLTexture> metalDepth;
    NSEventModifierFlags modifiers;
    CGFloat contentScaleFator;
}

@property (nonatomic) CGFloat contentScaleFactor;

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;
- (void)enableRetinaDisplay:(BOOL)enable scalePtr:(float *)scale;
- (void) setup;
- (void) tearDown;
-(void) resized;
- (BOOL) isFlipped;
- (BOOL) acceptsFirstResponder;

@end
