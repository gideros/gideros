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
    if (forDepth) {
        mrpd.depthAttachment.texture=tex->mtex;
        mrpd.stencilAttachment.texture=tex->mtex;
    }
    else {
	    mrpd.colorAttachments[0].texture=tex->mtex;
        //Alway create depth stencil, to avoid resetting the encoder
        MTLTextureDescriptor *td=[MTLTextureDescriptor new];
        td.pixelFormat=MTLPixelFormatDepth32Float_Stencil8;
        td.storageMode=MTLStorageModePrivate;
        td.width=tex->width;
        td.height=tex->height;
        td.usage=MTLTextureUsageRenderTarget;
        stencil=[metalDevice newTextureWithDescriptor:td];
        mrpd.depthAttachment.texture=stencil;
        mrpd.stencilAttachment.texture=stencil;
        [td release];
    }
    mrpd.depthAttachment.clearDepth=1.0;
    mrpd.depthAttachment.storeAction=MTLStoreActionStore;
    clearReq=0;
    depth=nil; //Combined depth-stencil
}

metalShaderBuffer::~metalShaderBuffer()
{
    //Delete depth buffer
    if (stencil) {
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

