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
    mrpd.depthAttachment.clearDepth=1.0;
    clearReq=0;
    depth=nil;
    //Alway create depth stencil, to avoid resetting the encoder
    MTLTextureDescriptor *td=[MTLTextureDescriptor new];
    td.pixelFormat=MTLPixelFormatDepth32Float_Stencil8;
    td.storageMode=MTLStorageModePrivate;
    td.width=tex->width;
    td.height=tex->height;
    td.usage=MTLTextureUsageRenderTarget;
    stencil=[metalDevice newTextureWithDescriptor:td];
    if (!forDepth_) {
        //td.pixelFormat=MTLPixelFormatDepth32Float;
        //depth=[metalDevice newTextureWithDescriptor:td];
        mrpd.depthAttachment.texture=stencil;
    }
    else {
        /*
        //Create the color attachment (shouldn't be required)
        td.pixelFormat=MTLPixelFormatRGBA8Unorm;
        depth=[metalDevice newTextureWithDescriptor:td];
        mrpd.colorAttachments[0].texture=depth;*/
    }
    mrpd.stencilAttachment.texture=stencil;
    mrpd.depthAttachment.storeAction=MTLStoreActionStore;
    [td release];
}

metalShaderBuffer::~metalShaderBuffer()
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
	[mrpd release];
}

void metalShaderBuffer::prepareDraw()
{
}

void metalShaderBuffer::needDepthStencil()
{
}

void metalShaderBuffer::unbound()
{
}

void metalShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
    tex->readPixels(x,y,width,height,format,packing,data);
}

