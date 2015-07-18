/*
 * gl2ShaderBuffer.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "dx11Shaders.h"
#include "string.h"

dx11ShaderBuffer::dx11ShaderBuffer(ShaderTexture *texture)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ID3D11Resource *res;
	((dx11ShaderTexture *)texture)->rsv->GetDesc(&desc);
	((dx11ShaderTexture *)texture)->rsv->GetResource(&res);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));

	renderTargetViewDesc.Format = desc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	g_dev->CreateRenderTargetView(res, &renderTargetViewDesc, &renderTarget);
}

dx11ShaderBuffer::~dx11ShaderBuffer()
{
	renderTarget->Release();
}

void dx11ShaderBuffer::prepareDraw()
{
}

void dx11ShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
	//TODO
}

