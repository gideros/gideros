/*
 * gl2ShaderTexture.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "dx11Shaders.h"
#include "glog.h"

ID3D11SamplerState *dx11ShaderTexture::samplerRepeat=NULL;
ID3D11SamplerState *dx11ShaderTexture::samplerClamp=NULL;
ID3D11SamplerState *dx11ShaderTexture::samplerRepeatFilter=NULL;
ID3D11SamplerState *dx11ShaderTexture::samplerClampFilter = NULL;
ID3D11SamplerState *dx11ShaderTexture::samplerDepthCompare = NULL;

dx11ShaderTexture::dx11ShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	this->width=width;
	this->height=height;
	this->wrap=wrap;
	this->filter=filtering;
	this->format = format;

    D3D11_TEXTURE2D_DESC tdesc;
    D3D11_SUBRESOURCE_DATA tbsd;

    tdesc.Width = width;
    tdesc.Height = height;
    tdesc.MipLevels = (filtering==FILT_LINEAR_MIPMAP)?1:1;
    tdesc.ArraySize = 1;
    tdesc.SampleDesc.Count = 1;
    tdesc.SampleDesc.Quality = 0;
    tdesc.Usage = D3D11_USAGE_DEFAULT;
    tdesc.BindFlags = ((format==FMT_DEPTH)? D3D11_BIND_DEPTH_STENCIL:D3D11_BIND_RENDER_TARGET) | D3D11_BIND_SHADER_RESOURCE;
    tdesc.CPUAccessFlags = 0;
	tdesc.MiscFlags =  ((filtering == FILT_LINEAR_MIPMAP) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
    tdesc.Format = DXGI_FORMAT_UNKNOWN;
    switch (format)
    {
		case FMT_ALPHA: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_R8_UNORM; break;
		case FMT_Y: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_R8_UNORM; break;
		case FMT_YA: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_R8G8_UNORM; break;
    	case FMT_RGB:
    		switch (packing)
    		{
    			//case PK_UBYTE: tbsd.SysMemPitch = width*3; tdesc.Format = DXGI_FORMAT_R8G8B8_UNORM; break;
    			case PK_USHORT_565: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B5G6R5_UNORM; break;
    		}
    		break;
		case FMT_RGBA:
			switch (packing)
			{
				case PK_UBYTE: tbsd.SysMemPitch = width*4; tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				case PK_USHORT_4444: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM; break;
				case PK_USHORT_5551: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B5G5R5A1_UNORM; break;
			}
			break;
		case FMT_DEPTH:
			switch (packing)
			{
			case PK_FLOAT: tbsd.SysMemPitch = width*4; tdesc.Format = DXGI_FORMAT_R24G8_TYPELESS; break;
			//case PK_USHORT: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_R16_UNORM; break;
			}
			break;
    }

    tbsd.SysMemSlicePitch = tbsd.SysMemPitch*height; // not needed
  	tbsd.pSysMem = data;
	if (tdesc.Format == DXGI_FORMAT_UNKNOWN){
  	  glog_w("glTexImage2D: unknown internal format");
  	  exit(1);
    }

    g_dev->CreateTexture2D(&tdesc,&tbsd,&tex);
	D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
	sr_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sr_desc.Texture2D.MostDetailedMip = 0;
	sr_desc.Texture2D.MipLevels = -1;
    g_dev->CreateShaderResourceView(tex,(format==FMT_DEPTH)?&sr_desc:NULL,&rsv);

    if (filtering==FILT_LINEAR_MIPMAP)
    	g_devcon->GenerateMips(rsv);

//    g_devcon->PSSetShaderResources(0,1,&g_RSV[g_curr_texind]);
}

void dx11ShaderTexture::updateData(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	rsv->Release();
    tex->Release();
	this->width=width;
	this->height=height;
	this->wrap=wrap;
	this->filter=filtering;

    D3D11_TEXTURE2D_DESC tdesc;
    D3D11_SUBRESOURCE_DATA tbsd;

    tdesc.Width = width;
    tdesc.Height = height;
    tdesc.MipLevels = (filtering==FILT_LINEAR_MIPMAP)?0:1;
    tdesc.ArraySize = 1;
    tdesc.SampleDesc.Count = 1;
    tdesc.SampleDesc.Quality = 0;
    tdesc.Usage = D3D11_USAGE_DEFAULT;
    tdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    tdesc.CPUAccessFlags = 0;
    tdesc.MiscFlags = 0;
    tdesc.Format = DXGI_FORMAT_UNKNOWN;
    switch (format)
    {
		case FMT_ALPHA: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_R8_UNORM; break;
		case FMT_Y: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_R8_UNORM; break;
		case FMT_YA: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_R8G8_UNORM; break;
    	case FMT_RGB:
    		switch (packing)
    		{
    			//case PK_UBYTE: tbsd.SysMemPitch = width*3; tdesc.Format = DXGI_FORMAT_R8G8B8_UNORM; break;
    			case PK_USHORT_565: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B5G6R5_UNORM; break;
    		}
    		break;
    	case FMT_RGBA:
    		switch (packing)
    		{
    			case PK_UBYTE: tbsd.SysMemPitch = width*4; tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
       			case PK_USHORT_4444: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM; break;
       			case PK_USHORT_5551: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_B5G5R5A1_UNORM; break;
       		}
		break;
    }

    tbsd.SysMemSlicePitch = tbsd.SysMemPitch*height; // not needed
  	tbsd.pSysMem = data;
	if (tdesc.Format == DXGI_FORMAT_UNKNOWN){
  	  glog_w("dx11Texture: unknown internal format");
  	  exit(1);
    }

    g_dev->CreateTexture2D(&tdesc,&tbsd,&tex);
    g_dev->CreateShaderResourceView(tex,NULL,&rsv);
    if (filtering==FILT_LINEAR_MIPMAP)
    	g_devcon->GenerateMips(rsv);
}

dx11ShaderTexture::~dx11ShaderTexture()
{
	rsv->Release();
    tex->Release();
}



