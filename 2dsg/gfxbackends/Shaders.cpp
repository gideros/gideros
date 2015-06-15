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

ShaderProgram *ShaderProgram::stdBasic=NULL;
ShaderProgram *ShaderProgram::stdColor=NULL;
ShaderProgram *ShaderProgram::stdTexture=NULL;
ShaderProgram *ShaderProgram::stdTextureColor=NULL;
ShaderEngine *ShaderEngine::Engine=NULL;

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

bool ShaderProgram::updateConstant(int index,ShaderProgram::ConstantType type,const void *ptr)
{
	char *b=(char *)(uniforms[index]._localPtr);
	int sl=0;
	switch (type)
	{
	case CINT:
	case CFLOAT:
		sl=4;
		break;
	case CFLOAT4:
		sl=16;
		break;
	case CMATRIX:
		sl=64;
		break;
	}
	if (!sl) return false;
	if (!memcmp(b,ptr,sl)) return false;
	memcpy(b,ptr,sl);
	return true;
}

void ShaderEngine::reset()
{
	setColor(1,1,1,1);
    oglProjection.identity();
    oglVPProjection.identity();
    oglModel.identity();
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


//Coordinates are untransformed coordinates (ie, cuurent Sprite local)
void ShaderEngine::pushClip(float x,float y,float w,float h)
{
	Vector4 v1(x,y,0,1);
	Vector4 v2(x+w,y+h,0,1);
	Matrix4 xform=oglVPProjection*oglModel;
	v1=xform*v1;
	v2=xform*v2;
	//glog_d("Scissor: [%f,%f->%f,%f] -> [%f,%f->%f,%f]",x,y,x+w,y+h,v1.x,v1.y,v2.x,v2.y);
	x=v1.x;
	w=v2.x-v1.x;
	if (w<0)
	{
		x=v2.x;
		w=-w;
	}
	y=v1.y;
	h=v2.y-v1.y;
	if (h<0)
	{
		y=v2.y;
		h=-h;
	}
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

int ShaderProgram::getSystemConstant(SystemConstant t)
{
	int n=(int)t;
	if (sysconstmask&(1<<n))
		return sysconstidx[n];
	return -1;
}

