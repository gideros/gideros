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
using namespace Microsoft::WRL;


ShaderProgram *dx11ShaderProgram::current=NULL;

void *LoadShaderFile(const char *fname,long *len){
	char name[256];
	sprintf(name, "%s.cso", fname);
	G_FILE *f=g_fopen(name, "r");
	if (f)
	{
		g_fseek(f, 0, SEEK_END);
		long sz = g_ftell(f);
		if (len) *len = sz;
		void *fdata = malloc(sz);
		g_fseek(f, 0, SEEK_SET);
		g_fread(fdata, 1, sz, f);
		g_fclose(f);
		return fdata;
	}
	sprintf(name, "Assets/%s.cso", fname);
	int fd=open(name, 0);
	if (fd >= 0) {
		lseek(fd, 0, SEEK_END);
		long sz = tell(fd);
		if (len) *len = sz;
		void *fdata = malloc(sz);
		lseek(fd, 0, SEEK_SET);
		read(fd,fdata, sz);
		close(fd);
		return fdata;
	}
	return NULL;
}

void dx11ShaderProgram::deactivate()
{
    current=NULL;
}

void dx11ShaderProgram::activate()
{
	if (current == this) return;
	if (current) current->deactivate();
    current=this;
	g_devcon->VSSetShader(g_pVS, 0, 0);
	g_devcon->PSSetShader(g_pPS, 0, 0);
	g_devcon->PSSetConstantBuffers(1, 1, &g_CBP);
	g_devcon->VSSetConstantBuffers(0, 1, &g_CBV);
	g_devcon->IASetInputLayout(g_pLayout);

}

ID3D11Buffer *dx11ShaderProgram::getGenericVBO(int index,int elmSize,int mult,int count)
{
 if ((genVBO[index]==NULL)||(genVBOcapacity[index]<count))
 {
	if (genVBO[index]!=NULL)
		 genVBO[index]->Release();
	if (count<16)
		count=16;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	bd.ByteWidth = elmSize*mult*count;
	bd.BindFlags = (index>0)?D3D11_BIND_VERTEX_BUFFER:D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
	g_dev->CreateBuffer(&bd, NULL, &genVBO[index]);
	genVBOcapacity[index]=count;
 }
 return genVBO[index];
}

void dx11ShaderProgram::setupBuffer(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache)
{
	bool normalize=false; //TODO
	int elmSize = 1;
	switch (type)
	{
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
	DataDesc dd=attributes[index];
	ID3D11Buffer *vbo=getGenericVBO(index+1,elmSize,dd.mult,count);
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr=g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	if ((mult==2)&&(dd.mult==3)&&(type==DFLOAT)) //TODO should be more generic
	{
		float *vdi = (float *)ptr;
		float *vdo = (float *)ms.pData;
		for (int k = 0; k < count; k++)
		{
			*(vdo++) = *(vdi++);
			*(vdo++) = *(vdi++);
			*(vdo++) = 0;
		}
		mult=dd.mult;
	}
	else
		memcpy(ms.pData, ptr, mult*elmSize*count);                 // copy the data
	//floatdump(vName, ms.pData, 8);
	g_devcon->Unmap(vbo, NULL);                                      // unmap the buffer

	UINT tstride = mult*elmSize;
	UINT offset = 0;

	g_devcon->IASetVertexBuffers(index, 1, &vbo, &tstride, &offset);
}

void dx11ShaderProgram::setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache)
{
	activate();
	setupBuffer(index,type,mult,ptr,count,modified,cache);
}

void dx11ShaderProgram::setConstant(int index,ConstantType type,const void *ptr)
{
	char *b=(char *)(uniforms[index].vertexShader?cbvData:cbpData);
	b+=uniforms[index].offset;
	switch (type)
	{
	case CINT:
	case CFLOAT:
		memcpy(b,ptr,4);
		break;
	case CFLOAT4:
		memcpy(b,ptr,16);
		break;
	case CMATRIX:
		memcpy(b,ptr,64);
		break;
	}
	if (uniforms[index].vertexShader)
		cbvMod=true;
	else
		cbpMod=true;
}

