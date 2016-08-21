/*
 * gl2ShaderTexture.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gl2Shaders.h"

ogl2ShaderTexture::ogl2ShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	glid=0;
	native=false;
	this->width=width;
	this->height=height;

    GLint oldTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

    glGenTextures(1, &glid);
    glBindTexture(GL_TEXTURE_2D, glid);
    switch (wrap)
    {
    case WRAP_CLAMP:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case WRAP_REPEAT:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }
    switch (filtering)
    {
    case FILT_NEAREST:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    case FILT_LINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    }

    GLuint glformat=GL_RGBA;
    switch (format)
    {
    	case FMT_ALPHA: glformat=GL_ALPHA; break;
    	case FMT_RGB: glformat=GL_RGB; break;
    	case FMT_RGBA: glformat=GL_RGBA; break;
    	case FMT_Y: glformat=GL_LUMINANCE; break;
    	case FMT_YA: glformat=GL_LUMINANCE_ALPHA; break;
    }
    GLuint gltype=GL_UNSIGNED_BYTE;
    switch (packing)
    {
    	case PK_UBYTE: gltype=GL_UNSIGNED_BYTE; break;
    	case PK_USHORT_565: gltype=GL_UNSIGNED_SHORT_5_6_5; break;
    	case PK_USHORT_4444: gltype=GL_UNSIGNED_SHORT_4_4_4_4; break;
    	case PK_USHORT_5551: gltype=GL_UNSIGNED_SHORT_5_5_5_1; break;
    }
    if (data)
    	glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, gltype, data);

    glBindTexture(GL_TEXTURE_2D, oldTex);
}

void ogl2ShaderTexture::setNative(void *externalTexture)
{
	if (!native)
		glDeleteTextures(1,&glid);
	glid=externalTexture?(*((GLuint *)externalTexture)):0;
	native=true;
}

void *ogl2ShaderTexture::getNative()
{
	return &glid;
}

ogl2ShaderTexture::~ogl2ShaderTexture()
{
	if (!native)
		glDeleteTextures(1,&glid);
}



