/*
 * gl2ShaderBuffer.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "metalShaders.h"
#include "string.h"

metalShaderBuffer::metalShaderBuffer(ShaderTexture *texture,bool forDepth)
{
    tex=(metalShaderTexture *)texture;
    forDepth_=forDepth;
	
	mrpd=[MTLRenderPassDescriptor renderPassDescriptor];
    [mrpd retain];
    if (forDepth)
        mrpd.depthAttachment.texture=tex->mtex;
    else
	    mrpd.colorAttachments[0].texture=tex->mtex;
    clearReq=0;
    depth=nil;
    stencil=nil;
}

metalShaderBuffer::~metalShaderBuffer()
{
	[mrpd release];
}

void metalShaderBuffer::prepareDraw()
{
}

void metalShaderBuffer::needDepthStencil()
{
    if (stencil==nil) {
        MTLTextureDescriptor *td=[MTLTextureDescriptor new];
        td.pixelFormat=MTLPixelFormatStencil8;
        td.width=width;
        td.height=height;
        td.usage=MTLTextureUsageRenderTarget;
        stencil=[metalDevice newTextureWithDescriptor:td];
        [stencil retain];
    	if (!forDepth_) {
	        td.pixelFormat=MTLPixelFormatDepth32Float;
	        depth=[metalDevice newTextureWithDescriptor:td];
	        [depth retain];
	        mrpd.depthAttachment.texture=depth;
	    }
        mrpd.stencilAttachment.texture=stencil;
    }
}

void metalShaderBuffer::unbound()
{
    //Delete depth buffer
    if (stencil) {
    	if (!forDepth_)
	        mrpd.depthAttachment.texture=nil;
        mrpd.stencilAttachment.texture=nil;
        if (depth)
	        [depth release];
        [stencil release];
        depth=nil;
        stencil=nil;
    }
}

void metalShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
    tex->readPixels(x,y,width,height,format,packing,data);
}

