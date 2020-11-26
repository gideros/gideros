//
//  EAGLView.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//
#import <Cocoa/Cocoa.h>

#import "Metal/Metal.h"
extern id<MTLDevice> metalDevice;
extern MTLRenderPassDescriptor *metalFramebuffer;

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : NSView <NSTextInput>
{
@private
    // The pixel dimensions of the CAEAGLLayer.
    GLint framebufferWidth;
    GLint framebufferHeight;
    
    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view.
    GLuint defaultFramebuffer, colorRenderbuffer;

	BOOL framebufferDirty;
	BOOL retinaDisplay;
    CAMetalLayer *metalLayer;
    id<CAMetalDrawable> metalDrawable;
    CGRect safeArea;
    id<MTLTexture> metalDepth;
    id<MTLTexture> metalStencil;
}

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;
- (void)enableRetinaDisplay:(BOOL)enable;
- (void) setup;
- (void) tearDown;

@end
