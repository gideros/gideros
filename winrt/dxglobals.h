#ifndef DXGLOBALS_H
#define DXGLOBALS_H

#include "pch.h"
using namespace Microsoft::WRL;

#ifdef WINSTORE
extern ComPtr<ID3D11Device1> g_dev;                     // the pointer to our Direct3D device interface (11.1)
extern ComPtr<ID3D11DeviceContext1> g_devcon;           // the pointer to our Direct3D device context (11.1)
extern ComPtr<IDXGISwapChain1> g_swapchain;             // the pointer to the swap chain interface (11.1)
extern ComPtr<IDXGIDevice3> dxgiDevice;
#else
extern IDXGISwapChain *g_swapchain;             // the pointer to the swap chain interface
extern ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
extern ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
#endif

extern ID3D11RenderTargetView *g_backbuffer;
extern ID3D11DepthStencilView *g_depthStencil;
extern ID3D11Texture2D* g_depthStencilTexture;
extern ID3D11InputLayout *g_pLayout;
extern ID3D11VertexShader *g_pVS;
extern ID3D11PixelShader *g_pPS;
extern ID3D11Buffer *g_pVBuffer;
extern ID3D11Buffer *g_pCBuffer;
extern ID3D11Buffer *g_pTBuffer;
extern ID3D11Buffer *g_pIBuffer;
extern float backcol[];
extern ID3D11Buffer *g_CBP;
extern ID3D11Buffer *g_CBV;
extern ID3D11SamplerState *g_samplerLinear;
extern ID3D11BlendState *g_pBlendState;
extern ID3D11DepthStencilState *g_pDSOff;
extern ID3D11DepthStencilState *g_pDSDepth;
extern ID3D11RasterizerState *g_pRSNormal;
extern ID3D11RasterizerState *g_pRSScissor;

struct cbp {
	DirectX::XMFLOAT4 fColor;
	float fTextureSel;
	float fColorSel;
	int r1, r2; //Padding
	bool dirty;
};

struct cbv {
	DirectX::XMFLOAT4X4 mvp;
	bool dirty;
};

extern struct cbv cbvData;
extern struct cbp cbpData;

#endif
