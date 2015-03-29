#ifndef GLCOMMON_H_INCLUDED
#define GLCOMMON_H_INCLUDED
#define GIDEROS_GL1 1
#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifdef GIDEROS_GL1
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#else
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
#endif
#define OPENGL_ES
#elif __ANDROID__
#ifdef GIDEROS_GL1
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#else
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif
#define OPENGL_ES
#else
#include <GL/glew.h>
#define OPENGL_DESKTOP
#endif

#ifdef OPENGL_DESKTOP
#include "glcompat.h"
#endif

#ifdef OPENGL_ES
#ifdef GIDEROS_GL1
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
#define glGenFramebuffers glGenFramebuffersOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glBindFramebuffer glBindFramebufferOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#endif
#endif

#define PREMULTIPLIED_ALPHA 1

#endif
