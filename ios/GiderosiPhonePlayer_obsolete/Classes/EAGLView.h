//
//  EAGLView.h
//  GiderosiPhonePlayer
//
//  Created by Atilim Cetin on 1/16/12.
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{
@private
    EAGLContext *context;
    
    // The pixel dimensions of the CAEAGLLayer.
    GLint framebufferWidth;
    GLint framebufferHeight;
    
    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view.
    GLuint defaultFramebuffer, colorRenderbuffer;

	BOOL retinaDisplay;
}

@property (nonatomic, retain) EAGLContext *context;

- (void)setFramebuffer;
- (BOOL)presentFramebuffer;
- (void)enableRetinaDisplay:(BOOL)enable;

@end
