/*
 * gl2ShaderProgram.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "dx11Shaders.h"
#include "glog.h"
//#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include "platform.h"
#include <gstdio.h>
#include <io.h>
#include "D3Dcompiler.h"
//using namespace Microsoft::WRL;

class dx11ShaderBufferCache : public ShaderBufferCache {
public:
	ID3D11Buffer *VBO;
	int VBOcapacity;
	int cachedMult;
	unsigned int offset; //offset of chunk, if small enough to be packed
	unsigned int size; //size of chunk, if small enough to be packed
	dx11ShaderBufferCache() { VBO = NULL; VBOcapacity = 0; cachedMult = 0; size = 0; offset = 0; }
	virtual ~dx11ShaderBufferCache()
	{
		if (VBO)
		{
#ifdef DX11SHADERS_COMMON_GENVBO
			if (!size)
				deleteVbo(1, &VBO);
			else
				freeVboPack(VBO, size);
#else
			VBO->Release();
#endif
		}
	}
#ifdef DX11SHADERS_COMMON_GENVBO
	struct VboPack {
		bool index;
		unsigned int used;
		unsigned int freeed;
	};
	void recreate()
	{
		if (VBO)
		{
			if (!size)
				deleteVbo(1, &VBO);
			else
				freeVboPack(VBO, size);
		}
		VBO = 0;
	}
	bool valid()
	{
		return (VBO != nullptr);
	}
	static std::map<ID3D11Buffer*, VboPack> vboPacks;
	static ID3D11Buffer* openVboPack[2];
	static void freeVboPack(ID3D11Buffer* vbo, unsigned int size) {
		vboPacks[vbo].freeed += ((size + 3) & (~3)); //Align
	}
	//At the beginning of each frame
	static void turnVboPacks() {
		for (size_t k = 0; k < 2; k++)
			if (openVboPack[k]) {
				if (vboPacks[openVboPack[k]].used)
					openVboPack[k] = 0; //Close open Vbo pack if any
			}
		std::vector<std::pair<bool,ID3D11Buffer*>> freeed;
		for (auto it : vboPacks) {
			auto& pack = it.second;
			if (pack.freeed == pack.used)
				freeed.push_back(std::make_pair(pack.index,it.first));
		}
		if (!freeed.empty()) {
			for (auto it : freeed) {
				vboPacks.erase(it.second);
				if (dx11ShaderProgram::nstdVboSet.count(it.second))
					deleteVbo(1, &it.second);
				else
					dx11ShaderProgram::freeVBOs[it.first?1:0].push_back(it.second);
			}
		}
	}
	static void freeVboPacks() {
		for (size_t k = 0; k < 2; k++)
			openVboPack[k] = 0;
		std::vector<ID3D11Buffer*> freeed;
		for (auto it : vboPacks)
			freeed.push_back(it.first);
		if (!freeed.empty()) {
			vboPacks.clear();
			deleteVbo(freeed.size(), freeed.data());
		}
	}
	static void deleteVbo(int count, ID3D11Buffer** vbos) {
		dx11ShaderProgram::deleteVBO(count, vbos);
	}
#endif
};

#define IDXBUFSIZE  65536

ShaderProgram *dx11ShaderProgram::current = NULL;
ID3D11Buffer* dx11ShaderProgram::curIndicesVBO = NULL;
#ifdef DX11SHADERS_COMMON_GENVBO
std::map<ID3D11Buffer*, dx11ShaderBufferCache::VboPack> dx11ShaderBufferCache::vboPacks;
ID3D11Buffer* dx11ShaderBufferCache::openVboPack[2] = { nullptr, nullptr };
std::unordered_set<ID3D11Buffer*> dx11ShaderProgram::nstdVboSet;
std::vector<ID3D11Buffer*> dx11ShaderProgram::freeVBOs[2];
std::vector<ID3D11Buffer*> dx11ShaderProgram::usedVBOs[2];
std::vector<ID3D11Buffer*> dx11ShaderProgram::renderedVBOs[2];
ID3D11Buffer* dx11ShaderProgram::curGenVBO[2] = { nullptr, nullptr };
size_t dx11ShaderProgram::genBufferOffset[2] = { 0,0 };

void dx11ShaderProgram::deleteVBO(int count, ID3D11Buffer** vbos) {
	for (int k = 0; k < count; k++) {
		if (vbos[k] == curIndicesVBO)
			curIndicesVBO = nullptr;
		nstdVboSet.erase(vbos[k]);
		vbos[k]->Release();
	}
}

ID3D11Buffer* dx11ShaderProgram::allocateVBO(bool index,size_t size, bool gen, bool standard) {
	D3D11_BUFFER_DESC bd;
	ID3D11Buffer* vbo;

	int bt = index ? 1 : 0;
	if ((!standard)||freeVBOs[bt].empty()) {
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;    // write access access by CPU and GPU
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU to write in buffer
		bd.ByteWidth = size;
		bd.BindFlags =
			(!index) ?
			D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER; // use as a vertex buffer
		g_dev->CreateBuffer(&bd, NULL, &vbo);
		if (gen)
			usedVBOs[bt].push_back(vbo);
		if (!standard)
			nstdVboSet.insert(vbo);
		return vbo;
	}
	vbo = freeVBOs[bt].back();
	freeVBOs[bt].pop_back();
	if (gen)
		usedVBOs[bt].push_back(vbo);
	return vbo;
}
#else
#endif

void dx11ShaderProgram::reset(bool reinit)
{
#ifdef DX11SHADERS_COMMON_GENVBO
	for (int k = 0; k < 2; k++) {
		for (auto vbo : renderedVBOs[k])
		{
			if (nstdVboSet.count(vbo))
				deleteVBO(1, &vbo);
			else
				freeVBOs->push_back(vbo);
		}
		renderedVBOs[k].clear();
		renderedVBOs[k].assign(usedVBOs[k].begin(), usedVBOs[k].end());
		usedVBOs[k].clear();
		curGenVBO[k] = 0;
	}
#endif
	dx11ShaderBufferCache::turnVboPacks();
	if (reinit) {
		dx11ShaderBufferCache::freeVboPacks();
		for (int k = 0; k < 2; k++) {
			deleteVBO(freeVBOs[k].size(), freeVBOs[k].data());
			deleteVBO(renderedVBOs[k].size(), renderedVBOs[k].data());
			deleteVBO(usedVBOs[k].size(), usedVBOs[k].data());
			freeVBOs[k].clear();
			renderedVBOs[k].clear();
			usedVBOs[k].clear();
			curGenVBO[k] = 0;
		}
	}
}

bool dx11ShaderProgram::isValid() {
	return g_pLayout != NULL;
}

const char *dx11ShaderProgram::compilationLog() {
	return errorLog.c_str();
}

void dx11ShaderProgram::deactivate() {
	current = NULL;
}

void dx11ShaderProgram::activate() {
	if (current == this)
		return;
	if (current)
		current->deactivate();
	current = this;
	g_devcon->VSSetShader(g_pVS, 0, 0);
	g_devcon->PSSetShader(g_pPS, 0, 0);
	g_devcon->PSSetConstantBuffers(1, 1, &g_CBP);
	g_devcon->VSSetConstantBuffers(0, 1, &g_CBV);
	g_devcon->IASetInputLayout(g_pLayout);
}

void dx11ShaderProgram::bindTexture(int num, ShaderTexture* texture)
{
	int ntex = textureMap[num];
	((dx11ShaderEngine*)(ShaderEngine::Engine))->bindTexture(num, texture, (ntex & 0x100) | num);
}


ID3D11Buffer *dx11ShaderProgram::getCachedVBO(ShaderBufferCache **cache, bool index,int elmSize, int mult,
	int count, bool &modified, size_t& offset) {
	if (!*cache)
		*cache = new dx11ShaderBufferCache();
	dx11ShaderBufferCache *dc = static_cast<dx11ShaderBufferCache*> (*cache);
	offset = 0;
#ifdef DX11SHADERS_COMMON_GENVBO
	int ptype = index ? 1 : 0;
	if (dc->valid() && (!modified))
	{
		// Cache is valid and unmodified, return as-is
		offset=dc->offset;
		return dc->VBO; 
	}
	unsigned int FBOSize = IDXBUFSIZE;
	size_t size = elmSize * mult * count;
	if (size >= (FBOSize >> 2)) {
		//Big enough, just create/update
		if (dc->valid()) //We're updating, always recreate in case it is used
			dc->recreate();
		dc->VBO = allocateVBO(index, size, false, false);
		dc->size = 0;
		dc->offset = 0;
		return dc->VBO;
	}
	else {
		//Small buffer
		if (dc->valid())
			dc->recreate(); //Clear if already valid
		if (size == 0) { //Empty buffer, don't bind a VBO at all
			dc->size = 0;
			dc->offset = 0;
			return dc->VBO;
		}
		if (dx11ShaderBufferCache::openVboPack[ptype]) {
			if ((dx11ShaderBufferCache::vboPacks[dx11ShaderBufferCache::openVboPack[ptype]].used + size) > FBOSize) {
				//Open pack is full, close it
				dx11ShaderBufferCache::openVboPack[ptype] = 0;
			}
		}
		if (!dx11ShaderBufferCache::openVboPack[ptype]) {
			//No open pack, create one
			dx11ShaderBufferCache::openVboPack[ptype] = allocateVBO(index, FBOSize, false, true);
			dx11ShaderBufferCache::VboPack pack;
			pack.index = index;
			pack.used = 0;
			pack.freeed = 0;
			dx11ShaderBufferCache::vboPacks[dx11ShaderBufferCache::openVboPack[ptype]] = pack;
		}
		//Here we have an open pack ready to use
		auto& pack = dx11ShaderBufferCache::vboPacks[dx11ShaderBufferCache::openVboPack[ptype]];
		dc->size = size;
		dc->offset = pack.used;
		dc->VBO = dx11ShaderBufferCache::openVboPack[ptype];
		offset = pack.used;
		pack.used += ((size + 3) & (~3)); //Align
		return dc->VBO;
	}
#else
	if ((dc->VBO == NULL) || (dc->VBOcapacity < count) || (dc->cachedMult != mult)) {
		if (dc->VBO != NULL)
			dc->VBO->Release();
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;    // write access access by CPU and GPU
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU to write in buffer
		bd.ByteWidth = elmSize * mult * count;
		bd.BindFlags =
			(!index)  ?
		D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER; // use as a vertex buffer
		g_dev->CreateBuffer(&bd, NULL, &(dc->VBO));
		dc->VBOcapacity = count;
		dc->cachedMult = mult;
		modified = true;
	}
#endif
	return dc->VBO;
}

ID3D11Buffer *dx11ShaderProgram::getGenericVBO(int index, int elmSize, int mult,
		int count, size_t &offset) {
#ifdef DX11SHADERS_COMMON_GENVBO
	int bt = index?0:1;
	size_t size = elmSize* mult* count;
	size_t psize = ((size + 3) & (~3));
	ID3D11Buffer* vbo = nullptr;
	if (psize > (IDXBUFSIZE / 2))
	{
		vbo = allocateVBO(!index, (psize<IDXBUFSIZE)?IDXBUFSIZE:psize, true, false);
		offset = 0;
		genBufferOffset[bt] = IDXBUFSIZE;
	}
	else if ((curGenVBO[bt] == nullptr) || ((genBufferOffset[bt] + psize) > IDXBUFSIZE))
	{
		vbo = curGenVBO[bt] = allocateVBO(!index,IDXBUFSIZE,true,true);
		offset = 0;
		genBufferOffset[bt] = 0;
	}
	else {
		offset = genBufferOffset[bt];
		vbo = curGenVBO[bt];
	}
	genBufferOffset[bt] += psize;
	return vbo;
#else
	offset = 0;
	if ((genVBO[index] == NULL) || (genVBOcapacity[index] < count)) {
		if (genVBO[index] != NULL)
			genVBO[index]->Release();
		if (count < 16)
			count = 16;
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;    // write access access by CPU and GPU
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU to write in buffer
		bd.ByteWidth = elmSize * mult * count;
		bd.BindFlags =
				(index > 0) ?
						D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER; // use as a vertex buffer
		g_dev->CreateBuffer(&bd, NULL, &genVBO[index]);
		genVBOcapacity[index] = count;
	}
	return genVBO[index];
#endif
}

static void copyEnlarge(const void *src, void *dst, int smult, int dmult, int esize, int count, int dup)
{
	switch (esize)
	{
	case 1:
	{
		uint8 *psrc = (uint8 *)src;
		uint8 *pdst = (uint8 *)dst;
		while (count--)
		{
			for (int kd = 0; kd < dup; kd++)
			{
				for (int kc = 0; kc < smult; kc++)
					*(pdst++) = psrc[kc];
				for (int kc = smult; kc < dmult; kc++)
					*(pdst++) = 0;
			}
			psrc += smult;
		}
	}
		break;
	case 2:
	{
		uint16 *psrc = (uint16 *)src;
		uint16 *pdst = (uint16 *)dst;
		while (count--)
		{
			for (int kd = 0; kd < dup; kd++)
			{
				for (int kc = 0; kc < smult; kc++)
					*(pdst++) = psrc[kc];
				for (int kc = smult; kc < dmult; kc++)
					*(pdst++) = 0;
			}
			psrc += smult;
		}
	}
		break;
	case 4:
	{
		uint32 *psrc = (uint32 *)src;
		uint32 *pdst = (uint32 *)dst;
		while (count--)
		{
			for (int kd = 0; kd < dup; kd++)
			{
				for (int kc = 0; kc < smult; kc++)
					*(pdst++) = psrc[kc];
				for (int kc = smult; kc < dmult; kc++)
					*(pdst++) = 0;
			}
			psrc += smult;
		}
	}
		break;
	}
}

void dx11ShaderProgram::setupBuffer(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache,int stride,int offset) {
	if (index >= attributes.size())
		return;
	bool normalize = false; //TODO
	int elmSize = 1;
	switch (type) {
	case DFLOAT:
		elmSize = 4;
		break;
	case DUSHORT:
	case DSHORT:
		elmSize = 2;
		break;
	case DINT:
		elmSize = 4;
		break;
	}
	const DataDesc &dd = attributes[index];
	if (!dd.mult) return;
	unsigned int bcount = (flags&ShaderProgram::Flag_PointShader) ? count * 4 : count;
	size_t boffset=0;
	ID3D11Buffer* vbo = cache ? getCachedVBO(cache, false, elmSize, dd.mult, bcount, modified, boffset) : nullptr;
	if (vbo == nullptr) vbo = getGenericVBO(index + 1, elmSize, dd.mult, bcount, boffset);
	if (!vbo) return;
	if (modified || (!cache))
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		HRESULT hr = g_devcon->Map(vbo, NULL, boffset?D3D11_MAP_WRITE_NO_OVERWRITE:D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		if (flags&ShaderProgram::Flag_PointShader)
		{
			copyEnlarge(ptr, ((char *)ms.pData)+boffset, mult, dd.mult, elmSize, count, 4);		
			if (index == ShaderProgram::DataVertex)
			{
				int gindex = ShaderProgram::DataTexture + 1;
				//Ensure TexCoord array is present and big enough
#ifdef DX11SHADERS_COMMON_GENVBO
				bool changed = true;
#else
				bool changed = (genVBO[gindex] == NULL) || (genVBOcapacity[gindex] < (count * 4));
#endif
				if (changed) {
					D3D11_MAPPED_SUBRESOURCE tms;
					size_t toffset = 0;
					ID3D11Buffer *tvbo = getGenericVBO(gindex, 4, 2, count*4,toffset);
					g_devcon->Map(tvbo, NULL, toffset ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD, NULL, &tms); // map the buffer
					float *t = (float *)(((char *)tms.pData)+toffset);
					for (int k = 0; k < count; k++) {
						*(t++) = 0.0;
						*(t++) = 0.0;
						*(t++) = 1.0;
						*(t++) = 0.0;
						*(t++) = 1.0;
						*(t++) = 1.0;
						*(t++) = 0.0;
						*(t++) = 1.0;
					}
					g_devcon->Unmap(tvbo, NULL);                          // unmap the buffer
#ifdef DX11SHADERS_COMMON_GENVBO
					UINT tstride = 8;
					UINT voff = toffset;
					g_devcon->IASetVertexBuffers(gindex-1, 1, &tvbo, &tstride, &voff);
#endif
				}
			}
		}
		else
		{
			if ((mult == 2) && (dd.mult == 3) && (type == DFLOAT)) //TODO should be more generic
			{
				float *vdi = (float *)ptr;
				float* vdo = (float*)(((char*)ms.pData) + boffset);
				for (int k = 0; k < count; k++) {
					*(vdo++) = *(vdi++);
					*(vdo++) = *(vdi++);
					*(vdo++) = 0;
				}
				mult = dd.mult;
			}
			else
				memcpy((((char*)ms.pData) + boffset), ptr, mult * elmSize * count);          // copy the data
		}
		//floatdump(vName, ms.pData, 8);
		g_devcon->Unmap(vbo, NULL);                              // unmap the buffer
	}
	UINT tstride = dd.mult * elmSize;
	if (stride)
		tstride=stride;

	UINT voff = boffset+offset;
	g_devcon->IASetVertexBuffers(index, 1, &vbo, &tstride, &voff);

#ifndef DX11SHADERS_COMMON_GENVBO
	if (flags&ShaderProgram::Flag_PointShader)
	{
		if (index == ShaderProgram::DataVertex)
		{
			UINT tstride = 8;
			UINT voff = 0;
			int gindex = ShaderProgram::DataTexture;
			size_t boffset = 0;
			ID3D11Buffer *tvbo = getGenericVBO(gindex+1, 4, 2, count * 4, boffset);
			g_devcon->IASetVertexBuffers(gindex, 1, &tvbo, &tstride, &voff);
		}
	}
#endif
}

void dx11ShaderProgram::setData(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache, int stride, int offset,int bufferingFlags) {
	activate();
	setupBuffer(index, type, mult, ptr, count, modified, cache,stride,offset);
}

void dx11ShaderProgram::setConstant(int index, ConstantType type, int mult,
		const void *ptr) {
	if (updateConstant(index, type, mult, ptr)) {
		if (uniforms[index].vertexShader)
			cbvMod = true;
		else
			cbpMod = true;
	}
}

dx11ShaderProgram::dx11ShaderProgram(const void *vshader, int vshadersz,
		const void *pshader, int pshadersz, int flags, const ConstantDesc *uniforms,
		const DataDesc *attributes) {
	buildShaderProgram(vshader, vshadersz, pshader, pshadersz, flags, uniforms,
			attributes);
}

dx11ShaderProgram::dx11ShaderProgram(const char *vshader, const char *pshader,int flags,
		const ConstantDesc *uniforms, const DataDesc *attributes) {
	bool fromCode=(flags&ShaderProgram::Flag_FromCode);
	long VSLen, PSLen;
	void *VSFile = fromCode?NULL:LoadShaderFile(vshader, "cso", &VSLen);
	if (!VSFile) {
		void *src = fromCode?(void *)vshader:LoadShaderFile(vshader, "hlsl", &VSLen);
		ID3DBlob *pCode;
		ID3DBlob *pError;
		if (fromCode&&vshader)
			VSLen = strlen(vshader);
		D3DCompile(src, VSLen, vshader, NULL, NULL, "VShader",
			"vs_4_0", D3DCOMPILE_PREFER_FLOW_CONTROL, 0, &pCode, &pError);
		if (src&&(!fromCode))
			free(src);
		if (pError) {
			errorLog.append("VertexShader:\n");
			errorLog.append((char *) pError->GetBufferPointer(),
					pError->GetBufferSize());
			errorLog.append("\n");
			pError->Release();
		}
		if (pCode) {
			VSLen = pCode->GetBufferSize();
			VSFile = malloc(VSLen);
			memcpy(VSFile, pCode->GetBufferPointer(), VSLen);
			pCode->Release();
		}
	}
	void *PSFile = fromCode?NULL:LoadShaderFile(pshader, "cso", &PSLen);
	if (!PSFile) {
		void *src = fromCode?(void *)pshader:LoadShaderFile(pshader, "hlsl", &PSLen);
		ID3DBlob *pCode;
		ID3DBlob *pError;
		if (fromCode&&pshader)
			PSLen = strlen(pshader);
		D3DCompile(src, PSLen, pshader, NULL, NULL, "PShader",
			"ps_4_0", D3DCOMPILE_PREFER_FLOW_CONTROL, 0, &pCode, &pError);
		if (src&&(!fromCode))
			free(src);
		if (pError) {
			errorLog.append("PixelShader:\n");
			errorLog.append((char *) pError->GetBufferPointer(),
					pError->GetBufferSize());
			errorLog.append("\n");
			pError->Release();
		}
		if (pCode) {
			PSLen = pCode->GetBufferSize();
			PSFile = malloc(PSLen);
			memcpy(PSFile, pCode->GetBufferPointer(), PSLen);
			pCode->Release();
		}
	}
	buildShaderProgram(VSFile, VSLen, PSFile, PSLen, flags, uniforms, attributes);
	if (VSFile)
		free(VSFile);
	if (PSFile)
		free(PSFile);
}

void dx11ShaderProgram::buildShaderProgram(const void *vshader, int vshadersz,
		const void *pshader, int pshadersz, int flags, const ConstantDesc *uniforms,
		const DataDesc *attributes) {
	g_pVS = NULL;
	g_pPS = NULL;
	g_pLayout = NULL;
	g_CBV = NULL;
	g_CBP = NULL;
	cbpData = NULL;
	cbvData = NULL;

#ifdef DX11SHADERS_COMMON_GENVBO
#else
	for (int k = 0; k < 17; k++) {
		genVBO[k] = NULL;
		genVBOcapacity[k] = 0;
	}
#endif
	cbvsData = 0;
	cbpsData = 0;
	this->flags = flags;
	if (!(vshader && pshader))
		return;
	g_dev->CreateVertexShader(vshader, vshadersz, NULL, &g_pVS);
	g_dev->CreatePixelShader(pshader, pshadersz, NULL, &g_pPS);

	g_devcon->VSSetShader(g_pVS, 0, 0);
	g_devcon->PSSetShader(g_pPS, 0, 0);

	int vtex = 0;
	int ftex = 0;
	int tslot;
	while (!uniforms->name.empty()) {
		int usz = 0, ual = 4;
		ConstantDesc cd;
		cd = *(uniforms++);
		switch (cd.type) {
		case CTEXTURE:
			tslot = ftex;
			if (cd.vertexShader)
				tslot = (vtex++) | 0x100;
			else
				ftex++;
			textureMap.push_back(tslot);
			break;
		case CINT:
			usz = 4;
			ual = 4;
			break;
		case CFLOAT:
			usz = 4;
			ual = 4;
			break;
		case CFLOAT2:
			usz = 8;
			ual = 4;
			break;
		case CFLOAT3:
			usz = 12;
			ual = 4;
			break;
		case CFLOAT4:
			usz = 16;
			ual = 16;
			break;
		case CMATRIX:
			usz = 64;
			ual = 16;
			break;
		}
		if (cd.mult>1)
		{
			usz*=cd.mult;
			ual=16;
		}
		if (usz)
		{
			int *cbData = cd.vertexShader ? &cbvsData : &cbpsData;
			if ((*cbData) & (ual - 1))
				(*cbData) += ual - ((*cbData) & (ual - 1));
			int rem = (16 - ((*cbData) & 15));
			if ((usz>rem) && (rem!=16))
				(*cbData) += rem;
			cd.offset = (*cbData);
			(*cbData) += usz;
		}
		this->uniforms.push_back(cd);
	}
	if (cbpsData & 15)
		cbpsData += 16 - (cbpsData & 15);
	if (cbvsData & 15)
		cbvsData += 16 - (cbvsData & 15);
	cbpData = malloc(cbpsData);
	cbvData = malloc(cbvsData);
	cbpMod = true;
	cbvMod = true;

	for (int iu = 0; iu < this->uniforms.size(); iu++) {
		char *b = (char *) (this->uniforms[iu].vertexShader ? cbvData : cbpData);
		this->uniforms[iu]._localPtr = b + this->uniforms[iu].offset;
	}

	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DYNAMIC;
	bd2.ByteWidth = cbpsData;
	bd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (cbpsData) {
		g_dev->CreateBuffer(&bd2, NULL, &g_CBP);
		g_devcon->PSSetConstantBuffers(1, 1, &g_CBP);
	}
	else g_CBP = nullptr;

	bd2.ByteWidth = cbvsData;
	if (cbvsData) {
		g_dev->CreateBuffer(&bd2, NULL, &g_CBV);
		g_devcon->VSSetConstantBuffers(0, 1, &g_CBV);
	}
	else g_CBV = nullptr;

	D3D11_INPUT_ELEMENT_DESC ied[16]; //Would someone really need more than 16 vertexdata flows ?
	int nie = 0;
	while (!attributes->name.empty()) {
		ied[nie].SemanticName = attributes->name.c_str();
		ied[nie].SemanticIndex = 0;
		switch (attributes->type) {
		case DataType::DFLOAT:
			switch (attributes->mult) {
			case 1:
				ied[nie].Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case 2:
				ied[nie].Format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case 3:
				ied[nie].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case 4:
				ied[nie].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				ied[nie].Format = DXGI_FORMAT_UNKNOWN;
				break;
			}
			break;
		case DataType::DUBYTE:
			switch (attributes->mult) {
			case 1:
				ied[nie].Format = DXGI_FORMAT_R8_UNORM;
				break;
			case 2:
				ied[nie].Format = DXGI_FORMAT_R8G8_UNORM;
				break;
			case 4:
				ied[nie].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			default:
				ied[nie].Format = DXGI_FORMAT_UNKNOWN;
				break;
			}
			break;
		default: //Unsupported XXX
			ied[nie].Format = DXGI_FORMAT_UNKNOWN;
		}
		ied[nie].InputSlot = attributes->slot;
		ied[nie].AlignedByteOffset = attributes->offset;
		ied[nie].InputSlotClass = attributes->instances?D3D11_INPUT_PER_INSTANCE_DATA:D3D11_INPUT_PER_VERTEX_DATA;
		ied[nie].InstanceDataStepRate = attributes->instances;
		if (ied[nie].Format != DXGI_FORMAT_UNKNOWN)
			nie++;
		this->attributes.push_back(*(attributes++));
	}
	g_dev->CreateInputLayout(ied, nie, vshader, vshadersz, &g_pLayout);
	shaderInitialized();
}

dx11ShaderProgram::~dx11ShaderProgram() {
	if (g_pLayout)
		g_pLayout->Release();
	if (g_pVS)
		g_pVS->Release();
	if (g_pPS)
		g_pPS->Release();
#ifdef DX11SHADERS_COMMON_GENVBO
#else
	for (int k = 0; k < 17; k++)
		if (genVBO[k])
			genVBO[k]->Release();
#endif
	if (cbpData)
		free(cbpData);
	if (cbvData)
		free(cbvData);
	if (g_CBP)
		g_CBP->Release();
	if (g_CBV)
		g_CBV->Release();
}

void dx11ShaderProgram::updateConstants() {
	D3D11_MAPPED_SUBRESOURCE ms;
	//Update CB{V,P} data
	if (cbpMod && g_CBP) {
		//floatdump("CBP", &cbpData, 6);
		g_devcon->Map(g_CBP, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		memcpy(ms.pData, cbpData, cbpsData);                 // copy the data
		g_devcon->Unmap(g_CBP, NULL);                        // unmap the buffer
		cbpMod = false;
	}
	if (cbvMod && g_CBV) {
		g_devcon->Map(g_CBV, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		memcpy(ms.pData, cbvData, cbvsData);                 // copy the data
		g_devcon->Unmap(g_CBV, NULL);                        // unmap the buffer
		cbvMod = false;
	}
}
void dx11ShaderProgram::drawArrays(ShapeType shape, int first,
		unsigned int count,unsigned int instances) {
	ShaderEngine::Engine->prepareDraw(this);
	activate();
	updateConstants();

	if (shape == Point)
	{
		if (flags&ShaderProgram::Flag_PointShader)
		{
			D3D11_MAPPED_SUBRESOURCE ms;
			int ntris = count;
#ifdef DX11SHADERS_COMMON_GENVBO
			bool changed = true; //TODO
#else
			bool changed = ((genVBO[0] == NULL) || (genVBOcapacity[0] < (6 * ntris))); //We should really check if first is still the same here, but current gideros code uses 0 for Point Shaders
#endif
			size_t boffset = 0;
			ID3D11Buffer *vbo = getGenericVBO(0, 2, 1, 6 * ntris, boffset);
			if (changed)
			{
				g_devcon->Map(vbo, NULL, boffset ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
				unsigned short *i = (unsigned short *)(((char *)ms.pData)+boffset);

				for (int t = 0; t < ntris; t++) {
					unsigned short b = (first + t) * 4;
					*(i++) = b + 0;
					*(i++) = b + 1;
					*(i++) = b + 2;
					*(i++) = b + 0;
					*(i++) = b + 2;
					*(i++) = b + 3;
				}
				g_devcon->Unmap(vbo, NULL);                          // unmap the buffer
			}
			g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, boffset);
			curIndicesVBO = vbo;
			g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g_devcon->DrawIndexed(ntris * 6, 0, 0);
			return;
		}
		else
			g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	}
	else if (shape == TriangleStrip)
		g_devcon->IASetPrimitiveTopology(
				D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (shape == Triangles)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (shape == LineLoop) {
		D3D11_MAPPED_SUBRESOURCE ms;
		size_t boffset = 0;
		ID3D11Buffer *vbo = getGenericVBO(0, 2, 1, count + 1, boffset);
		g_devcon->Map(vbo, NULL, boffset ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		unsigned short *i = (unsigned short *) (((char *)ms.pData)+boffset);
		for (int t = 0; t < count; t++)
			*(i++) = first + t;
		*(i++) = first;
		g_devcon->Unmap(vbo, NULL);                          // unmap the buffer
		g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, boffset);
		curIndicesVBO = vbo;
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		g_devcon->DrawIndexed(count + 1, 0, 0);
		return;
	} else if (shape == Lines)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	else {
		glog_e("glDrawArrays unknown pattern\n");
		exit(1);
	}
	if (instances)
		g_devcon->DrawInstanced(count, instances, 0, 0);
	else
		g_devcon->Draw(count, 0);
}
void dx11ShaderProgram::drawElements(ShapeType shape, unsigned int count,
		DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first,unsigned int dcount,unsigned int instances,int bufferingFlags) {
	ShaderEngine::Engine->prepareDraw(this);
	activate();
	updateConstants();

	int indiceSize = 4;
	DXGI_FORMAT iFmt = DXGI_FORMAT_R32_UINT;

	if (type == DUSHORT) {
		indiceSize = 2;
		iFmt = DXGI_FORMAT_R16_UINT;
	}

	D3D11_MAPPED_SUBRESOURCE ms;
	size_t boffset = 0;
	ID3D11Buffer* vbo = cache ? getCachedVBO(cache, true, indiceSize, 1, count, modified, boffset) : nullptr;
	if (vbo==nullptr) vbo=getGenericVBO(0, indiceSize, 1, count, boffset);
	if (modified || (!cache))
	{
		g_devcon->Map(vbo, NULL, boffset ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		memcpy(((char *)ms.pData)+boffset, indices, indiceSize * count);              // copy the data
		g_devcon->Unmap(vbo, NULL);                              // unmap the buffer
	}

	UINT stride = indiceSize;
	UINT offset = 0;

	if ((curIndicesVBO != vbo) || (!cache))
	{
		g_devcon->IASetIndexBuffer(vbo, iFmt, boffset);
#ifndef DX11SHADERS_COMMON_GENVBO
		curIndicesVBO = vbo;
#endif
	}

	if (shape == Point)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	else if (shape == TriangleStrip)
		g_devcon->IASetPrimitiveTopology(
				D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (shape == Triangles)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (shape == LineLoop)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	else if (shape == Lines)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	else {
		glog_e("glDrawElements: bad mode\n");
		exit(1);
	}

	if (instances)
		g_devcon->DrawIndexedInstanced(dcount?dcount:count, instances, first, 0, 0);
	else
		g_devcon->DrawIndexed(dcount?dcount:count, first, 0);
}
