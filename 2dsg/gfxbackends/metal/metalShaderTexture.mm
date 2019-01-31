/*
 * gl2ShaderTexture.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "metalShaders.h"

metalShaderTexture::metalShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	this->width=width;
	this->height=height;
	this->wrap=wrap;
	this->filter=filtering;

    MTLPixelFormat glformat=MTLPixelFormatRGBA8Unorm;
    int bpr=4;
    switch (packing)
    {
    	case PK_UBYTE: {
    	    switch (format)
    	    {
    	    	case FMT_ALPHA: glformat=MTLPixelFormatA8Unorm; bpr=1; break;
    	    	case FMT_RGB: glformat=MTLPixelFormatRGBA8Unorm; break;
    	    	case FMT_RGBA: glformat=MTLPixelFormatRGBA8Unorm; break;
    	    	case FMT_Y: glformat=MTLPixelFormatR8Unorm; bpr=1; break;
    	    	case FMT_YA: glformat=MTLPixelFormatRG8Unorm; bpr=2; break;
    	    }
    	}
    	case PK_USHORT_565: {
    	    switch (format)
    	    {
    	    	case FMT_RGB: glformat=MTLPixelFormatB5G6R5Unorm; bpr=2; break;
    	    }
    		break;
    	}
    	case PK_USHORT_4444:  
    	    switch (format)
    	    {
    	    	case FMT_RGBA: glformat=MTLPixelFormatABGR4Unorm; bpr=2; break;
    	    }
    		break;
    	case PK_USHORT_5551: 
    	    switch (format)
    	    {
    	    	case FMT_RGBA: glformat=MTLPixelFormatA1BGR5Unorm; bpr=2; break;
    	    }
    		break;
    }
    
	MTLTextureDescriptor * md=[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:(MTLPixelFormat)glformat 
	                                                       width:(NSUInteger)width 
	                                                      height:(NSUInteger)height 
	                                                   mipmapped:(BOOL)NO];

	mtex=[MTLTexture newTextureWithDescriptor:md];                                                   
    if (data) {
    	[mtex replaceRegion:MTLRegionMake2D(0,0,width,height) 
         mipmapLevel:0 
           withBytes:data 
         bytesPerRow:bpr*width];
    }
    [md release];
}

void metalShaderTexture::updateData(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	this->width=width;
	this->height=height;
	this->wrap=wrap;
	this->filter=filtering;

    glformat=MTLPixelFormatRGBA8Unorm;
    bpr=4;
    switch (packing)
    {
    	case PK_UBYTE: {
    	    switch (format)
    	    {
    	    	case FMT_ALPHA: glformat=MTLPixelFormatA8Unorm; bpr=1; break;
    	    	case FMT_RGB: glformat=MTLPixelFormatRGBA8Unorm; break;
    	    	case FMT_RGBA: glformat=MTLPixelFormatRGBA8Unorm; break;
    	    	case FMT_Y: glformat=MTLPixelFormatR8Unorm; bpr=1; break;
    	    	case FMT_YA: glformat=MTLPixelFormatRG8Unorm; bpr=2; break;
    	    }
    	}
    	case PK_USHORT_565: {
    	    switch (format)
    	    {
    	    	case FMT_RGB: glformat=MTLPixelFormatB5G6R5Unorm; bpr=2; break;
    	    }
    		break;
    	}
    	case PK_USHORT_4444:  
    	    switch (format)
    	    {
    	    	case FMT_RGBA: glformat=MTLPixelFormatABGR4Unorm; bpr=2; break;
    	    }
    		break;
    	case PK_USHORT_5551: 
    	    switch (format)
    	    {
    	    	case FMT_RGBA: glformat=MTLPixelFormatA1BGR5Unorm; bpr=2; break;
    	    }
    		break;
    }
    if (data) {
    	[mtex replaceRegion:MTLRegionMake2D(0,0,width,height) 
         mipmapLevel:0 
           withBytes:data 
         bytesPerRow:bpr*width];
    }
}

void metalShaderTexture::setNative(void *externalTexture)
{
}

void *metalShaderTexture::getNative()
{
}

metalShaderTexture::~metalShaderTexture()
{
	[mtex release];
}



