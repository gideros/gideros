/*
 * dx11ParticleShader.cpp
 *
 *  Created on: 19 juin 2015
 *      Author: Nicolas
 */

#include "dx11ParticleShader.h"
#include "dx11Shaders.h"
#include <stdlib.h>

void dx11ParticleShader::setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache)
{
	if (index==ShaderProgram::DataVertex)
	{
		float psize = this->psize;
		float *pdata=(float *)malloc(sizeof(float)*count*16);
		float *vdata = pdata;
		float *tdata=pdata+(count*8);
		float *sdata=(float *)ptr;
		for (int k=0;k<count;k++)
		{
			float x=*(sdata++);
			float y=*(sdata++);
			*(vdata++)=(x-psize);
			*(vdata++)=(y-psize);
			*(tdata++)=0;
			*(tdata++)=0;
			*(vdata++)=(x+psize);
			*(vdata++)=(y-psize);
			*(tdata++)=1;
			*(tdata++)=0;
			*(vdata++)=(x-psize);
			*(vdata++)=(y+psize);
			*(tdata++)=0;
			*(tdata++)=1;
			*(vdata++)=(x+psize);
			*(vdata++)=(y+psize);
			*(tdata++)=1;
			*(tdata++)=1;
		}
		vdata = pdata;
		tdata = pdata + (count * 8);
		count *= 4;
		dx11ShaderProgram::setData(index, type, 2, vdata, count, modified, cache);
		dx11ShaderProgram::setData(ShaderProgram::DataTexture, type, 2, tdata, count, true, NULL);
		free(pdata);
	}
	else
	{
		if (index==ShaderProgram::DataColor)
		{
			unsigned long *pdata = (unsigned long *)malloc(count * sizeof(unsigned long) * 4);
			unsigned long *vdata = pdata;
			unsigned long *sdata = (unsigned long *)ptr;
			for (int k=0;k<count;k++)
			{
				unsigned long c = *(sdata++);
				*(vdata++)=c;
				*(vdata++)=c;
				*(vdata++)=c;
				*(vdata++)=c;
			}
			vdata = pdata;
			count *= 4;
			dx11ShaderProgram::setData(index, type, mult, vdata, count, modified, cache);
			free(pdata);
		}
	}
}

void dx11ParticleShader::setConstant(int index,ConstantType type, int mult,const void *ptr)
{
	if (index==getSystemConstant(SysConst_ParticleSize))
		psize=(((float *)ptr)[0])/2; //Width->Radius
	else
		dx11ShaderProgram::setConstant(index,type,mult,ptr);
}

void dx11ParticleShader::drawArrays(ShapeType shape, int first, unsigned int count)
{
	if (shape != Point) return;
	shape=ShaderProgram::Triangles;

	unsigned long *pdata = (unsigned long *)malloc(count * sizeof(unsigned long) * 6);
	unsigned long *vdata = pdata;
	unsigned long c = first * 4;
	for (int k = 0; k<count; k++)
	{
		*(vdata++) = c;
		*(vdata++) = c + 1;
		*(vdata++) = c + 2;
		*(vdata++) = c + 1;
		*(vdata++) = c + 2;
		*(vdata++) = c + 3;
		c += 4;
	}
	vdata = pdata;
	count *= 6;
	dx11ShaderProgram::drawElements(shape, count, DINT, vdata,true,NULL);
	free(pdata);
}

void dx11ParticleShader::drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache)
{
	//Not supported
}

 dx11ParticleShader::dx11ParticleShader(const void *vshader,int vshadersz,const void *pshader,int pshadersz,int flags,
                     const ConstantDesc *uniforms, const DataDesc *attributes) : dx11ShaderProgram(vshader,vshadersz,pshader,pshadersz,flags,uniforms,attributes)
 {
	 psize=1;
 }

