#ifndef GLCOMMON_H_INCLUDED
#define GLCOMMON_H_INCLUDED

#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define OPENGL_ES
#elif __ANDROID__
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#define OPENGL_DESKTOP
#endif

#ifdef OPENGL_DESKTOP
#include "glcompat.h"
#endif

#ifdef OPENGL_ES_1
//#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
//#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
//#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
#define glGenFramebuffers glGenFramebuffersOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glBindFramebuffer glBindFramebufferOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#endif

#define PREMULTIPLIED_ALPHA 1

#endif
