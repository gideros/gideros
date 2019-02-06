/*
 * gl2ShaderBuffer.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "metalShaders.h"
#include "string.h"

metalShaderBuffer::metalShaderBuffer(ShaderTexture *texture)
{
    tex=(metalShaderTexture *)texture;
	
	mrpd=[MTLRenderPassDescriptor renderPassDescriptor];
    [mrpd retain];
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
    if (depth==nil) {
        MTLTextureDescriptor *td=[MTLTextureDescriptor new];
        td.pixelFormat=MTLPixelFormatDepth32Float;
        td.width=width;
        td.height=height;
        td.usage=MTLTextureUsageRenderTarget;
        depth=[metalDevice newTextureWithDescriptor:td];
        [depth retain];
        td.pixelFormat=MTLPixelFormatStencil8;
        stencil=[metalDevice newTextureWithDescriptor:td];
        [stencil retain];
        mrpd.depthAttachment.texture=depth;
        mrpd.stencilAttachment.texture=stencil;
    }
}

void metalShaderBuffer::unbound()
{
    //Delete depth buffer
    if (depth) {
        mrpd.depthAttachment.texture=nil;
        mrpd.stencilAttachment.texture=nil;
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