dx11ShaderProgram::dx11ShaderProgram(const char *vshader,const char *pshader,
                 const ConstantDesc *uniforms, const DataDesc *attributes)
{
	long VSLen, PSLen;
	void *VSFile = LoadShaderFile(vshader,&VSLen);
	void *PSFile = LoadShaderFile(pshader,&PSLen);

	g_dev->CreateVertexShader(VSFile, VSLen, NULL, &g_pVS);
	g_dev->CreatePixelShader(PSFile, PSLen, NULL, &g_pPS);

	g_devcon->VSSetShader(g_pVS, 0, 0);
	g_devcon->PSSetShader(g_pPS, 0, 0);

	for (int k=0;k<17;k++)
	{
		genVBO[k]=NULL;
		genVBOcapacity[k]=0;
	}

	cbvsData=0;
	cbpsData=0;
    while (uniforms->name)
    {
    	int usz=4,ual=4;
    	ConstantDesc cd;
    	cd=*(uniforms++);
    	switch (cd.type)
    	{
    	case CINT: usz=4; ual=4; break;
    	case CFLOAT: usz=4; ual=4; break;
    	case CFLOAT4: usz=16; ual=16;break;
    	case CMATRIX: usz=64; ual=16; break;
    	}
    	if (cd.vertexShader)
    	{
    		if (cbvsData&(ual-1))
    			cbvsData+=ual-(cbvsData&(ual-1));
    		cd.offset=cbvsData;
    		cbvsData+=usz;
    	}
    	else
    	{
    		if (cbpsData&(ual-1))
    			cbpsData+=ual-(cbpsData&(ual-1));
    		cd.offset=cbpsData;
    		cbpsData+=usz;
    	}
        this->uniforms.push_back(cd);
    }
    cbpData=malloc(cbpsData);
    cbvData=malloc(cbvsData);
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
    int nie=0;
    while (attributes->name)
    {
    	ied[nie].SemanticName=attributes->name;
    	ied[nie].SemanticIndex=0;
    	switch (attributes->type)
    	{
    		case DataType::DFLOAT:
    			switch (attributes->mult)
    			{
    			case 1: ied[nie].Format=DXGI_FORMAT_R32_FLOAT; break;
    			case 2: ied[nie].Format=DXGI_FORMAT_R32G32_FLOAT; break;
    			case 3: ied[nie].Format=DXGI_FORMAT_R32G32B32_FLOAT; break;
    			case 4: ied[nie].Format=DXGI_FORMAT_R32G32B32A32_FLOAT; break;
    			default:
        			ied[nie].Format=DXGI_FORMAT_UNKNOWN;
        			break;
    			}
    			break;
    		default: //Unsupported XXX
    			ied[nie].Format=DXGI_FORMAT_UNKNOWN;
    	}
    	  ied[nie].InputSlot=attributes->slot;
    	  ied[nie].AlignedByteOffset=attributes->offset;
    	  ied[nie].InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
    	  ied[nie].InstanceDataStepRate=0;
		  if (ied[nie].Format!=DXGI_FORMAT_UNKNOWN)
			nie++;
        this->attributes.push_back(*(attributes++));
    }
	g_dev->CreateInputLayout(ied, nie, VSFile, VSLen, &g_pLayout);
	free(VSFile);
	free(PSFile);
}

dx11ShaderProgram::~dx11ShaderProgram()
{
	g_pLayout->Release();
	g_pVS->Release();
	g_pPS->Release();
	for (int k=0;k<17;k++)
		if (genVBO[k]) genVBO[k]->Release();
	free(cbpData);
	free(cbvData);
	g_CBP->Release();
	g_CBV->Release();
}

