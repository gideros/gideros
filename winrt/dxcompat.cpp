#include <windows.h>
//#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <math.h>
#include <vector>
#include "dxglobals.h"
#include "dxcompat.hpp"
//#include "util.h"

#include "glog.h"

// This is needed for WinRT apps
#ifdef WINSTORE
#include <memory>
#include "pch.h"
using namespace Microsoft::WRL;
#endif

using namespace std;

// ######################################################################

// The following parameters must be set by main program before we draw anything
// For Windows Store and WP8, we use 11.1 objects for g_dev, g_devcon, g_swapchain

#ifdef WINSTORE
ComPtr<ID3D11Device1> g_dev;                     // the pointer to our Direct3D device interface (11.1)
ComPtr<ID3D11DeviceContext1> g_devcon;           // the pointer to our Direct3D device context (11.1)
ComPtr<IDXGISwapChain1> g_swapchain;             // the pointer to the swap chain interface (11.1)
ComPtr<IDXGIDevice3> dxgiDevice;
#else
ID3D11Device *g_dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *g_devcon;           // the pointer to our Direct3D device context
IDXGISwapChain *g_swapchain;             // the pointer to the swap chain interface
#endif

ID3D11RenderTargetView *g_backbuffer;
ID3D11DepthStencilView *g_depthStencil;
ID3D11Texture2D* g_depthStencilTexture;
ID3D11InputLayout *g_pLayout;
ID3D11VertexShader *g_pVS;
ID3D11PixelShader *g_pPS;
ID3D11Buffer *g_pVBuffer;                  // Vertex buffer: we put our geometry here
ID3D11Buffer *g_pCBuffer;                  // Vertex buffer: we put our geometry here
ID3D11Buffer *g_pTBuffer;                  // Vertex buffer: we put our geometry here
ID3D11Buffer *g_pIBuffer;                  // Vertex buffer: we put our geometry here
ID3D11Buffer *g_CBP, *g_CBV;                        // Constant buffer: pass settings like whether to use textures or not
vector<ID3D11ShaderResourceView*> g_RSV;   // list of textures.
vector<bool> g_RSVused;                    // true if texture index number has been assigned

struct cbv cbvData;
struct cbp cbpData;

struct Backcol
{
	float red;
	float green;
	float blue;
	float alpha;
};

vector<ID3D11RenderTargetView*> g_renderTarget;
vector<bool> g_renderTargetused;
vector<Backcol> g_renderTargetCol;

ID3D11SamplerState *g_samplerLinear;
ID3D11BlendState *g_pBlendState;
ID3D11DepthStencilState *g_pDSOff;
ID3D11DepthStencilState *g_pDSDepth;
ID3D11RasterizerState *g_pRSNormal;
ID3D11RasterizerState *g_pRSScissor;

bool dxcompat_force_lines = false;
bool dxcompat_zrange01 = true;

int dxcompat_maxvertices = 16384;

// "OpenGL" state machine
static float g_r=1, g_g=1, g_b=1, g_a=1;
static bool g_color_array=false, g_tex_array=false;
static bool g_modelview=true;
static GLuint g_curr_texind, g_curr_framebuffer=0;

float backcol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
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

void glClear(GLbitfield mask){

	if (mask&GL_DEPTH_BUFFER_BIT)
	{
		g_devcon->ClearDepthStencilView(g_depthStencil, D3D11_CLEAR_DEPTH, 1.0, 0);
	}
	if (mask&GL_STENCIL_BUFFER_BIT)
	{
		g_devcon->ClearDepthStencilView(g_depthStencil, D3D11_CLEAR_STENCIL, 1.0, 0);
	}
	if (mask& GL_COLOR_BUFFER_BIT)
	{
		if (g_curr_framebuffer == 0){
			g_devcon->ClearRenderTargetView(g_backbuffer, backcol);
		}
		else{
			float col[4];
			col[0] = g_renderTargetCol[g_curr_framebuffer].red;
			col[1] = g_renderTargetCol[g_curr_framebuffer].green;
			col[2] = g_renderTargetCol[g_curr_framebuffer].blue;
			col[3] = g_renderTargetCol[g_curr_framebuffer].alpha;

			g_devcon->ClearRenderTargetView(g_renderTarget[g_curr_framebuffer], col);
		}
	}
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	if (g_curr_framebuffer == 0){
		backcol[0] = red;
		backcol[1] = green;
		backcol[2] = blue;
		backcol[3] = alpha;
	}
	else {
		g_renderTargetCol[g_curr_framebuffer].red = red;
		g_renderTargetCol[g_curr_framebuffer].green = green;
		g_renderTargetCol[g_curr_framebuffer].blue = blue;
		g_renderTargetCol[g_curr_framebuffer].alpha = alpha;
	}
}

