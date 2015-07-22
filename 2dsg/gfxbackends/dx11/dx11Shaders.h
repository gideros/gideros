/*
 * gl2Shaders.h
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#ifndef DX11SHADERS_H_
#define DX11SHADERS_H_

#include <windows.h>
//#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <math.h>
#include <vector>
#include "dxglobals.h"
#include "dxcompat.hpp"
#include "Shaders.h"
#include "Matrices.h"
#include "gtexture.h"
#include "dxcompat.hpp"
#define OPENGL_DESKTOP

#ifdef OPENGL_DESKTOP
#include "glcompat.h"
#endif

#ifdef WINSTORE
extern ComPtr<ID3D11Device1> g_dev;                     // the pointer to our Direct3D device interface (11.1)
extern ComPtr<ID3D11DeviceContext1> g_devcon;           // the pointer to our Direct3D device context (11.1)
#else
extern ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
extern ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
#endif



class dx11ShaderProgram : public ShaderProgram
{
	friend class dx11ShaderEngine;
	ID3D11InputLayout *g_pLayout;
	ID3D11VertexShader *g_pVS;
	ID3D11PixelShader *g_pPS;
	static ID3D11Buffer *g_pVBuffer;                  // Vertex buffer: we put our geometry here
	static ID3D11Buffer *g_pCBuffer;                  // Vertex buffer: we put our geometry here
	static ID3D11Buffer *g_pTBuffer;                  // Vertex buffer: we put our geometry here
	static ID3D11Buffer *g_pIBuffer;                  // Vertex buffer: we put our geometry here
	ID3D11Buffer *g_CBP, *g_CBV;                        // Constant buffer: pass settings like whether to use textures or not
protected:
    std::vector<DataDesc> attributes;
    std::string errorLog;
    static ShaderProgram *current;
	static ID3D11Buffer *curIndicesVBO;
    void *cbpData;
    void *cbvData;
    bool cbpMod;
    bool cbvMod;
    int cbpsData;
    int cbvsData;
    ID3D11Buffer *genVBO[16+1];
    int genVBOcapacity[16+1];
    void setupBuffer(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache);
    ID3D11Buffer *getGenericVBO(int index,int elmSize,int mult,int count);
	ID3D11Buffer *getCachedVBO(ShaderBufferCache **cache, bool index, int elmSize, int mult, int count);
	void updateConstants();
    void buildShaderProgram(const void *vshader,int vshadersz,const void *pshader,int pshadersz,
                     const ConstantDesc *uniforms, const DataDesc *attributes);
public:
    virtual void activate();
    virtual void deactivate();
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache);
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache);
    virtual bool isValid();
    virtual const char *compilationLog();
    dx11ShaderProgram(const char *vshader,const char *pshader,
                     const ConstantDesc *uniforms, const DataDesc *attributes);
    dx11ShaderProgram(const void *vshader,int vshadersz,const void *pshader,int pshadersz,
                     const ConstantDesc *uniforms, const DataDesc *attributes);
    virtual ~dx11ShaderProgram();
};


class dx11ShaderTexture : public ShaderTexture
{
	friend class dx11ShaderBuffer;
	friend class dx11ShaderEngine;
protected:
	static ID3D11SamplerState *samplerRepeat;
	static ID3D11SamplerState *samplerClamp;
	static ID3D11SamplerState *samplerRepeatFilter;
	static ID3D11SamplerState *samplerClampFilter;
	ID3D11Texture2D *tex;
	ID3D11ShaderResourceView *rsv;
	int width,height;
	Wrap wrap;
	Filtering filter;
public:
	dx11ShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	virtual ~dx11ShaderTexture();
};

class dx11ShaderBuffer : public ShaderBuffer
{
	friend class dx11ShaderEngine;
protected:
	ID3D11RenderTargetView *renderTarget;
public:
	dx11ShaderBuffer(ShaderTexture *texture);
	virtual ~dx11ShaderBuffer();
	void readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data);
	void prepareDraw();
};

class dx11ShaderEngine : public ShaderEngine
{
	ShaderBuffer *currentBuffer;
	ID3D11RenderTargetView *g_backbuffer;
	ID3D11DepthStencilView *g_depthStencil;
	ID3D11Texture2D* g_depthStencilTexture;
	ID3D11DepthStencilState *g_pDSOff;
	ID3D11DepthStencilState *g_pDSDepth;
	ID3D11RasterizerState *g_pRSNormal;
	ID3D11RasterizerState *g_pRSScissor;
	ID3D11BlendState *g_pBlendState;
	ID3D11BlendState *g_pCBlendState;
	int s_depthEnable;
	bool s_depthBufferCleared;
	D3D11_BLEND blendFactor2D3D11(BlendFactor blendFactor);
	BlendFactor curSrcFactor, curDstFactor;
	BlendFactor curCSrcFactor, curCDstFactor;
public:
	static ID3D11Texture2D* pBackBuffer;
	dx11ShaderEngine(int sw, int sh);
	virtual ~dx11ShaderEngine();
	void reset(bool reinit=false);
	const char *getVersion();
	void setDepthTest(bool enable);
	ShaderTexture *createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	ShaderBuffer *createRenderTarget(ShaderTexture *texture);
	ShaderBuffer *setFramebuffer(ShaderBuffer *fbo);
	ShaderProgram *createShaderProgram(const char *vshader,const char *pshader,int flags, const ShaderProgram::ConstantDesc *uniforms, const ShaderProgram::DataDesc *attributes);
	void setViewport(int x,int y,int width,int height);
	void clearColor(float r,float g,float b,float a);
	void bindTexture(int num,ShaderTexture *texture);
	void setClip(int x,int y,int w,int h);
	void setBlendFunc(BlendFactor sfactor, BlendFactor dfactor);
	virtual Matrix4 setFrustum(float l, float r, float b, float t, float n, float f);
};


#endif /* GL2SHADERS_H_ */
