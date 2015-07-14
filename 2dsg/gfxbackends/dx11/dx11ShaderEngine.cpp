/*
 * gl2ShaderEngine.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "dx11Shaders.h"
#include "gtexture.h"
#include "glog.h"
#include "ogl.h"
#include "dx11ParticleShader.h"

#include "dx11_shaders.c"

#ifdef WINSTORE
ComPtr<ID3D11Device1> g_dev;                     // the pointer to our Direct3D device interface (11.1)
ComPtr<ID3D11DeviceContext1> g_devcon;           // the pointer to our Direct3D device context (11.1)
#else
ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
#endif

ShaderProgram *dx11ShaderEngine::createShaderProgram(const char *vshader,const char *pshader,int flags, const ShaderProgram::ConstantDesc *uniforms, const ShaderProgram::DataDesc *attributes)
{
	return new dx11ShaderProgram(vshader,pshader,uniforms,attributes);
}

const char *dx11ShaderEngine::getVersion()
{
	return "DX11";
}

void dx11ShaderEngine::reset()
{
	ShaderEngine::reset();
	g_devcon->OMSetDepthStencilState(g_pDSOff, 1);
	g_devcon->RSSetState(g_pRSNormal);
	g_devcon->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFF);
	curDstFactor = ONE_MINUS_SRC_ALPHA;
	curSrcFactor = ONE;
	s_depthEnable=0;
	s_depthBufferCleared=false;
}

void dx11SetupShaders()
{
	const ShaderProgram::ConstantDesc stdConstants[]={
			{"vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0},
			{"fColor",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_Color,false,0},
			{"fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0},
			NULL
	};
	const ShaderProgram::DataDesc stdBAttributes[]={
			{"vVertex",dx11ShaderProgram::DFLOAT,3,0,0},
			{"vColor",dx11ShaderProgram::DUBYTE,0,1,0},
			{"vTexCoord",dx11ShaderProgram::DFLOAT,0,2,0},
			NULL
	};
	const ShaderProgram::DataDesc stdCAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 4, 1, 0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 0, 2, 0 },
		NULL
	};
	const ShaderProgram::DataDesc stdTAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 0, 1, 0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 2, 2, 0 },
		NULL
	};
	const dx11ShaderProgram::DataDesc stdTCAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 4, 1, 0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 2, 2, 0 },
		NULL
	};

    ShaderProgram::stdBasic = new dx11ShaderProgram(vBasic_cso,sizeof(vBasic_cso),pBasic_cso,sizeof(pBasic_cso),stdConstants,stdBAttributes);
    ShaderProgram::stdColor = new dx11ShaderProgram(vColor_cso,sizeof(vColor_cso),pColor_cso,sizeof(pColor_cso),stdConstants,stdCAttributes);
    ShaderProgram::stdTexture = new dx11ShaderProgram(vTexture_cso,sizeof(vTexture_cso),pTexture_cso,sizeof(pTexture_cso),stdConstants,stdTAttributes);
    ShaderProgram::stdTextureColor = new dx11ShaderProgram(vTextureColor_cso,sizeof(vTextureColor_cso),pTextureColor_cso,sizeof(pTextureColor_cso),stdConstants,stdTCAttributes);

	const ShaderProgram::ConstantDesc stdPConstants[]={
			{"vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0},
			{ "vPSize", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_ParticleSize, true, 0 },
			{ "fTexture", ShaderProgram::CTEXTURE, 1, ShaderProgram::SysConst_None, false, 0 },
			{"fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0},
			NULL
	};
    ShaderProgram::stdParticle = new dx11ParticleShader(vParticle_cso,sizeof(vParticle_cso),pParticle_cso,sizeof(pParticle_cso),stdPConstants,stdTCAttributes);

}

ID3D11Texture2D* dx11ShaderEngine::pBackBuffer=NULL;

dx11ShaderEngine::dx11ShaderEngine(int sw,int sh)
{
	currentBuffer=NULL;
	dx11ShaderProgram::current=NULL;

#ifndef GIDEROS_GL1
 dx11SetupShaders();
#endif

 D3D11_TEXTURE2D_DESC backbuff_desc;
 pBackBuffer->GetDesc(&backbuff_desc);

 g_dev->CreateRenderTargetView(pBackBuffer, NULL, &g_backbuffer);
 pBackBuffer->Release();

	//Depth / Stencil setup
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = backbuff_desc.Width;
	descDepth.Height = backbuff_desc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format =  DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	g_dev->CreateTexture2D(&descDepth, NULL, &g_depthStencilTexture);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Depth test parameters
	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	g_dev->CreateDepthStencilState(&dsDesc, &g_pDSOff);
	dsDesc.DepthEnable = true;
	g_dev->CreateDepthStencilState(&dsDesc, &g_pDSDepth);


	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	g_dev->CreateDepthStencilView(g_depthStencilTexture, // Depth stencil texture
		&descDSV, // Depth stencil desc
		&g_depthStencil);  // [out] Depth stencil view

	//
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerRepeat);
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerClamp);


	// Set rasterizer state to switch off backface culling

	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSNormal);
	rasterDesc.ScissorEnable = false;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSScissor);

	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;  // previously D3D11_BLEND_SRC_ALPHA
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	g_dev->CreateBlendState(&blendStateDesc, &g_pBlendState);

	g_devcon->OMSetRenderTargets(1, &g_backbuffer, g_depthStencil);  // could call this "rendertarget"
    g_pCBlendState=NULL;
	reset();
}

dx11ShaderEngine::~dx11ShaderEngine()
{
	if (currentBuffer)
		setFramebuffer(NULL);

    delete ShaderProgram::stdBasic;
    delete ShaderProgram::stdColor;
    delete ShaderProgram::stdTexture;
    delete ShaderProgram::stdTextureColor;

    g_depthStencil->Release();
    g_pBlendState->Release();
    g_pRSNormal->Release();
    g_pRSScissor->Release();
    g_pDSDepth->Release();
    g_pDSOff->Release();
    dx11ShaderTexture::samplerClamp->Release();
    dx11ShaderTexture::samplerRepeat->Release();
    if (g_pCBlendState)
        g_pCBlendState->Release();
#ifdef OPENGL_ES
	glDeleteRenderbuffers(1,&_depthRenderBuffer);
#endif
}

ShaderTexture *dx11ShaderEngine::createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)
{
	return new dx11ShaderTexture(format,packing,width,height,data,wrap,filtering);
}

ShaderBuffer *dx11ShaderEngine::createRenderTarget(ShaderTexture *texture)
{
	return new dx11ShaderBuffer(texture);
}

ShaderBuffer *dx11ShaderEngine::setFramebuffer(ShaderBuffer *fbo)
{
	ShaderBuffer *previous=currentBuffer;
	if (fbo)
	{
		ID3D11ShaderResourceView *nrs = NULL;
		g_devcon->PSSetShaderResources(0, 1, &nrs);
	}
	g_devcon->OMSetRenderTargets(1,
		fbo?&(((dx11ShaderBuffer *)fbo)->renderTarget):&g_backbuffer, 
		fbo?NULL:g_depthStencil);
    currentBuffer=fbo;
    return previous;
}

void dx11ShaderEngine::setViewport(int x,int y,int width,int height)
{
	  D3D11_VIEWPORT viewport;
	  ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

	  viewport.TopLeftX=x;
	  viewport.TopLeftY=y;
	  viewport.Width=width;
	  viewport.Height=height;
	  viewport.MinDepth = 0;
	  viewport.MaxDepth = 1.0;
	  g_devcon->RSSetViewports(1, &viewport);
}

void dx11ShaderEngine::clearColor(float r,float g,float b,float a)
{
	float col[4]={r,g,b,a};
	ID3D11RenderTargetView *fbo = currentBuffer ? (((dx11ShaderBuffer *)currentBuffer)->renderTarget) : g_backbuffer;
	g_devcon->ClearRenderTargetView(fbo, col);
}

void dx11ShaderEngine::bindTexture(int num,ShaderTexture *texture)
{
	dx11ShaderTexture *tex=(dx11ShaderTexture *)texture;
	g_devcon->PSSetShaderResources(num,1,&tex->rsv);
	if (tex->wrap==ShaderTexture::WRAP_CLAMP)
		g_devcon->PSSetSamplers(0, 1, &dx11ShaderTexture::samplerRepeat/*Clamp*/);
	else
		g_devcon->PSSetSamplers(0, 1, &dx11ShaderTexture::samplerRepeat);
}