// GenTextures adds new texture RSV to list g_RSV and sets to NULL
// also sets g_RSVused to true so that the same index will not be reissued
// does not actually allocate any memory. Done in glTexImage2D
// glGenTextures(2,myarray);
// g_RSV:
// 0 NULL true
// 1 NULL true (myarray[0],myarray[1]=0,1)
// glTexImage2D (1=current)
// 0 = NULL  true
// 1 = 0x111 true
// glDeleteTextures(1,&mytexind); mytexind=0
// 0 = NULL  false// deleted  << CAN BE REUSED by another glGenTextures
// 1 = 0x111 true // keeps same number

void glGenTextures(GLsizei n, GLuint *texinds)
{
  int i,j;  // i: index in texinds, j: index in g_RSV

  i=0;
  for (j=0;j<g_RSV.size();j++){
	  if (!g_RSVused[j]){
		  g_RSV[j] = NULL;
		  g_RSVused[j] = true;
		  texinds[i] = j;
		  i++;
		  if (i == n) return;
	  }
  }

  while (true){
    g_RSV.push_back(NULL);
	g_RSVused.push_back(true);
    texinds[i]=j;
    i++;
    j++;
    if (i==n) return;
  }
}

// eg glDeleteTextures(2,{3,4})
void glDeleteTextures(GLsizei n, GLuint *texinds)
{
  int i,count;

  for (i=0;i<n;i++){
    GLuint mytexind=texinds[i];
	if (mytexind < g_RSV.size()){
		if (g_RSV[mytexind] != NULL) g_RSV[mytexind]->Release();
		g_RSV[mytexind] = NULL;
		g_RSVused[mytexind] = false;
	}
  }

  // remove end of list if full of unused slots
  count=0;
  for (i=g_RSV.size()-1;i>=0;i--){
    if (g_RSVused[i]) break;
    count++;
  }

  for (i = 0; i < count; i++){
	  g_RSV.pop_back();
	  g_RSVused.pop_back();
  }
}

void glBindTexture(GLenum target, GLuint texind)
{

	if (target != GL_TEXTURE_2D){
		glog_e("glBindTexture: wrong target\n");
		exit(1);
	}

	if (texind >= g_RSV.size()) return;
	if (!g_RSVused[texind]) return;

    g_curr_texind=texind;

//	if (g_RSV[texind]!=NULL)
		g_devcon->PSSetShaderResources(0,1,&g_RSV[texind]);
}

