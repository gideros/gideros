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
ComPtr<IDXGISwapChain1> g_swapchain;             // the pointer to the swap chain interface (11.1)
#else
ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
IDXGISwapChain *g_swapchain;             // the pointer to the swap chain interface
#endif

ShaderProgram *dx11ShaderEngine::createShaderProgram(const char *vshader,const char *pshader,int flags, const ShaderProgram::ConstantDesc *uniforms, const ShaderProgram::DataDesc *attributes)
{
	return new dx11ShaderProgram(vshader,pshader,flags,uniforms,attributes);
}

const char *dx11ShaderEngine::getVersion()
{
	return "DX11";
}

ShaderTexture::Packing dx11ShaderEngine::getPreferredPackingForTextureFormat(ShaderTexture::Format format)
{
	switch (format) {
	case ShaderTexture::FMT_DEPTH: return ShaderTexture::PK_FLOAT;
	default:
		return ShaderTexture::PK_UBYTE;
	}
}

void dx11ShaderEngine::reset(bool reinit)
{
	ShaderEngine::reset(reinit);
	g_devcon->OMSetDepthStencilState(g_pDSOff, 1);
	g_devcon->RSSetState(g_pRSNormalCN);
	g_devcon->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFF);
	curDstFactor = ONE_MINUS_SRC_ALPHA;
	curSrcFactor = ONE;
	s_depthEnable=0;
	s_depthBufferCleared=false;
	s_clipEnabled=false;
}

