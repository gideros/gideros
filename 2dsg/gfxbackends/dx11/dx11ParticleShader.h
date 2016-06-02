/*
 * dx11ParticleShader.h
 *
 *  Created on: 19 juin 2015
 *      Author: Nicolas
 */

#ifndef DX11PARTICLESHADER_H_
#define DX11PARTICLESHADER_H_

#include "dx11Shaders.h"

class dx11ParticleShader: public dx11ShaderProgram {
protected:
	float psize;
public:
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache);
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache);
    dx11ParticleShader(const void *vshader,int vshadersz,const void *pshader,int pshadersz,int flags,
                     const ConstantDesc *uniforms, const DataDesc *attributes);
};

#endif /* DX11PARTICLESHADER_H_ */