// bind current texture g_curr_texind
void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{

  ID3D11Texture2D *tex;
  D3D11_TEXTURE2D_DESC tdesc;
  D3D11_SUBRESOURCE_DATA tbsd;

  char *pixels1,*pixels2;
  int i;

  if (target != GL_TEXTURE_2D){
	  glog_e("glTexImage2D: unknown target\n");
	  exit(1);
  }

  if (level != 0) glog_w("glTexImage2D: level not zero\n");

  if (border != 0) {
	  glog_e("glTexImage2D: border must be zero\n");
	  exit(1);
  }

  if (format != internalFormat) glog_w("glTexImage2D: Warning format, internalFormat different\n");

  if (type != GL_UNSIGNED_BYTE) glog_w("glTexImage2D: unexpected pixel data type\n");

  pixels1 = (char *)pixels;

//  for (int i = 0; i < 400;i++)
//	  pixels1[i] = 255;

  tbsd.SysMemPitch = width * 4;
  tbsd.SysMemSlicePitch = width*height * 4; // not needed

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
  tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  if (internalFormat==GL_RGBA){
	tbsd.pSysMem = pixels;
  }
  else if (internalFormat==GL_RGB){
	  pixels2 = (char *)malloc(width*height * 4);
	  for (i = 0; i < width*height; i++){
		  pixels2[i * 4] = pixels1[i * 3];
		  pixels2[i * 4+1] = pixels1[i * 3+1];
		  pixels2[i * 4+2] = pixels1[i * 3+2];
		  pixels2[i * 4+3] = 255;
	  }
	  tbsd.pSysMem = pixels2;
  }
  else {
	  glog_w("glTexImage2D: unknown internal format");
	  exit(1);
  }

  g_dev->CreateTexture2D(&tdesc,&tbsd,&tex);
  g_dev->CreateShaderResourceView(tex,NULL,&g_RSV[g_curr_texind]);
  tex->Release();  // We only need the resource view

  g_devcon->PSSetShaderResources(0,1,&g_RSV[g_curr_texind]);

  if (internalFormat == GL_RGB) free(pixels2);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param )
{
}

void glPixelStorei(GLenum pname, GLint param)
{
}

void glColor4ub(GLubyte r,GLubyte g, GLubyte b, GLubyte a)
{
  g_r=r/255.0;
  g_g=g/255.0;
  g_b=b/255.0;
  g_a=a/255.0;
}

//######################################################################
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  g_r=r;
  g_g=g;
  g_b=b;
  g_a=a;
}

//######################################################################
void glEnableClientState(GLenum type)
{
}

//######################################################################
void glDisableClientState(GLenum type)
{
}

void glEnable(GLenum type)
{
	switch (type)
	{
	case GL_DEPTH_TEST:
		g_devcon->OMSetDepthStencilState(g_pDSDepth, 1);
		break;
	case GL_SCISSOR_TEST:
		g_devcon->RSSetState(g_pRSScissor);
		break;
	}
}

void glDisable(GLenum type)
{
	switch (type)
	{
	case GL_DEPTH_TEST:
		g_devcon->OMSetDepthStencilState(g_pDSOff, 1);
		break;
	case GL_SCISSOR_TEST:
		g_devcon->RSSetState(g_pRSNormal);
		break;
	}
}

void memdump(const char *chn, const void *bv, int sz) {
	int csz, done = 0, i;
	const unsigned char *b = (const unsigned char *)bv;
	glog_i("[%s]\n", chn);
	while (sz) {
		csz = sz;
		if (sz > 16)
			csz = 16;
		glog_i("%08lx:", done);
		{
			for (i = 0; i < csz; i++)
				glog_i("%02x ", b[i]);
			for (i = csz; i < 16; i++)
				glog_i("   ");
		}
		if (0)
			for (i = 0; i < csz; i++) {
				int c = b[i];
				glog_i("%c", (((c < 32) || (c > 127)) ? '.' : c));
			}
		glog_i("\n");
		b += csz;
		done += csz;
		sz -= csz;
	}
}

void floatdump(const char *chn, const void *bv, int sz) {
	int csz, done = 0, i;
	const float *b = (const float *)bv;
	glog_i("[%s]\n", chn);
	while (sz) {
		csz = sz;
		if (sz > 4)
			csz = 4;
		glog_i("%08lx:", done);
		{
			for (i = 0; i < csz; i++)
				glog_i("%8.6f ", b[i]);
			for (i = csz; i < 16; i++)
				glog_i("                ");
		}
		if (0)
			for (i = 0; i < csz; i++) {
				int c = b[i];
				glog_i("%c", (((c < 32) || (c > 127)) ? '.' : c));
			}
		glog_i("\n");
		b += csz;
		done += csz;
		sz -= csz;
	}
}