void dx11SetupShaders()
{
	const ShaderProgram::ConstantDesc stdConstants[] = {
			{"vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL},
			{"fColor",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_Color,false,0,NULL},
			{"fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL},
			{"",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL}
	};
	const ShaderProgram::ConstantDesc stdCConstants[] = {
			{"vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL},
			{"vfColor",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_Color,true,0,NULL},
			{"fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL},
			{"",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL}
	};
	const ShaderProgram::DataDesc stdBAttributes[]={
			{"vVertex",dx11ShaderProgram::DFLOAT,3,0,0,0},
			{"vColor",dx11ShaderProgram::DUBYTE,0,1,0,0},
			{"vTexCoord",dx11ShaderProgram::DFLOAT,0,2,0,0},
			{"",ShaderProgram::DFLOAT,0,0,0,0}
	};
	const ShaderProgram::DataDesc stdCAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0,0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 4, 1, 0,0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 0, 2, 0,0 },
		{"",ShaderProgram::DFLOAT,0,0,0,0}
	};
	const ShaderProgram::DataDesc stdTAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0,0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 0, 1, 0,0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 2, 2, 0,0 },
		{"",ShaderProgram::DFLOAT,0,0,0,0}
	};
	const dx11ShaderProgram::DataDesc stdTCAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 3, 0, 0,0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 4, 1, 0,0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 2, 2, 0,0 },
		{ "",ShaderProgram::DFLOAT,0,0,0,0 }
	};
	const ShaderProgram::ConstantDesc stdPSConstants[] = {
		{ "vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL },
		{ "vWorldMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldMatrix,true,0,NULL },
		{ "vfColor",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_Color,true,0,NULL},
		{ "fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL },
		{ "fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0,NULL },
		{ "",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL }
	};
    const ShaderProgram::ConstantDesc stdPS3Constants[] = {
        { "vWorldMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldMatrix,true,0,NULL },
        { "vViewMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_ViewMatrix,true,0,NULL },
        { "vProjMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_ProjectionMatrix,true,0,NULL },
        { "fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL },
        { "fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0,NULL },
        { "vfColor", ShaderProgram::CFLOAT4, 1,ShaderProgram::SysConst_Color, true, 0, NULL },
        { "",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL }
    };
	const dx11ShaderProgram::DataDesc stdPSAttributes[] = {
		{ "vVertex", dx11ShaderProgram::DFLOAT, 4, 0, 0,0 },
		{ "vColor", dx11ShaderProgram::DUBYTE, 4, 1, 0,0 },
		{ "vTexCoord", dx11ShaderProgram::DFLOAT, 4, 2, 0,0 },
		{ "",ShaderProgram::DFLOAT,0,0,0,0 }
	};

    ShaderProgram::stdBasic = new dx11ShaderProgram(vBasic_cso,sizeof(vBasic_cso),pBasic_cso,sizeof(pBasic_cso),0,stdConstants,stdBAttributes);
    ShaderProgram::stdColor = new dx11ShaderProgram(vColor_cso,sizeof(vColor_cso),pColor_cso,sizeof(pColor_cso),0,stdCConstants,stdCAttributes);
    ShaderProgram::stdTexture = new dx11ShaderProgram(vTexture_cso,sizeof(vTexture_cso),pTexture_cso,sizeof(pTexture_cso),0,stdConstants,stdTAttributes);
    ShaderProgram::stdTextureAlpha = new dx11ShaderProgram(vTextureAlpha_cso,sizeof(vTextureAlpha_cso),pTextureAlpha_cso,sizeof(pTextureAlpha_cso),0,stdConstants,stdTAttributes);
	ShaderProgram::stdTextureColor = new dx11ShaderProgram(vTextureColor_cso, sizeof(vTextureColor_cso), pTextureColor_cso, sizeof(pTextureColor_cso), 0, stdCConstants, stdTCAttributes);
	ShaderProgram::stdTextureAlphaColor = new dx11ShaderProgram(vTextureAlphaColor_cso, sizeof(vTextureAlphaColor_cso), pTextureAlphaColor_cso, sizeof(pTextureAlphaColor_cso), 0, stdCConstants, stdTCAttributes);
	ShaderProgram::stdParticles = new dx11ShaderProgram(vParticles_cso, sizeof(vParticles_cso), pParticles_cso, sizeof(pParticles_cso), 0, stdPSConstants, stdPSAttributes);
	ShaderProgram::stdParticles3 = new dx11ShaderProgram(vParticles3_cso, sizeof(vParticles3_cso), pParticles3_cso, sizeof(pParticles3_cso), 0, stdPS3Constants, stdPSAttributes);

	const ShaderProgram::ConstantDesc stdPConstants[]={
			{"vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL},
			{ "vfColor",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_Color,true,0,NULL},
			{ "vPSize", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_ParticleSize, true, 0,NULL },
			{ "fTexture", ShaderProgram::CTEXTURE, 1, ShaderProgram::SysConst_None, false, 0,NULL },
			{"fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0,NULL},
			{"",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL}
	};
    ShaderProgram::stdParticle = new dx11ParticleShader(vParticle_cso,sizeof(vParticle_cso),pParticle_cso,sizeof(pParticle_cso),0,stdPConstants,stdTCAttributes);

	const ShaderProgram::ConstantDesc pathUniformsFC[] = {
		{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
		{ "xform",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_None, true, 0, NULL },
		{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
		{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };
	const ShaderProgram::DataDesc pathAttributesFC[] = {
		{ "vVertex",ShaderProgram::DFLOAT, 4, 0, 0,0 },
		{ "", ShaderProgram::DFLOAT, 0, 0, 0,0 } };

	const ShaderProgram::ConstantDesc pathUniformsSC[] = {
		{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
		{ "xform",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_None, true, 0, NULL },
		{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
		{ "feather", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_None, false, 0, NULL },
		{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };
	const ShaderProgram::DataDesc pathAttributesSC[] = {
		{ "dataA",ShaderProgram::DFLOAT, 4, 0, 0,0 },
		{ "dataB", ShaderProgram::DFLOAT, 4, 1, 0,0 },
		{ "dataC", ShaderProgram::DFLOAT, 4, 2, 0,0 },
		{ "", ShaderProgram::DFLOAT, 0, 0, 0,0 } };

	const ShaderProgram::ConstantDesc pathUniformsSL[] = {
		{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
		{ "xform",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_None, true, 0, NULL },
		{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
		{ "feather", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_None, false, 0, NULL },
		{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };
	const ShaderProgram::DataDesc pathAttributesSL[] = {
		{ "vVertex",ShaderProgram::DFLOAT, 4, 0, 0,0 },
		{ "", ShaderProgram::DFLOAT, 0, 0, 0,0 } };

	ShaderProgram::pathShaderFillC = new dx11ShaderProgram(vPathFillC_cso, sizeof(vPathFillC_cso), pPathFillC_cso, sizeof(pPathFillC_cso), 0, pathUniformsFC, pathAttributesFC);
	ShaderProgram::pathShaderStrokeC = new dx11ShaderProgram(vPathStrokeC_cso, sizeof(vPathStrokeC_cso), pPathStrokeC_cso, sizeof(pPathStrokeC_cso), 0, pathUniformsSC, pathAttributesSC);
	ShaderProgram::pathShaderStrokeLC = new dx11ShaderProgram(vPathStrokeLC_cso, sizeof(vPathStrokeLC_cso), pPathStrokeLC_cso, sizeof(pPathStrokeLC_cso), 0, pathUniformsSL, pathAttributesSL);
}

dx11ShaderEngine::dx11ShaderEngine(int sw,int sh)
{
	currentBuffer=NULL;
	dx11ShaderProgram::current=NULL;

	dx11SetupShaders();

	ID3D11Texture2D* pBackBuffer;
	HRESULT hr = g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	
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
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
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
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerRepeat);
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerRepeatFilter);

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerClamp);
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerClampFilter);

	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	g_dev->CreateSamplerState(&sampDesc, &dx11ShaderTexture::samplerDepthCompare);

	// Set rasterizer state to switch off backface culling

	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	rasterDesc.CullMode = D3D11_CULL_NONE;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSNormalCN);
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSNormalCF);
	rasterDesc.CullMode = D3D11_CULL_BACK;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSNormalCB);

	rasterDesc.ScissorEnable = true;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSScissorCN);
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSScissorCF);
	rasterDesc.CullMode = D3D11_CULL_BACK;
	g_dev->CreateRasterizerState(&rasterDesc, &g_pRSScissorCB);

	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;  // previously D3D11_BLEND_SRC_ALPHA
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	g_dev->CreateBlendState(&blendStateDesc, &g_pBlendState);

	g_devcon->OMSetRenderTargets(1, &g_backbuffer, g_depthStencil);  // could call this "rendertarget"
    g_pCBlendState=NULL;
	g_pCDSState = NULL;
	reset();
}

void dx11ShaderEngine::resizeFramebuffer(int width,int height)
{
    if (g_swapchain)
    {
        g_devcon->OMSetRenderTargets(0, 0, 0);

        // Release all outstanding references to the swap chain's buffers.
        g_backbuffer->Release();

        HRESULT hr;
        // Preserve the existing buffer count and format.
        // Automatically choose the width and height to match the client rect for HWNDs.
        hr = g_swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        // Perform error handling here!

        // Get buffer and create a render-target-view.
        ID3D11Texture2D* pBackBuffer;
        hr = g_swapchain->GetBuffer(0, __uuidof( ID3D11Texture2D),  (void**) &pBackBuffer );
        // Perform error handling here!

		D3D11_TEXTURE2D_DESC backbuff_desc;
		pBackBuffer->GetDesc(&backbuff_desc);

        g_dev->CreateRenderTargetView(pBackBuffer, NULL, &g_backbuffer);
        pBackBuffer->Release();

        g_depthStencil->Release();
        g_depthStencilTexture->Release();

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

    	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    	ZeroMemory(&descDSV, sizeof(descDSV));
    	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    	descDSV.Texture2D.MipSlice = 0;

    	// Create the depth stencil view
    	g_dev->CreateDepthStencilView(g_depthStencilTexture, // Depth stencil texture
    		&descDSV, // Depth stencil desc
    		&g_depthStencil);  // [out] Depth stencil view

        g_devcon->OMSetRenderTargets(1, &g_backbuffer, g_depthStencil );

        // Set up the viewport.
        D3D11_VIEWPORT vp;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_devcon->RSSetViewports( 1, &vp );
    }
}

dx11ShaderEngine::~dx11ShaderEngine()
{
	if (currentBuffer)
		setFramebuffer(NULL);

    delete ShaderProgram::stdBasic;
    delete ShaderProgram::stdColor;
    delete ShaderProgram::stdTexture;
    delete ShaderProgram::stdTextureAlpha;
    delete ShaderProgram::stdTextureColor;
    delete ShaderProgram::stdParticle;
	delete ShaderProgram::pathShaderFillC;
	delete ShaderProgram::pathShaderStrokeC;

    g_depthStencil->Release();
    g_pBlendState->Release();
    g_pRSNormalCN->Release();
    g_pRSNormalCF->Release();
    g_pRSNormalCB->Release();
    g_pRSScissorCN->Release();
    g_pRSScissorCF->Release();
    g_pRSScissorCB->Release();
    g_pDSDepth->Release();
    g_pDSOff->Release();
	dx11ShaderTexture::samplerClamp->Release();
	dx11ShaderTexture::samplerRepeat->Release();
	dx11ShaderTexture::samplerClampFilter->Release();
	dx11ShaderTexture::samplerRepeatFilter->Release();
	dx11ShaderTexture::samplerDepthCompare->Release();
	if (g_pCBlendState)
        g_pCBlendState->Release();
	if (g_pCDSState)
		g_pCDSState->Release();
	g_backbuffer->Release();
}

ShaderTexture *dx11ShaderEngine::createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering,bool forRT)
{
	return new dx11ShaderTexture(format,packing,width,height,data,wrap,filtering);
}

ShaderBuffer *dx11ShaderEngine::createRenderTarget(ShaderTexture *texture,bool forDepth)
{
	return new dx11ShaderBuffer(texture,forDepth);
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
		fbo?(((dx11ShaderBuffer *)fbo)->depthStencil):g_depthStencil);
    currentBuffer=fbo;
	if (previous)
		previous->unbound();
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
	if (fbo)
		g_devcon->ClearRenderTargetView(fbo, col);
}

void dx11ShaderEngine::bindTexture(int num,ShaderTexture *texture)
{
	dx11ShaderTexture *tex=(dx11ShaderTexture *)texture;
	g_devcon->PSSetShaderResources(num,1,&tex->rsv);
	if ((tex->format==ShaderTexture::FMT_DEPTH)&&(tex->filter == ShaderTexture::FILT_LINEAR))
		g_devcon->PSSetSamplers(num, 1, &dx11ShaderTexture::samplerDepthCompare);
	else if (tex->wrap==ShaderTexture::WRAP_CLAMP)
		g_devcon->PSSetSamplers(num, 1, (tex->filter==ShaderTexture::FILT_NEAREST)?&dx11ShaderTexture::samplerClamp:&dx11ShaderTexture::samplerClampFilter);
	else
		g_devcon->PSSetSamplers(num, 1, (tex->filter==ShaderTexture::FILT_NEAREST)?&dx11ShaderTexture::samplerRepeat:&dx11ShaderTexture::samplerRepeatFilter);
}

void dx11ShaderEngine::updateRasterizer()
{
	ID3D11RasterizerState *rs=NULL;
	switch (dsCurrent.cullMode) { //TODO
	case CULL_FRONT: rs=s_clipEnabled?g_pRSScissorCF:g_pRSNormalCF; break;
	case CULL_BACK: rs=s_clipEnabled?g_pRSScissorCB:g_pRSNormalCB; break;
	default: rs=s_clipEnabled?g_pRSScissorCN:g_pRSNormalCN; break;
	}
	g_devcon->RSSetState(rs);
}

void dx11ShaderEngine::setClip(int x,int y,int w,int h)
{
	if ((w<0)||(h<0)) {
		s_clipEnabled=false;
		updateRasterizer();
	}
	else
	{
		D3D11_RECT pRect;
		pRect.left = x;
		pRect.top = y;
		pRect.right = x + w;
		pRect.bottom = y + h;
		g_devcon->RSSetScissorRects(1, &pRect);
		s_clipEnabled=true;
		updateRasterizer();
	}
}

static D3D11_STENCIL_OP stencilopToDX11(ShaderEngine::StencilOp sf)
{
	switch (sf)
	{
	case ShaderEngine::STENCIL_KEEP: return D3D11_STENCIL_OP_KEEP;
	case ShaderEngine::STENCIL_ZERO: return D3D11_STENCIL_OP_ZERO;
	case ShaderEngine::STENCIL_REPLACE: return D3D11_STENCIL_OP_REPLACE;
	case ShaderEngine::STENCIL_INCR: return D3D11_STENCIL_OP_INCR_SAT;
	case ShaderEngine::STENCIL_INCR_WRAP: return D3D11_STENCIL_OP_INCR;
	case ShaderEngine::STENCIL_DECR: return D3D11_STENCIL_OP_DECR_SAT;
	case ShaderEngine::STENCIL_DECR_WRAP: return D3D11_STENCIL_OP_DECR;
	case ShaderEngine::STENCIL_INVERT: return D3D11_STENCIL_OP_INVERT;
	}
	return D3D11_STENCIL_OP_KEEP;
}

void dx11ShaderEngine::setDepthStencil(DepthStencil state)
{
	ID3D11DepthStencilState *cs = g_pDSOff;
	if (state.dClear)
	{
		state.dClear=false;
		s_depthBufferCleared=false;
	}
	if (state.dTest)
	{
		if (!s_depthEnable)
		{
			if (currentBuffer)
				currentBuffer->needDepthStencil();
			if ((!s_depthBufferCleared)||(state.dClear))
			{
    			g_devcon->ClearDepthStencilView(currentBuffer?((dx11ShaderBuffer *)currentBuffer)->depthStencil:g_depthStencil, D3D11_CLEAR_DEPTH, 1.0, 0);
    			s_depthBufferCleared=true;
    			state.dClear=false;
			}
			cs = g_pDSDepth;
			s_depthEnable=true;
		}
	}
	else
	{
		if (s_depthEnable)
		{
			s_depthEnable=false;
		}
	}


	if (state.sClear)
	{
		if (currentBuffer)
			currentBuffer->needDepthStencil();
		g_devcon->ClearDepthStencilView(currentBuffer ? ((dx11ShaderBuffer *)currentBuffer)->depthStencil : g_depthStencil, D3D11_CLEAR_STENCIL, 0, 0);
		state.sClear = false;
	}

	if (!((state.sFail == STENCIL_KEEP) && (state.dFail == STENCIL_KEEP) && (state.dPass == STENCIL_KEEP) &&
		(state.sFunc == STENCIL_DISABLE) && (state.sMask = 0xFF) && (state.sRef == 0)))
	{
		D3D11_COMPARISON_FUNC sf = D3D11_COMPARISON_ALWAYS;
		switch (state.sFunc)
		{
		case STENCIL_NEVER: sf = D3D11_COMPARISON_NEVER; break;
		case STENCIL_LESS: sf = D3D11_COMPARISON_LESS; break;
		case STENCIL_LEQUAL: sf = D3D11_COMPARISON_LESS_EQUAL; break;
		case STENCIL_GREATER: sf = D3D11_COMPARISON_GREATER; break;
		case STENCIL_GEQUAL: sf = D3D11_COMPARISON_GREATER_EQUAL; break;
		case STENCIL_EQUAL: sf = D3D11_COMPARISON_EQUAL; break;
		case STENCIL_NOTEQUAL: sf = D3D11_COMPARISON_NOT_EQUAL; break;
		}

		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(dsDesc));
		// Depth test parameters
		dsDesc.DepthEnable = state.dTest;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		// Stencil test parameters
		dsDesc.StencilEnable = (state.sFunc != STENCIL_DISABLE);
		dsDesc.StencilReadMask = state.sMask;
		dsDesc.StencilWriteMask = state.sWMask;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = stencilopToDX11(state.sFail);
		dsDesc.FrontFace.StencilDepthFailOp = stencilopToDX11(state.dFail);
		dsDesc.FrontFace.StencilPassOp = stencilopToDX11(state.dPass);
		dsDesc.FrontFace.StencilFunc = sf;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = stencilopToDX11(state.sFail);
		dsDesc.BackFace.StencilDepthFailOp = stencilopToDX11(state.dFail);
		dsDesc.BackFace.StencilPassOp = stencilopToDX11(state.dPass);
		dsDesc.BackFace.StencilFunc = sf;

		// Create depth stencil state
		if (g_pCDSState)
			g_pCDSState->Release();
		g_dev->CreateDepthStencilState(&dsDesc, &g_pCDSState);
		cs = g_pCDSState;
	}

	g_devcon->OMSetDepthStencilState(cs, state.sRef);
	dsCurrent = state;
	updateRasterizer();
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

D3D11_BLEND dx11ShaderEngine::blendAlpha2D3D11(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case ZERO:
		return D3D11_BLEND_ZERO;
	case ONE:
		return D3D11_BLEND_ONE;
	case SRC_COLOR:
	case SRC_ALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case SRC_ALPHA_SATURATE:
		return D3D11_BLEND_SRC_ALPHA_SAT;
	case ONE_MINUS_SRC_COLOR:
	case ONE_MINUS_SRC_ALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case ONE_MINUS_DST_COLOR:
	case ONE_MINUS_DST_ALPHA:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case DST_ALPHA:
	case DST_COLOR:
		return D3D11_BLEND_DEST_ALPHA;
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
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = blendAlpha2D3D11(sfactor);// D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = blendAlpha2D3D11(dfactor);// D3D11_BLEND_DEST_ALPHA;
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