void dx11ShaderEngine::setClip(int x,int y,int w,int h)
{
	if ((w<0)||(h<0))
		g_devcon->RSSetState(g_pRSNormal);
	else
	{
		D3D11_RECT pRect;
		pRect.left = x;
		pRect.top = y;
		pRect.right = x + w;
		pRect.bottom = y + h;
		g_devcon->RSSetScissorRects(1, &pRect);
		g_devcon->RSSetState(g_pRSScissor);
	}
}

void dx11ShaderEngine::setDepthTest(bool enable)
{
	if (enable)
	{
		if (!(s_depthEnable++))
		{
			if (!s_depthBufferCleared)
			{
    			g_devcon->ClearDepthStencilView(g_depthStencil, D3D11_CLEAR_DEPTH, 1.0, 0);
    			//g_devcon->ClearDepthStencilView(g_depthStencil, D3D11_CLEAR_STENCIL, 1.0, 0);
    			s_depthBufferCleared=true;
			}
			g_devcon->OMSetDepthStencilState(g_pDSDepth, 1);
		}
	}
	else
	{
		if (!(--s_depthEnable))
			g_devcon->OMSetDepthStencilState(g_pDSOff, 1);
	}
}


D3D11_BLEND dx11ShaderEngine::blendFactor2D3D11(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case ZERO:
		   return D3D11_BLEND_ZERO;
		case ONE:
			return D3D11_BLEND_ONE;
		case SRC_COLOR:
			return D3D11_BLEND_SRC_COLOR;
		case ONE_MINUS_SRC_COLOR:
			return D3D11_BLEND_INV_SRC_COLOR;
		case DST_COLOR:
			return D3D11_BLEND_DEST_COLOR;
		case ONE_MINUS_DST_COLOR:
			return D3D11_BLEND_INV_DEST_COLOR;
		case SRC_ALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		case ONE_MINUS_SRC_ALPHA:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case DST_ALPHA:
			return D3D11_BLEND_DEST_ALPHA;
		case ONE_MINUS_DST_ALPHA:
			return D3D11_BLEND_INV_DEST_ALPHA;
		//case CONSTANT_COLOR:
		//   return GL_CONSTANT_COLOR;
		//case ONE_MINUS_CONSTANT_COLOR:
		//   return GL_ONE_MINUS_CONSTANT_COLOR;
		//case CONSTANT_ALPHA:
		//   return GL_CONSTANT_ALPHA;
		//case ONE_MINUS_CONSTANT_ALPHA:
		//   return GL_ONE_MINUS_CONSTANT_ALPHA;
		case SRC_ALPHA_SATURATE:
			return D3D11_BLEND_SRC_ALPHA_SAT;
	}

	return D3D11_BLEND_ZERO;
}

