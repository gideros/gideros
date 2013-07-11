#ifndef GLCOMMON_H_INCLUDED
#define GLCOMMON_H_INCLUDED

#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#define OPENGL_ES
#elif __ANDROID__
#include <GLES/gl.h>
#include <GLES/glext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#define OPENGL_DESKTOP
#endif

#ifdef OPENGL_DESKTOP
#include "glcompat.h"
#endif

#ifdef OPENGL_ES
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
#define glGenFramebuffers glGenFramebuffersOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glBindFramebuffer glBindFramebufferOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#endif

#define PREMULTIPLIED_ALPHA 1

#endif
