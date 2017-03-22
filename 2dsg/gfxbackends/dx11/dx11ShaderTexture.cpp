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
ID3D11SamplerState *dx11ShaderTexture::samplerClampFilter=NULL;

dx11ShaderTexture::dx11ShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	this->width=width;
	this->height=height;
	this->wrap=wrap;
	this->filter=filtering;

    D3D11_TEXTURE2D_DESC tdesc;
    D3D11_SUBRESOURCE_DATA tbsd;

    tdesc.Width = width;
    tdesc.Height = height;
    tdesc.MipLevels = 1;
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
		case FMT_ALPHA: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_A8_UNORM; break;
		//case FMT_Y: tbsd.SysMemPitch = width; tdesc.Format = DXGI_FORMAT_L8_UNORM; break;
		//case FMT_YA: tbsd.SysMemPitch = width*2; tdesc.Format = DXGI_FORMAT_A8L8_UNORM; break; //XXX bytes need to be swapped, but this mode is not used by gideros anyhow
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
  	  glog_w("glTexImage2D: unknown internal format");
  	  exit(1);
    }

    g_dev->CreateTexture2D(&tdesc,&tbsd,&tex);
    g_dev->CreateShaderResourceView(tex,NULL,&rsv);

//    g_devcon->PSSetShaderResources(0,1,&g_RSV[g_curr_texind]);
}

dx11ShaderTexture::~dx11ShaderTexture()
{
	rsv->Release();
    tex->Release();
}