void glVertexAttribPointer(GLuint  index, GLint  size, GLenum  type, GLboolean  normalized, GLsizei  stride, const GLvoid *  pointer,GLsizei count, bool modified, GLuint *cache)
{
	if (size > dxcompat_maxvertices) size = dxcompat_maxvertices;  // avoid overflow
	ID3D11Buffer *vbo=NULL;
	const char *vName = "VB";
	switch (index)
	{
	case 0:
		vbo = g_pVBuffer;
		break;
	case 1:
		vbo = g_pCBuffer;
		vName = "CB";
		break;
	case 2:
		vbo = g_pTBuffer;
		vName = "TB";
		break;
	default:
		return;
	}
	int elmSize = 1;
	switch (type)
	{
	case GL_FLOAT:
		elmSize = sizeof(GLfloat);
		break;
	}
	D3D11_MAPPED_SUBRESOURCE ms;
	g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer    
	if ((index == 0) && (size == 2)) //special case expand X,Y to X,Y,0 data
	{
		float *vdi = (float *)pointer;
		float *vdo = (float *)ms.pData;
		for (int k = 0; k < count; k++)
		{
			*(vdo++) = *(vdi++);
			*(vdo++) = *(vdi++);
			*(vdo++) = 0;
		}
		size = 3;
	}
	else
		memcpy(ms.pData, pointer, size*elmSize*count);                 // copy the data    
	//floatdump(vName, ms.pData, 8);
	g_devcon->Unmap(vbo, NULL);                                      // unmap the buffer

	UINT tstride = size*elmSize;
	UINT offset = 0;

	g_devcon->IASetVertexBuffers(index, 1, &vbo, &tstride, &offset);
}

void glEnableVertexAttribArray(GLuint index)
{
	switch (index)
	{
	case 1:
		if (cbpData.fColorSel == 0)
		{
			cbpData.fColorSel = 1.0;
			cbpData.dirty = true;
		}
		break;
	case 2:
		if (cbpData.fTextureSel == 0)
		{
			cbpData.fTextureSel = 1.0;
			cbpData.dirty = true;
		}
		break;
	}
}

void glDisableVertexAttribArray(GLuint index)
{
	switch (index)
	{
	case 1:
		if (cbpData.fColorSel != 0)
		{
			cbpData.fColorSel = 0.0;
			cbpData.dirty = true;
		}
		break;
	case 2:
		if (cbpData.fTextureSel != 0)
		{
			cbpData.fTextureSel = 0.0;
			cbpData.dirty = true;
		}
		break;
	}
}


void glUniform1f(GLint location, GLfloat v0)
{
	switch (location)
	{
	case 22: //fColorSel
		cbpData.fColorSel = v0;
		cbpData.dirty = true;
		break;
	case 23: //fTextureSel
		cbpData.fTextureSel = v0;
		cbpData.dirty = true;
		break;
	}
}

void glUniform1i(GLint location, GLint v0)
{
	switch (location)
	{
	case 21: //fTexture
		break;
	}
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	switch (location)
	{
	case 20: //fColor
		cbpData.fColor=DirectX::XMFLOAT4(v0,v1,v2,v3);
		cbpData.dirty = true;
		break;
	}
}
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	switch (location)
	{
	case 10: //vMatrix
		cbvData.mvp = DirectX::XMFLOAT4X4(value);
		cbvData.dirty = true;
		break;
	}
}

//######################################################################
void updateShaders()
{
	//Update CB{V,P} data
	if (cbpData.dirty)
	{
		//floatdump("CBP", &cbpData, 6);
		g_devcon->UpdateSubresource(g_CBP, 0, NULL, &cbpData, 0, 0);
		g_devcon->PSSetConstantBuffers(0, 1, &g_CBP);
		cbpData.dirty = false;
	}
	if (cbvData.dirty)
	{
		g_devcon->UpdateSubresource(g_CBV, 0, NULL, &cbvData, 0, 0);
		g_devcon->VSSetConstantBuffers(0, 1, &g_CBV);
		cbvData.dirty = false;
	}
}

