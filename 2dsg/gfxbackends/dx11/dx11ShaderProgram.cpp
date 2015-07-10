/*
 * gl2ShaderProgram.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "dx11Shaders.h"
#include "glog.h"
#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include "platform.h"
#include "pch.h"
#include <gstdio.h>
#include <io.h>
#include "D3Dcompiler.h"
using namespace Microsoft::WRL;

ShaderProgram *dx11ShaderProgram::current = NULL;

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

ID3D11Buffer *dx11ShaderProgram::getGenericVBO(int index, int elmSize, int mult,
		int count) {
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
}

void dx11ShaderProgram::setupBuffer(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache) {
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
	DataDesc dd = attributes[index];
	ID3D11Buffer *vbo = getGenericVBO(index + 1, elmSize, dd.mult, count);
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
	if ((mult == 2) && (dd.mult == 3) && (type == DFLOAT)) //TODO should be more generic
			{
		float *vdi = (float *) ptr;
		float *vdo = (float *) ms.pData;
		for (int k = 0; k < count; k++) {
			*(vdo++) = *(vdi++);
			*(vdo++) = *(vdi++);
			*(vdo++) = 0;
		}
		mult = dd.mult;
	} else
		memcpy(ms.pData, ptr, mult * elmSize * count);          // copy the data
	//floatdump(vName, ms.pData, 8);
	g_devcon->Unmap(vbo, NULL);                              // unmap the buffer

	UINT tstride = mult * elmSize;
	UINT offset = 0;

	g_devcon->IASetVertexBuffers(index, 1, &vbo, &tstride, &offset);
}

void dx11ShaderProgram::setData(int index, DataType type, int mult,
		const void *ptr, unsigned int count, bool modified,
		ShaderBufferCache **cache) {
	activate();
	setupBuffer(index, type, mult, ptr, count, modified, cache);
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
		const void *pshader, int pshadersz, const ConstantDesc *uniforms,
		const DataDesc *attributes) {
	buildShaderProgram(vshader, vshadersz, pshader, pshadersz, uniforms,
			attributes);
}

dx11ShaderProgram::dx11ShaderProgram(const char *vshader, const char *pshader,
		const ConstantDesc *uniforms, const DataDesc *attributes) {
	long VSLen, PSLen;
	void *VSFile = LoadShaderFile(vshader, "cso", &VSLen);
	if (!VSFile) {
		void *src = LoadShaderFile(vshader, "hlsl", &VSLen);
		ID3DBlob *pCode;
		ID3DBlob *pError;
		D3DCompile(src, VSLen, vshader, NULL, NULL, "VShader",
			"vs_4_0_level_9_3", D3DCOMPILE_PREFER_FLOW_CONTROL, 0, &pCode, &pError);
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
	void *PSFile = LoadShaderFile(pshader, "cso", &PSLen);
	if (!PSFile) {
		void *src = LoadShaderFile(pshader, "hlsl", &PSLen);
		ID3DBlob *pCode;
		ID3DBlob *pError;
		D3DCompile(src, PSLen, pshader, NULL, NULL, "PShader",
			"ps_4_0_level_9_3", D3DCOMPILE_PREFER_FLOW_CONTROL, 0, &pCode, &pError);
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
	buildShaderProgram(VSFile, VSLen, PSFile, PSLen, uniforms, attributes);
	if (VSFile)
		free(VSFile);
	if (PSFile)
		free(PSFile);
}

void dx11ShaderProgram::buildShaderProgram(const void *vshader, int vshadersz,
		const void *pshader, int pshadersz, const ConstantDesc *uniforms,
		const DataDesc *attributes) {
	g_pVS = NULL;
	g_pPS = NULL;
	g_pLayout = NULL;
	g_CBV = NULL;
	g_CBP = NULL;
	cbpData = NULL;
	cbvData = NULL;
	for (int k = 0; k < 17; k++) {
		genVBO[k] = NULL;
		genVBOcapacity[k] = 0;
	}
	cbvsData = 0;
	cbpsData = 0;
	if (!(vshader && pshader))
		return;
	g_dev->CreateVertexShader(vshader, vshadersz, NULL, &g_pVS);
	g_dev->CreatePixelShader(pshader, pshadersz, NULL, &g_pPS);

	g_devcon->VSSetShader(g_pVS, 0, 0);
	g_devcon->PSSetShader(g_pPS, 0, 0);

	while (uniforms->name) {
		int usz = 0, ual = 4;
		ConstantDesc cd;
		cd = *(uniforms++);
		switch (cd.type) {
		case CINT:
			usz = 4;
			ual = 4;
			break;
		case CFLOAT:
			usz = 4;
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
		if (cd.vertexShader) {
			if (cbvsData & (ual - 1))
				cbvsData += ual - (cbvsData & (ual - 1));
			cd.offset = cbvsData;
			cbvsData += usz*cd.mult;
		} else {
			if (cbpsData & (ual - 1))
				cbpsData += ual - (cbpsData & (ual - 1));
			cd.offset = cbpsData;
			cbpsData += usz*cd.mult;
		}
		this->uniforms.push_back(cd);
	}
	if (cbpsData & 15)
		cbpsData += 16 - (cbpsData & 15);
	if (cbvsData & 15)
		cbvsData += 16 - (cbvsData & 15);
	cbpData = malloc(cbpsData);
	cbvData = malloc(cbvsData);
	cbpMod = false;
	cbvMod = false;

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

	g_dev->CreateBuffer(&bd2, NULL, &g_CBP);
	g_devcon->PSSetConstantBuffers(1, 1, &g_CBP);

	bd2.ByteWidth = cbvsData;
	g_dev->CreateBuffer(&bd2, NULL, &g_CBV);
	g_devcon->VSSetConstantBuffers(0, 1, &g_CBV);

	D3D11_INPUT_ELEMENT_DESC ied[16]; //Would someone really need more than 16 vertexdata flows ?
	int nie = 0;
	while (attributes->name) {
		ied[nie].SemanticName = attributes->name;
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
		ied[nie].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		ied[nie].InstanceDataStepRate = 0;
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
	for (int k = 0; k < 17; k++)
		if (genVBO[k])
			genVBO[k]->Release();
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
	if (cbpMod) {
		//floatdump("CBP", &cbpData, 6);
		g_devcon->Map(g_CBP, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		memcpy(ms.pData, cbpData, cbpsData);                 // copy the data
		g_devcon->Unmap(g_CBP, NULL);                        // unmap the buffer
		cbpMod = false;
	}
	if (cbvMod) {
		g_devcon->Map(g_CBV, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		memcpy(ms.pData, cbvData, cbvsData);                 // copy the data
		g_devcon->Unmap(g_CBV, NULL);                        // unmap the buffer
		cbvMod = false;
	}
}
void dx11ShaderProgram::drawArrays(ShapeType shape, int first,
		unsigned int count) {
	ShaderEngine::Engine->prepareDraw(this);
	activate();
	updateConstants();

	if (shape == Point)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	else if (shape == TriangleStrip)
		g_devcon->IASetPrimitiveTopology(
				D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (shape == TriangleFan) {
		D3D11_MAPPED_SUBRESOURCE ms;
		int ntris = count - 2;
		ID3D11Buffer *vbo = getGenericVBO(0, 2, 1, 3 * ntris);
		g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		unsigned short *i = (unsigned short *) ms.pData;
		for (int t = 0; t < ntris; t++) {
			*(i++) = first;
			*(i++) = first + t + 1;
			*(i++) = first + t + 2;
		}
		g_devcon->Unmap(vbo, NULL);                          // unmap the buffer
		g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, 0);
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_devcon->DrawIndexed(ntris * 3, 0, 0);
		return;
	} else if (shape == Triangles)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (shape == LineLoop) {
		D3D11_MAPPED_SUBRESOURCE ms;
		ID3D11Buffer *vbo = getGenericVBO(0, 2, 1, count + 1);
		g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
		unsigned short *i = (unsigned short *) ms.pData;
		for (int t = 0; t < count; t++)
			*(i++) = first + t;
		*(i++) = first;
		g_devcon->Unmap(vbo, NULL);                          // unmap the buffer
		g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, 0);
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		g_devcon->DrawIndexed(count + 1, 0, 0);
		return;
	} else if (shape == Lines)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	else {
		glog_e("glDrawArrays unknown pattern\n");
		exit(1);
	}

	g_devcon->Draw(count, 0);
}
void dx11ShaderProgram::drawElements(ShapeType shape, unsigned int count,
		DataType type, const void *indices, bool modified, ShaderBufferCache **cache) {
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
	ID3D11Buffer *vbo = getGenericVBO(0, indiceSize, 1, count);
	g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
	memcpy(ms.pData, indices, indiceSize * count);              // copy the data
	g_devcon->Unmap(vbo, NULL);                              // unmap the buffer

	UINT stride = indiceSize;
	UINT offset = 0;

	g_devcon->IASetIndexBuffer(vbo, iFmt, 0);

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

	g_devcon->DrawIndexed(count, 0, 0);
}
