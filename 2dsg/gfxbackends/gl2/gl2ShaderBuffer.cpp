/*
 * gl2ShaderBuffer.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gl2Shaders.h"
#include "string.h"

int ogl2ShaderBuffer::qualcommFix_=-1;

GLint ogl2ShaderBuffer::bindBuffer(GLint fbo)
{
	GLCALL_INIT;
	GLint oldFBO=0;
	GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
#ifdef OPENGL_DESKTOP
        if (GLEW_ARB_framebuffer_object)
#endif
			GLCALL glBindFramebuffer(GL_FRAMEBUFFER, fbo);
#ifdef OPENGL_DESKTOP
		else
			GLCALL glBindFramebufferEXT(GL_FRAMEBUFFER, fbo);
#endif
        return oldFBO;
}

ogl2ShaderBuffer::ogl2ShaderBuffer(ShaderTexture *texture,bool forDepth)
{
	GLCALL_INIT;
	_depthRenderBuffer=0;
	forDepth_=forDepth;
	width=((ogl2ShaderTexture *)texture)->width;
	height=((ogl2ShaderTexture *)texture)->height;

    if (qualcommFix_ == -1)
    {
        const char *extensions = (const char *)GLCALL glGetString(GL_EXTENSIONS);
        qualcommFix_ = (extensions && strstr(extensions, "GL_QCOM"));
    }

    if (qualcommFix_)
        tempTexture_ = gtexture_TempTextureCreate(((ogl2ShaderTexture *)texture)->width, ((ogl2ShaderTexture *)texture)->height);
    else
        tempTexture_ = 0;


    textureId_=((ogl2ShaderTexture *)texture)->glid;

    GLint oldFBO = 0;
    GLCALL glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
#ifdef OPENGL_DESKTOP
    if (GLEW_ARB_framebuffer_object)
    {
#endif

    	GLCALL glGenFramebuffers(1, &glid);
    	GLCALL glBindFramebuffer(GL_FRAMEBUFFER, glid);

    	GLCALL glFramebufferTexture2D(GL_FRAMEBUFFER, forDepth?GL_DEPTH_ATTACHMENT:GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId_, 0);

    	GLCALL glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

#ifdef OPENGL_DESKTOP
    }
	else
	{

		GLCALL glGenFramebuffersEXT(1, &glid);
		GLCALL glBindFramebufferEXT(GL_FRAMEBUFFER, glid);

		GLCALL glFramebufferTexture2DEXT(GL_FRAMEBUFFER, forDepth?GL_DEPTH_ATTACHMENT:GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId_, 0);

		GLCALL glBindFramebufferEXT(GL_FRAMEBUFFER, oldFBO);
	}
#endif
}

ogl2ShaderBuffer::~ogl2ShaderBuffer()
{
	GLCALL_INIT;
#ifdef OPENGL_DESKTOP
    if (GLEW_ARB_framebuffer_object)
#endif
    	GLCALL glDeleteFramebuffers(1,&glid);
#ifdef OPENGL_DESKTOP
	else
		GLCALL glDeleteFramebuffersEXT(1,&glid);
#endif
    if (tempTexture_)
        gtexture_TempTextureDelete(tempTexture_);
}

void ogl2ShaderBuffer::prepareDraw()
{
	GLCALL_INIT;
    if (qualcommFix_)
    {
    	GLCALL glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ((ogl2ShaderTexture *)gtexture_TempTextureGetName(tempTexture_))->glid, 0);
    	GLCALL glClear(GL_COLOR_BUFFER_BIT);
    	GLCALL glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId_, 0);
    }
}

void ogl2ShaderBuffer::needDepthStencil()
{
	if (forDepth_) return;
	GLCALL_INIT;
	int depthfmt = 0;
#ifdef __EMSCRIPTEN__
        depthfmt=0x84F9;//GL_DEPTH_STENCIL;
#else
#ifdef GL_DEPTH24_STENCIL8_OES
	depthfmt=GL_DEPTH24_STENCIL8_OES;
#else
	depthfmt = GL_DEPTH24_STENCIL8;
#endif
#endif
#ifdef OPENGL_DESKTOP
     if (GLEW_ARB_framebuffer_object)
     {
#endif
	if (!GLCALL glIsRenderbuffer(_depthRenderBuffer))
	{
		GLCALL glGenRenderbuffers(1, &_depthRenderBuffer);
		GLCALL glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
		GLCALL glRenderbufferStorage(GL_RENDERBUFFER, depthfmt, width,height);
		GLCALL glBindRenderbuffer(GL_RENDERBUFFER, 0);
#ifdef __EMSCRIPTEN__
#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT	0x821A
#endif
		GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
#else
		GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
		GLCALL glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
#endif
	}
#ifdef OPENGL_DESKTOP
     }
	else {
		if (!GLCALL glIsRenderbufferEXT(_depthRenderBuffer))
		{
			GLCALL glGenRenderbuffersEXT(1, &_depthRenderBuffer);
			GLCALL glBindRenderbufferEXT(GL_RENDERBUFFER, _depthRenderBuffer);
			GLCALL glRenderbufferStorageEXT(GL_RENDERBUFFER, depthfmt, width,height);
			GLCALL glBindRenderbufferEXT(GL_RENDERBUFFER, 0);
			if (!forDepth_)
				GLCALL glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
			GLCALL glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
		}
	}
#endif

}

void ogl2ShaderBuffer::unbound()
{
	GLCALL_INIT;
#ifdef OPENGL_DESKTOP
     if (GLEW_ARB_framebuffer_object)
     {
#endif
	if (GLCALL glIsRenderbuffer(_depthRenderBuffer))
	{
		GLCALL glDeleteRenderbuffers(1, &_depthRenderBuffer);
	}
#ifdef OPENGL_DESKTOP
     }
	else {
		if (!GLCALL glIsRenderbufferEXT(_depthRenderBuffer))
		{
			GLCALL glDeleteRenderbuffersEXT(1, &_depthRenderBuffer);
		}
	}
#endif
	_depthRenderBuffer=0;
}

void ogl2ShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
	GLCALL_INIT;
    GLint oldFBO=bindBuffer(glid);
    GLCALL glPixelStorei(GL_PACK_ALIGNMENT, 1);
    GLuint glformat=GL_RGBA;
    switch (format)
    {
    	case ShaderTexture::FMT_ALPHA: glformat=GL_ALPHA; break;
    	case ShaderTexture::FMT_RGB: glformat=GL_RGB; break;
    	case ShaderTexture::FMT_RGBA: glformat=GL_RGBA; break;
    	case ShaderTexture::FMT_Y: glformat=GL_LUMINANCE; break;
    	case ShaderTexture::FMT_YA: glformat=GL_LUMINANCE_ALPHA; break;
    	case ShaderTexture::FMT_DEPTH: glformat=GL_DEPTH_COMPONENT; break;
    }
    GLuint gltype=GL_UNSIGNED_BYTE;
    switch (packing)
    {
    	case ShaderTexture::PK_UBYTE: gltype=GL_UNSIGNED_BYTE; break;
    	case ShaderTexture::PK_USHORT_565: gltype=GL_UNSIGNED_SHORT_5_6_5; break;
    	case ShaderTexture::PK_USHORT_4444: gltype=GL_UNSIGNED_SHORT_4_4_4_4; break;
    	case ShaderTexture::PK_USHORT_5551: gltype=GL_UNSIGNED_SHORT_5_5_5_1; break;
    	case ShaderTexture::PK_USHORT: gltype=GL_UNSIGNED_SHORT; break;
    	case ShaderTexture::PK_UINT: gltype=GL_UNSIGNED_INT; break;
    	case ShaderTexture::PK_FLOAT: gltype=GL_FLOAT; break;
    }
    GLCALL glReadPixels(x,y,width,height,glformat,gltype,data);
    GLCALL glPixelStorei(GL_PACK_ALIGNMENT, 4);
    bindBuffer(oldFBO);
}