void glDrawArrays(GLenum pattern, GLint zero, GLsizei npoints)
{
	updateShaders();

  if (dxcompat_force_lines)
     pattern = GL_LINE_STRIP;

  if (pattern == GL_POINTS)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  else if (pattern == GL_TRIANGLE_STRIP)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  else if (pattern == GL_TRIANGLES)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  else if (pattern == GL_LINE_STRIP)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  else if (pattern == GL_LINE_LOOP)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  else if (pattern==GL_LINES)
	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  else {
	  glog_e("glDrawArrays unknown pattern\n");
	  exit(1);
  }

  g_devcon->Draw(npoints,0);
}

void glLineWidth(GLfloat width)
{
}

// ================ NEW =========================

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{

}



void glGetIntegerv(GLenum pname, GLint *params)
{
	if (pname == GL_TEXTURE_BINDING_2D)
		*params = g_curr_texind;
	else if (pname == GL_FRAMEBUFFER_BINDING)
		*params = g_curr_framebuffer;
	else {
		glog_w("Warning, glGetIntegerv pname not supported\n");
		*params = 0;
	}
}

const GLubyte *glGetString(GLenum name)
{
	glog_w("glGetString not supported\n");
	return NULL;
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	glog_v("glTexEnvi not supported\n");
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, bool modified, GLuint *cache)
{
	D3D11_MAPPED_SUBRESOURCE ms;

	updateShaders();

	int indiceSize = 4;
	DXGI_FORMAT iFmt = DXGI_FORMAT_R32_UINT;


	if (count > dxcompat_maxvertices) count = dxcompat_maxvertices;  // prevent overflow

	if (type == GL_UNSIGNED_SHORT)
	{
		indiceSize = 2;
		iFmt = DXGI_FORMAT_R16_UINT;
	}

	g_devcon->Map(g_pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer    
	memcpy(ms.pData, indices, indiceSize*count);                 // copy the data    
	g_devcon->Unmap(g_pIBuffer, NULL);                                      // unmap the buffer

	UINT stride = indiceSize;
	UINT offset = 0;

	g_devcon->IASetIndexBuffer(g_pIBuffer, iFmt,0);

	if (dxcompat_force_lines)
		mode = GL_LINE_STRIP;

	if (mode == GL_POINTS)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	else if (mode == GL_TRIANGLE_STRIP)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (mode == GL_TRIANGLES)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (mode == GL_LINE_STRIP)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	else if (mode == GL_LINE_LOOP)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	else if (mode == GL_LINES)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	else {
		glog_e("glDrawElements: bad mode\n");
		exit(1);
	}


	g_devcon->DrawIndexed(count, 0, 0);

}


void glBindBuffer(GLenum target, GLuint buffer)
{
}

void glDeleteFramebuffers(GLsizei n, GLuint *framebuffers)
{
	int i, count;

	for (i = 0; i<n; i++){
		GLuint mytexind = framebuffers[i];
		if (mytexind < g_renderTarget.size()){
			if (g_renderTarget[mytexind] != NULL) g_renderTarget[mytexind]->Release();   // delete rendertarget
			g_renderTarget[mytexind] = NULL;
			g_renderTargetused[mytexind] = false;
		}
	}

	// remove end of list if full of unused slots
	count = 0;
	for (i = g_renderTarget.size() - 1; i >= 0; i--){
		if (g_renderTargetused[i]) break;
		count++;
	}

	for (i = 0; i < count; i++){
		g_renderTarget.pop_back();
		g_renderTargetused.pop_back();
		g_renderTargetCol.pop_back();
	}

}

void glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
	int i, j;  // i: index in framebuffers, j: index in g_renderTarget
	Backcol col = { 1.0f, 1.0f, 1.0f, 1.0f };

	i = 0;
	for (j = 0; j<g_renderTarget.size(); j++){
		if (!g_renderTargetused[j]){
			g_renderTarget[j] = NULL;
			g_renderTargetused[j] = true;
			g_renderTargetCol[j] = col;

			framebuffers[i] = j;
			i++;
			if (i == n) return;
		}
	}

	while (true){
		g_renderTarget.push_back(NULL);
		g_renderTargetused.push_back(true);
		g_renderTargetCol.push_back(col);

		framebuffers[i] = j;
		i++;
		j++;
		if (i == n) return;
	}

}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	if (target != GL_FRAMEBUFFER){
		glog_e("glBindFramebuffer: wrong target\n");
		exit(1);
	}

	if (framebuffer == 0){
		g_devcon->OMSetRenderTargets(1, &g_backbuffer, g_depthStencil);  // draw on screen (actually back buffer)
		g_curr_framebuffer = 0;
		return;
	}

	if (framebuffer >= g_renderTarget.size()) return;
	if (!g_renderTargetused[framebuffer]) return;

	g_curr_framebuffer = framebuffer;

	if (g_renderTarget[g_curr_framebuffer]!=NULL)
		g_devcon->OMSetRenderTargets(1, &g_renderTarget[g_curr_framebuffer], NULL);  // draw on texture
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ID3D11Resource *res;

	if (target != GL_FRAMEBUFFER){
		glog_e("glFramebufferTexture2D: bad target\n");
		exit(1);
	}

	if (attachment != GL_COLOR_ATTACHMENT0){
		glog_e("glFramebufferTexture2D: bad attachment\n");
		exit(1);
	}

	if (textarget != GL_TEXTURE_2D){
		glog_e("glFramebufferTexture2D: bad textarget\n");
		exit(1);
	}

	if (level != 0){
		glog_e("glFramebufferTexture2D: bad level\n");
		exit(1);
	}

	if (g_RSV[texture] == NULL) return;

	g_RSV[texture]->GetDesc(&desc);
	g_RSV[texture]->GetResource(&res);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));

	renderTargetViewDesc.Format = desc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	g_dev->CreateRenderTargetView(res, &renderTargetViewDesc, &g_renderTarget[g_curr_framebuffer]);  // create rendertarget
	g_devcon->OMSetRenderTargets(1, &g_renderTarget[g_curr_framebuffer], NULL);   // draw on texture
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{

}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	glog_v("glTexParameteri not supported\n");
}


