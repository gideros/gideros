//
//  EAGLView.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : NSOpenGLView <NSTextInputClient>
{
@private
    NSOpenGLContext *context;
    NSMutableAttributedString *markedText;
    
    // The pixel dimensions of the CAEAGLLayer.
    GLint framebufferWidth;
    GLint framebufferHeight;
    
    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view.
    GLuint defaultFramebuffer, colorRenderbuffer;

	BOOL framebufferDirty;
	BOOL retinaDisplay;
}

@property (nonatomic, retain) NSOpenGLContext *context;
@property (nonatomic, retain) NSMutableAttributedString *markedText;

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;
- (void)enableRetinaDisplay:(BOOL)enable;

@end