void dx11ShaderProgram::updateConstants()
{
		D3D11_MAPPED_SUBRESOURCE ms;
		//Update CB{V,P} data
		if (cbpMod)
		{
			//floatdump("CBP", &cbpData, 6);
			g_devcon->Map(g_CBP, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
			memcpy(ms.pData, cbpData,cbpsData);                 // copy the data
			g_devcon->Unmap(g_CBP, NULL);                                      // unmap the buffer
			cbpMod = false;
		}
		if (cbvMod)
		{
			g_devcon->Map(g_CBV, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
			memcpy(ms.pData, cbvData, cbvsData);                 // copy the data
			g_devcon->Unmap(g_CBV, NULL);                                      // unmap the buffer
			cbvMod = false;
		}
}
void dx11ShaderProgram::drawArrays(ShapeType shape, int first, unsigned int count)
{
	((dx11ShaderEngine *)ShaderEngine::Engine)->preDraw(this);
    activate();
    updateConstants();

    if (shape==Point)
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    else if (shape==TriangleStrip)
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    else if (shape==TriangleFan)
    {
  	  D3D11_MAPPED_SUBRESOURCE ms;
  	  int ntris = count - 2;
  	  ID3D11Buffer *vbo=getGenericVBO(0,2,1,3*ntris);
  	  g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
  	  unsigned short *i = (unsigned short *)ms.pData;
  	  for (int t = 0; t < ntris; t++)
  	  {
  		  *(i++) = first;
  		  *(i++) = first+t+1;
  		  *(i++) = first+t+2;
  	  }
  	  g_devcon->Unmap(vbo, NULL);                                      // unmap the buffer
  	  g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, 0);
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  	  g_devcon->DrawIndexed(ntris * 3, 0, 0);
  	  return;
    }
    else if (shape==Triangles)
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    else if (shape==LineLoop)
    {
  	  D3D11_MAPPED_SUBRESOURCE ms;
  	  ID3D11Buffer *vbo=getGenericVBO(0,2,1,count+1);
  	  g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
  	  unsigned short *i = (unsigned short *)ms.pData;
  	  for (int t = 0; t < count; t++)
  		  *(i++) = first+t;
  	  *(i++) = first;
  	  g_devcon->Unmap(vbo, NULL);                                      // unmap the buffer
  	  g_devcon->IASetIndexBuffer(vbo, DXGI_FORMAT_R16_UINT, 0);
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  	  g_devcon->DrawIndexed(count+1, 0, 0);
  	  return;
    }
    else if (shape==Lines)
  	  g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    else {
  	  glog_e("glDrawArrays unknown pattern\n");
  	  exit(1);
    }

    g_devcon->Draw(count,0);
}
void dx11ShaderProgram::drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, BufferCache *cache)
{
	((dx11ShaderEngine *)ShaderEngine::Engine)->preDraw(this);
    activate();
    updateConstants();

	int indiceSize = 4;
	DXGI_FORMAT iFmt = DXGI_FORMAT_R32_UINT;

	if (type == DUSHORT)
	{
		indiceSize = 2;
		iFmt = DXGI_FORMAT_R16_UINT;
	}

	D3D11_MAPPED_SUBRESOURCE ms;
	ID3D11Buffer *vbo = getGenericVBO(0, indiceSize, 1, count);
	g_devcon->Map(vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, indices, indiceSize*count);                 // copy the data
	g_devcon->Unmap(vbo, NULL);                                      // unmap the buffer

	UINT stride = indiceSize;
	UINT offset = 0;

	g_devcon->IASetIndexBuffer(vbo, iFmt,0);

	if (shape==Point)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	else if (shape==TriangleStrip)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (shape==Triangles)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (shape==LineLoop)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	else if (shape==Lines)
		g_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	else {
		glog_e("glDrawElements: bad mode\n");
		exit(1);
	}


	g_devcon->DrawIndexed(count, 0, 0);
}
