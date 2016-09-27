/*
 * Shaders.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */
#include "Shaders.h"
#include <stdlib.h>
#include <string.h>
#include <gstdio.h>
#include <algorithm>
#include "platform.h"

ShaderProgram *ShaderProgram::stdBasic=NULL;
ShaderProgram *ShaderProgram::stdColor=NULL;
ShaderProgram *ShaderProgram::stdTexture=NULL;
ShaderProgram *ShaderProgram::stdTextureColor=NULL;
ShaderProgram *ShaderProgram::stdParticle=NULL;
ShaderProgram *ShaderProgram::stdParticles=NULL;
ShaderProgram *ShaderProgram::pathShaderFillC=NULL;
ShaderProgram *ShaderProgram::pathShaderStrokeC = NULL;
ShaderProgram *ShaderProgram::pathShaderStrokeLC = NULL;
ShaderEngine *ShaderEngine::Engine=NULL;

void ShaderProgram::Retain()
{
	refCount++;
}

void ShaderProgram::Release()
{
	refCount--;
	if (refCount==0)
		delete this;
}

ShaderProgram::ShaderProgram()
{
	refCount=1;
}

void *ShaderProgram::LoadShaderFile(const char *fname, const char *ext, long *len) {
	char name[256];
	sprintf(name, "%s.%s", fname,ext);
	G_FILE *f = g_fopen(name, "r");
	if (f) {
		g_fseek(f, 0, SEEK_END);
		long sz = g_ftell(f);
		if (len)
			*len = sz;
		void *fdata = malloc(sz+1);
		((char *)fdata)[sz]=0; //NUL TERMINATE
		g_fseek(f, 0, SEEK_SET);
		g_fread(fdata, 1, sz, f);
		g_fclose(f);
		return fdata;
	}
	return NULL;
}

bool ShaderProgram::updateConstant(int index,ShaderProgram::ConstantType type, int mult,const void *ptr)
{
	char *b=(char *)(uniforms[index]._localPtr);
	int sl=0;
	switch (type)
	{
	case CINT:
	case CFLOAT:
		sl=4;
		break;
	case CFLOAT2:
		sl=8;
		break;
	case CFLOAT3:
		sl=12;
		break;
	case CFLOAT4:
		sl=16;
		break;
	case CMATRIX:
		sl=64;
		break;
	}
	if (!sl) return false;
	sl*=mult;
	if (!memcmp(b,ptr,sl)) return false;
	memcpy(b,ptr,sl);
	return true;
}

void ShaderEngine::reset(bool reinit)
{
	setColor(1,1,1,1);
    oglProjection.identity();
    oglVPProjection.identity();
    oglVPProjectionUncorrected.identity();
    oglModel.identity();
    oglCombined.identity();
    dsCurrent.dTest=false;
    dsCurrent.sRef=0;
    dsCurrent.sMask=0xFF;
    dsCurrent.sClear=false;
    dsCurrent.sFail=STENCIL_KEEP;
    dsCurrent.dFail=STENCIL_KEEP;
    dsCurrent.dPass=STENCIL_KEEP;
    dsCurrent.sFunc=STENCIL_DISABLE;
    while (!dsStack.empty())
    	dsStack.pop();
    setDepthStencil(dsCurrent);
}

void ShaderEngine::setViewportProjection(const Matrix4 vp, float width, float height)
{
	/*if (vp==oglVPProjectionUncorrected)
		return;*/
	oglVPProjectionUncorrected=vp;
	oglVPProjection=vp;
	adjustViewportProjection(oglVPProjection,width,height);
}

ShaderEngine::DepthStencil ShaderEngine::pushDepthStencil()
{
	dsStack.push(dsCurrent);
	return dsCurrent;
}

void ShaderEngine::popDepthStencil()
{
	if (!dsStack.empty())
	{
		dsCurrent=dsStack.top();
		dsStack.pop();
	    setDepthStencil(dsCurrent);
	}
}