void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	D3D11_RECT pRect;
	pRect.left = x;
	pRect.top = y;
	pRect.right = x + width - 1;
	pRect.bottom = y + height - 1;
	g_devcon->RSSetScissorRects(1, &pRect);
}

void glDepthFunc(GLenum func)
{

}

GLuint glCreateShader(GLenum shaderType)  { return 0; }
GLuint glCreateProgram(void) { return 0;  }
void glCompileShader(GLuint shader) {}
void glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) {}
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {}
void glGetProgramiv(GLuint program, GLenum pname, GLint *params) {}
void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {}
void glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {}
void glDeleteShader(GLuint shader) {}
void glAttachShader(GLuint program, GLuint shader) {}
void glLinkProgram(GLuint program) {}
void glUseProgram(GLuint program) {}
void glDeleteProgram(GLuint program) {}
void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{

}

GLint glGetAttribLocation(GLuint program, const GLchar *name)
{
	if (!strcmp(name, "vVertex")) return 0;
	if (!strcmp(name, "vColor")) return 1;
	if (!strcmp(name, "vTexCoord")) return 2;
	return -1;
}

GLint glGetUniformLocation(GLuint program, const GLchar *name)
{
	if (!strcmp(name, "vMatrix")) return 10;
	if (!strcmp(name, "fColor")) return 20;
	if (!strcmp(name, "fTexture")) return 21;
	if (!strcmp(name, "fColorSel")) return 22;
	if (!strcmp(name, "fTextureSel")) return 23;
	return -1;
}


void glActiveTexture(GLenum texture)
{

}


// ######################################################################
// On WinRT these functions do not exist (headers are present for compiling)
// but they are needed by Lua so put do-nothing functions for link stage

#ifdef WINSTORE
extern "C"{
  char *getenv(const char *string){ return NULL; }
  int system(const char *string){ return 0; }
}
#endif
