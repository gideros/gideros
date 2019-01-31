/*
 * gl2Shaders.h
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#ifndef METALSHADERS_H_
#define METALSHADERS_H_

#include <vector>
#include "Shaders.h"
#include "Matrices.h"
#include "gtexture.h"

#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#ifdef TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif TARGET_OS_OSX
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif

class metalShaderProgram : public ShaderProgram
{
	friend class metalShaderEngine;
	MTLRenderPipelineState mrps;
	MTLRenderPipelineDescriptor mrpd;
    std::vector<DataDesc> attributes;
    std::string errorLog;
    void *cbData;
    int cbsData;
    MTLBuffer genVBO[16+1];
    static ShaderProgram *current;
    static std::vector<metalShaderProgram *> shaders;

    GLuint getGenericVBO(int index);
public:
	static int vboFreeze, vboUnfreeze;
    virtual void activate();
    virtual void deactivate();
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache,int stride=0,int offset=0);
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first=0,unsigned int dcount=0);
    virtual bool isValid();
    virtual const char *compilationLog();

    virtual void recreate();
    virtual void resetUniforms();
    static void resetAll();
    static void resetAllUniforms();

    metalShaderProgram(const char *vprogram,const char *fprogram,
					 const ConstantDesc *uniforms, const DataDesc *attributes);
    metalShaderProgram(const char *vshader,const char *fshader,int flags,
					 const ConstantDesc *uniforms, const DataDesc *attributes);
    virtual ~metalShaderProgram();
    void useProgram();
};


class metalShaderTexture : public ShaderTexture
{
	friend class metalShaderBuffer;
	friend class metalShaderEngine;
protected:
	MTLPixelFormat glformat;
	int bpr;
	int width,height;
	Wrap wrap;
	Filtering filter;
	MTLTexture mtex;
public:
	metalShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	void updateData(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	void setNative(void *externalTexture);
	void *getNative();
	virtual ~metalShaderTexture();
};

class metalShaderBuffer : public ShaderBuffer
{
	friend class metalShaderEngine;
protected:
	MTLRenderPassDescriptor *mrpd;
	int width,height;
public:
	metalShaderBuffer(ShaderTexture *texture);
	virtual ~metalShaderBuffer();
	void readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data);
	void prepareDraw();
	void unbound();
	void needDepthStencil();
};

class metalShaderEngine : public ShaderEngine
{
	ShaderBuffer *currentBuffer;
	int devWidth,devHeight;
protected:
	static MTLRenderCommandEncoder mrce;
	static BlendFactor curSFactor;
	static BlendFactor curDFactor;
public:
    metalShaderEngine(int sw,int sh);
	virtual ~metalShaderEngine();
	const char *getVersion();
	const char *getShaderLanguage() { return "msl"; };
	void reset(bool reinit=false);
	ShaderTexture *createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	ShaderBuffer *createRenderTarget(ShaderTexture *texture);
	ShaderBuffer *setFramebuffer(ShaderBuffer *fbo);
	ShaderProgram *createShaderProgram(const char *vshader,const char *pshader,int flags, const ShaderProgram::ConstantDesc *uniforms, const ShaderProgram::DataDesc *attributes);
	void setViewport(int x,int y,int width,int height);
	void adjustViewportProjection(Matrix4 &vp, float width, float height);
	void resizeFramebuffer(int width,int height);
	void setProjection(const Matrix4 p);
	void setModel(const Matrix4 m);
	void clearColor(float r,float g,float b,float a);
	void bindTexture(int num,ShaderTexture *texture);
	void setClip(int x,int y,int w,int h);
	void setBlendFunc(BlendFactor sfactor, BlendFactor dfactor);
	void setDepthStencil(DepthStencil state);
	void setVBOThreshold(int freeze,int unfreeze) { metalShaderProgram::vboFreeze=freeze; metalShaderProgram::vboUnfreeze=unfreeze; };
	void getProperties(std::map<std::string,std::string> &props);
};


#endif /* GL2SHADERS_H_ */
