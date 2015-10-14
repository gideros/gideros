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
	width = ((dx11ShaderTexture *)texture)->width;
	height = ((dx11ShaderTexture *)texture)->height;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));

	renderTargetViewDesc.Format = desc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	g_dev->CreateRenderTargetView(res, &renderTargetViewDesc, &renderTarget);
	depthStencil = NULL;
}

dx11ShaderBuffer::~dx11ShaderBuffer()
{
	renderTarget->Release();
}

void dx11ShaderBuffer::prepareDraw()
{
}

void dx11ShaderBuffer::unbound()
{
	if (depthStencil)
	{
		depthStencil->Release();
		depthStencilTexture->Release();
	}
	depthStencil = NULL;
}

void dx11ShaderBuffer::needDepthStencil()
{
	if (!depthStencil)
	{

	//Depth / Stencil setup
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	g_dev->CreateTexture2D(&descDepth, NULL, &depthStencilTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	g_dev->CreateDepthStencilView(depthStencilTexture, // Depth stencil texture
		&descDSV, // Depth stencil desc
		&depthStencil);  // [out] Depth stencil view
	//Attach the newly created depth/stencil buf
	g_devcon->OMSetRenderTargets(1,&renderTarget,depthStencil);
	}
}

void dx11ShaderBuffer::readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)
{
	//TODO
}

