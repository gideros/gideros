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

#ifdef WINSTORE
ComPtr<ID3D11Device1> g_dev;                     // the pointer to our Direct3D device interface (11.1)
ComPtr<ID3D11DeviceContext1> g_devcon;           // the pointer to our Direct3D device context (11.1)
#else
ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
#endif

void dx11ShaderEngine::reset()
{
	ShaderEngine::reset();
	g_devcon->OMSetDepthStencilState(g_pDSOff, 1);
	g_devcon->RSSetState(g_pRSNormal);
	s_depthEnable=0;
	s_depthBufferCleared=false;

    oglCombined.identity();

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
	g_devcon->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFF);
}

void dx11SetupShaders()
{
	const dx11ShaderProgram::ConstantDesc stdConstants[]={
			{"vMatrix",dx11ShaderProgram::CMATRIX,true,0},
			{"fColor",dx11ShaderProgram::CFLOAT4,false,0},
			NULL
	};
	const dx11ShaderProgram::DataDesc stdAttributes[]={
			{"vVertex",dx11ShaderProgram::DFLOAT,3,0,0},
			{"vColor",dx11ShaderProgram::DFLOAT,4,1,0},
			{"vTexCoord",dx11ShaderProgram::DFLOAT,2,2,0},
			NULL
	};

    ShaderProgram::stdBasic = new dx11ShaderProgram("assets/vBasic.cso","assets/pBasic.cso",stdConstants,stdAttributes);
    ShaderProgram::stdColor = new dx11ShaderProgram("assets/vColor.cso","assets/pColor.cso",stdConstants,stdAttributes);
    ShaderProgram::stdTexture = new dx11ShaderProgram("assets/vTexture.cso","assets/pTexture.cso",stdConstants,stdAttributes);
    ShaderProgram::stdTextureColor = new dx11ShaderProgram("assets/vTextureColor.cso","assets/pTextureColor.cso",stdConstants,stdAttributes);
}

ID3D11Texture2D* dx11ShaderEngine::pBackBuffer=NULL;

dx11ShaderEngine::dx11ShaderEngine(int sw,int sh)
{
	currentBuffer=NULL;
	matrixDirty=true;
	constColR=1;
	constColG=1;
	constColB=1;
	constColA=1;
	colorDirty=true;
	dx11ShaderProgram::current=NULL;

#ifndef GIDEROS_GL1
 dx11SetupShaders();
#endif

 D3D11_TEXTURE2D_DESC backbuff_desc;
 pBackBuffer->GetDesc(&backbuff_desc);

 g_dev->CreateRenderTargetView(pBackBuffer, NULL, &g_backbuffer);
 pBackBuffer->Release();

 g_devcon->OMSetRenderTargets(1, &g_backbuffer, NULL);  // could call this "rendertarget"


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
	g_devcon->OMSetRenderTargets(1, fbo?&(((dx11ShaderBuffer *)fbo)->renderTarget):&g_backbuffer, g_depthStencil);
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

void dx11ShaderEngine::setModel(const Matrix4 m)
{
	ShaderEngine::setModel(m);
	oglCombined=oglProjection*oglModel;
    matrixDirty=true;
}

void dx11ShaderEngine::setProjection(const Matrix4 p)
{
	ShaderEngine::setProjection(p);
}

void dx11ShaderEngine::clearColor(float r,float g,float b,float a)
{
	float col[4]={r,g,b,a};
	g_devcon->ClearRenderTargetView(((dx11ShaderBuffer *)currentBuffer)->renderTarget, col);
}

void dx11ShaderEngine::setColor(float r,float g,float b,float a)
{
    constColR=r;
    constColG=g;
    constColB=b;
    constColA=a;
    colorDirty=true;
}


void dx11ShaderEngine::bindTexture(int num,ShaderTexture *texture)
{
	dx11ShaderTexture *tex=(dx11ShaderTexture *)texture;
	g_devcon->PSSetShaderResources(num,1,&tex->rsv);
	if (tex->wrap==ShaderTexture::WRAP_CLAMP)
		g_devcon->PSSetSamplers(0, 1, &dx11ShaderTexture::samplerClamp);
	else
		g_devcon->PSSetSamplers(0, 1, &dx11ShaderTexture::samplerRepeat);
}

void dx11ShaderEngine::preDraw(ShaderProgram *program)
{
   	program->setConstant(ShaderProgram::ConstantMatrix,ShaderProgram::CMATRIX,oglCombined.data());
   	float constCol[4]={constColR,constColG,constColB,constColA};
   	program->setConstant(ShaderProgram::ConstantColor,ShaderProgram::CFLOAT4,constCol);
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
		pRect.right = x + w - 1;
		pRect.bottom = y + h - 1;
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

/*
GLenum dx11ShaderEngine::blendFactor2GLenum(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case ZERO:
		   return GL_ZERO;
		case ONE:
		   return GL_ONE;
		case SRC_COLOR:
		   return GL_SRC_COLOR;
		case ONE_MINUS_SRC_COLOR:
		   return GL_ONE_MINUS_SRC_COLOR;
		case DST_COLOR:
		   return GL_DST_COLOR;
		case ONE_MINUS_DST_COLOR:
		   return GL_ONE_MINUS_DST_COLOR;
		case SRC_ALPHA:
		   return GL_SRC_ALPHA;
		case ONE_MINUS_SRC_ALPHA:
		   return GL_ONE_MINUS_SRC_ALPHA;
		case DST_ALPHA:
		   return GL_DST_ALPHA;
		case ONE_MINUS_DST_ALPHA:
		   return GL_ONE_MINUS_DST_ALPHA;
		//case CONSTANT_COLOR:
		//   return GL_CONSTANT_COLOR;
		//case ONE_MINUS_CONSTANT_COLOR:
		//   return GL_ONE_MINUS_CONSTANT_COLOR;
		//case CONSTANT_ALPHA:
		//   return GL_CONSTANT_ALPHA;
		//case ONE_MINUS_CONSTANT_ALPHA:
		//   return GL_ONE_MINUS_CONSTANT_ALPHA;
		case SRC_ALPHA_SATURATE:
		   return GL_SRC_ALPHA_SATURATE;
	}

	return GL_ZERO;
}*/

void dx11ShaderEngine::setBlendFunc(BlendFactor sfactor, BlendFactor dfactor)
{
	//TODO glBlendFunc(blendFactor2GLenum(sfactor), blendFactor2GLenum(dfactor));
}