void dx11ShaderEngine::setBlendFunc(BlendFactor sfactor, BlendFactor dfactor)
{
	if ((sfactor == curSrcFactor) && (dfactor == curDstFactor)) return;
	curSrcFactor = sfactor;
	curDstFactor = dfactor;
	if ((sfactor == BlendFactor::ONE) && (dfactor == BlendFactor::ONE_MINUS_SRC_ALPHA))
	{
		g_devcon->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFF);
		return;
	}
	if ((g_pCBlendState != NULL) && (sfactor == curCSrcFactor) && (dfactor == curCDstFactor))
	{
		g_devcon->OMSetBlendState(g_pCBlendState, NULL, 0xFFFFFF);
		return;
	}
	curCSrcFactor = sfactor;
	curCDstFactor = dfactor;
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = blendFactor2D3D11(sfactor);
	blendStateDesc.RenderTarget[0].DestBlend = blendFactor2D3D11(dfactor);
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (g_pCBlendState != NULL)
		g_pCBlendState->Release();
	g_dev->CreateBlendState(&blendStateDesc, &g_pCBlendState);
	g_devcon->OMSetBlendState(g_pCBlendState, NULL, 0xFFFFFF);
}

Matrix4 dx11ShaderEngine::setFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
	int df = 1, dn = 0;
	mat[0] = 2 * n / (r - l);
	mat[5] = 2 * n / (t - b);
	mat[8] = (r + l) / (r - l);
	mat[9] = (t + b) / (t - b);
	mat[10] = -(df*f - dn*n) / (f - n);
	mat[11] = -1;
	mat[14] = -((df - dn) * f * n) / (f - n);
	mat[15] = 0;
	mat.type = Matrix4::FULL;
	return mat;
}

