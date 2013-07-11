//
//  ES1Renderer.h
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "ESRenderer.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#include <vector>
#include <string>

class LuaApplication;
class Server;
class SoundImplementation;

@interface ES1Renderer : NSObject <ESRenderer>
{
@private
	EAGLContext *context;
	
	// The pixel dimensions of the CAEAGLLayer
	GLint backingWidth;
	GLint backingHeight;
	
	// The OpenGL names for the framebuffer and renderbuffer used to render to this view
	GLuint defaultFramebuffer, colorRenderbuffer;

	LuaApplication* application_;
	Server* server_;
	std::vector<std::string> fileList_;
	SoundImplementation* soundImplementation_;
}

- (void) render;
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;
- (LuaApplication*) getLuaApplication;


@end