void ShaderEngine::prepareDraw(ShaderProgram *program)
{
	int c=program->getSystemConstant(ShaderProgram::SysConst_WorldViewProjectionMatrix);
	if (c>=0)
		program->setConstant(c,ShaderProgram::CMATRIX,1,oglCombined.data());
	c=program->getSystemConstant(ShaderProgram::SysConst_Color);
	if (c>=0) {
		program->setConstant(c,ShaderProgram::CFLOAT4,1,constCol);
	}
	c=program->getSystemConstant(ShaderProgram::SysConst_WorldMatrix);
	if (c>=0) {
		program->setConstant(c,ShaderProgram::CMATRIX,1,oglModel.data());
	}
	c=program->getSystemConstant(ShaderProgram::SysConst_WorldInverseTransposeMatrix);
	if (c>=0) {
		Matrix4 m=oglModel.inverse().transpose();
		program->setConstant(c,ShaderProgram::CMATRIX,1,m.data());
	}
	c=program->getSystemConstant(ShaderProgram::SysConst_WorldInverseTransposeMatrix3);
	if (c>=0) {
		const float *om=oglModel.data();
		Matrix4 m3(om[0],om[1],om[2],0,
				om[4],om[5],om[6],0,
				om[8],om[9],om[10],0,
				0,0,0,1);
		Matrix4 m=m3.inverse().transpose();
		program->setConstant(c,ShaderProgram::CMATRIX,1,m.data());
	}
	c=program->getSystemConstant(ShaderProgram::SysConst_Timer);
	if (c>=0) {
		float clk=iclock();
		program->setConstant(c,ShaderProgram::CFLOAT,1,&clk);
	}
}

Matrix4 ShaderEngine::setFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
#ifdef DXCOMPAT_H
	int df = 1, dn = 0;
#else
 	int df = 1, dn = -1;
#endif
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

Matrix4 ShaderEngine::setOrthoFrustum(float l, float r, float b, float t, float n, float f)
{
	Matrix4 mat;
	mat[0] = 2 / (r - l);
	mat[5] = 2 / (t - b);
	mat[10] = -2 / (f - n);
	mat[12] = -(r + l) / (r - l);
	mat[13] = -(t + b) / (t - b);
	mat[14] = -(f + n) / (f - n);
	mat.type = Matrix4::M2D;
	return mat;
}


//Coordinates are untransformed coordinates (ie, current Sprite local)
void ShaderEngine::pushClip(float x,float y,float w,float h)
{
	Vector4 v1(x,y,0,1);
	Vector4 v2(x+w,y+h,0,1);
	Vector4 v3(x+w,y,0,1);
	Vector4 v4(x,y+h,0,1);
	Matrix4 xform=oglVPProjection*oglModel;
	v1=xform*v1;
	v2=xform*v2;
	v3=xform*v3;
	v4=xform*v4;
	x=(std::min)((std::min)(v1.x,v2.x),(std::min)(v3.x,v4.x));
	y=(std::min)((std::min)(v1.y,v2.y),(std::min)(v3.y,v4.y));
	float x2=(std::max)((std::max)(v1.x,v2.x),(std::max)(v3.x,v4.x));
	float y2=(std::max)((std::max)(v1.y,v2.y),(std::max)(v3.y,v4.y));
	w=x2-x;
	h=y2-y;
	if (scissorStack.empty())
	{
		Scissor s(x,y,w,h);
		scissorStack.push(s);
		setClip(s.x,s.y,s.w,s.h);
	}
	else
	{
		Scissor s(scissorStack.top(),x,y,w,h);
		scissorStack.push(s);
		setClip(s.x,s.y,s.w,s.h);
	}
}

void ShaderEngine::popClip()
{
	if (scissorStack.empty()) return; //Probably a code issue
	scissorStack.pop();
	if (scissorStack.empty())
	{
		setClip(-1,-1,-1,-1);
	}
	else
	{
		Scissor s=scissorStack.top();
		setClip(s.x,s.y,s.w,s.h);
	}
}

void ShaderEngine::setColor(float r,float g,float b,float a)
{
    constCol[0]=r;
    constCol[1]=g;
    constCol[2]=b;
    constCol[3]=a;
}

void ShaderEngine::setModel(const Matrix4 m)
{
    oglModel=m;
	oglCombined=oglProjection*oglModel;
}

void ShaderProgram::shaderInitialized()
{
	sysconstmask=0;
	for (int i=0;i<uniforms.size();i++)
		if (uniforms[i].sys)
		{
			int sn=uniforms[i].sys;
			sysconstmask|=(1<<sn);
			sysconstidx[sn]=i;
		}
}

int ShaderProgram::getConstantByName(const char *name)
{
	for (int i=0;i<uniforms.size();i++)
		if (!(strcmp(uniforms[i].name.c_str(),name)))
			return i;
	return -1;
}

int ShaderProgram::getSystemConstant(SystemConstant t)
{
	int n=(int)t;
	if (sysconstmask&(1<<n))
		return sysconstidx[n];
	return -1;
}

