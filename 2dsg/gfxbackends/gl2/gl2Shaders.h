/*
 * gl2Shaders.h
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#ifndef GL2SHADERS_H_
#define GL2SHADERS_H_

#include <vector>
#include "Shaders.h"
#include "Matrices.h"
#include "gtexture.h"

#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifdef GIDEROS_GL1
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#else
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
#endif
#define OPENGL_ES
#elif __ANDROID__
#ifdef GIDEROS_GL1
    #include <GLES/gl.h>
    #include <GLES/glext.h>
#else
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif
#define OPENGL_ES
#elif WINSTORE
#include "dxcompat.hpp"
#define OPENGL_DESKTOP
#elif EMSCRIPTEN
#define	OPENGL_ES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#elif RASPBERRY_PI
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#define OPENGL_DESKTOP
#endif

#ifdef OPENGL_DESKTOP
#include "glcompat.h"
#endif

#ifdef OPENGL_ES
#ifdef GIDEROS_GL1
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
#define glGenFramebuffers glGenFramebuffersOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glBindFramebuffer glBindFramebufferOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#endif
#endif


class ogl2ShaderProgram : public ShaderProgram
{
	friend class ogl2ShaderEngine;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    std::vector<DataDesc> attributes;
    std::vector<GLint> glattributes;
    std::vector<GLint> gluniforms;
    std::string errorLog;
    std::string vshadercode;
    std::string fshadercode;
    unsigned long uninit_uniforms;
    void *cbData;
    int cbsData;
    static GLint curProg;
    static ShaderProgram *current;
    static std::vector<ogl2ShaderProgram *> shaders;
    void buildProgram(const char *vshader1,const char *vshader2,
                     const char *fshader1, const char *fshader2,
					 const ConstantDesc *uniforms, const DataDesc *attributes);
public:
    virtual void activate();
    virtual void deactivate();
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache,int stride=0,int offset=0);
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first=0,unsigned int dcount=0);
    virtual bool isValid();
    virtual const char *compilationLog();

    virtual void recreate();
    static void resetAll();

    ogl2ShaderProgram(const char *vshader1,const char *vshader2,
                     const char *fshader1, const char *fshader2,
					 const ConstantDesc *uniforms, const DataDesc *attributes);
    ogl2ShaderProgram(const char *vshader,const char *fshader,int flags,
					 const ConstantDesc *uniforms, const DataDesc *attributes);
    virtual ~ogl2ShaderProgram();
    void useProgram();
};


class ogl2ShaderTexture : public ShaderTexture
{
	friend class ogl2ShaderBuffer;
	friend class ogl2ShaderEngine;
protected:
	GLuint glid;
	int width,height;
	bool native;
public:
	ogl2ShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
	void setNative(void *externalTexture);
	void *getNative();
	virtual ~ogl2ShaderTexture();
};

class ogl2ShaderBuffer : public ShaderBuffer
{
	friend class ogl2ShaderEngine;
	static int qualcommFix_;
	g_id tempTexture_;
	GLuint textureId_;
protected:
	GLuint _depthRenderBuffer;
	GLuint glid;
	int width,height;
	static GLint bindBuffer(GLint n);
public:
	ogl2ShaderBuffer(ShaderTexture *texture);
	virtual ~ogl2ShaderBuffer();
	void readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data);
	void prepareDraw();
	void unbound();
	void needDepthStencil();
};

class ogl2ShaderEngine : public ShaderEngine
{
	ShaderBuffer *currentBuffer;
	GLuint _depthRenderBuffer;
	GLuint s_texture;
	bool s_depthEnable;
	bool s_depthBufferCleared;
	GLenum blendFactor2GLenum(BlendFactor blendFactor);
	int devWidth,devHeight;
public:
	ogl2ShaderEngine(int sw,int sh);
	virtual ~ogl2ShaderEngine();
	const char *getVersion();
	const char *getShaderLanguage() { return "glsl"; };
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
};


#endif /* GL2SHADERS_H_ */
