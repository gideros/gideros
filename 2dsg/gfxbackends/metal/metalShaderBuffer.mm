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
	width=((metalShaderTexture *)texture)->width;
	height=((metalShaderTexture *)texture)->height;
	
	mrpd=[MTLRenderPassDescriptor renderPassDescriptor];
	[mrpd.colorAttachments objectAtIndexedSubscript:0].texture=texture->mtex;
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


}

void metalShaderBuffer::unbound()
{
 //Delete depth buffer
}

void metalShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
 //TODO
}

