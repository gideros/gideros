//
//  EAGLView.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "Metal/Metal.h"
extern id<MTLDevice> metalDevice;
extern MTLRenderPassDescriptor *metalFramebuffer;

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView <UIKeyInput>
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
}

@property (nonatomic, readonly) BOOL hasText;
@property (nonatomic) UITextAutocorrectionType autocorrectionType;         // default is UITextAutocorrectionTypeDefault

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;
- (void)enableRetinaDisplay:(BOOL)enable;
- (void) setup;
- (void) tearDown;

@end
