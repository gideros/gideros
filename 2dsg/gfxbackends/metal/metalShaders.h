#ifndef METALSHADERS_H_
#define METALSHADERS_H_

#include <vector>
#include <map>
#include "Shaders.h"
#include "Matrices.h"
#include "gtexture.h"

#ifdef __APPLE__
   #include <TargetConditionals.h>
#endif

#import <Metal/Metal.h>
extern id<MTLDevice> metalDevice;
extern MTLRenderPassDescriptor *metalFramebuffer;

class metalShaderProgram : public ShaderProgram
{
	friend class metalShaderEngine;
    std::map<int,id<MTLRenderPipelineState>> mrps;
	MTLRenderPipelineDescriptor *mrpd;
    std::vector<DataDesc> attributes;
    std::string errorLog;
    void *cbData;
    int cbsData;
    bool uniformVmodified,uniformFmodified;
    static ShaderProgram *current;
    static std::vector<metalShaderProgram *> shaders;

     MTLBlendFactor blendFactor2metal(ShaderEngine::BlendFactor blendFactor);
    void setupStructures(const ConstantDesc *uniforms, const DataDesc *attributes,int attmap,int attstride);
public:
    static ShaderProgram *stdParticleT;
    static ShaderProgram *stdParticlesT;
    static ShaderProgram *stdParticles3T;
    static ShaderProgram *stdBasic3;
    static ShaderProgram *stdColor3;
    static ShaderProgram *stdTexture3;
    static ShaderProgram *stdTextureColor3;
	static int vboFreeze, vboUnfreeze;
    virtual void activate();
    virtual void deactivate();
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, ShaderBufferCache **cache,int stride=0,int offset=0);
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr);
    virtual void drawArrays(ShapeType shape, int first, unsigned int count,unsigned int instances=0);
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, ShaderBufferCache **cache,unsigned int first=0,unsigned int dcount=0,unsigned int instances=0);
    virtual bool isValid();
    virtual const char *compilationLog();

    virtual void recreate();
    virtual void resetUniforms();
    static void resetAll();
    static void resetAllUniforms();

    metalShaderProgram(const char *vprogram,const char *fprogram,
					 const ConstantDesc *uniforms, const DataDesc *attributes, int attmap,int attstride);
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
	Format format;
	int bpr;
	int width,height;
	Wrap wrap;
	Filtering filter;
	id<MTLTexture> mtex;
public:
	metalShaderTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering,bool forRT);
	void updateData(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering);
    void readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data);
	void setNative(void *externalTexture);
	void *getNative();
	virtual ~metalShaderTexture();
};

class metalShaderBuffer : public ShaderBuffer
{
	friend class metalShaderEngine;
protected:
	MTLRenderPassDescriptor *mrpd;
    metalShaderTexture *tex;
    id<MTLTexture> depth;
    id<MTLTexture> stencil;
    int clearReq;
    bool forDepth_;
public:
	metalShaderBuffer(ShaderTexture *texture,bool forDepth=false);
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
    id<MTLCommandQueue> mcq;
    MTLViewport vp_;
    int clipX,clipY,clipW,clipH;
protected:
    id<MTLSamplerState> tsNC,tsFC,tsNR,tsFR,tsDC;
    id<MTLCommandBuffer> mcb;
    id<MTLRenderCommandEncoder> mrce;
    int clearReq;
    void clear(int f);
public:
    static BlendFactor curSFactor;
    static BlendFactor curDFactor;
    id<MTLRenderCommandEncoder> encoder();
    MTLRenderPassDescriptor *pass();
    void closeEncoder();
    void present(id<MTLDrawable> drawable);
    void newFrame();
    metalShaderEngine(int sw,int sh);
	virtual ~metalShaderEngine();
	const char *getVersion();
	const char *getShaderLanguage() { return "msl"; };
	void reset(bool reinit=false);
	ShaderTexture::Packing getPreferredPackingForTextureFormat(ShaderTexture::Format format);
	ShaderTexture *createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering,bool forRT=false);
	ShaderBuffer *createRenderTarget(ShaderTexture *texture,bool forDepth=false);
	ShaderBuffer *setFramebuffer(ShaderBuffer *fbo);
    ShaderBuffer *getFramebuffer() { return currentBuffer; };
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
    ShaderProgram *getDefault(StandardProgram id,int variant=0);
};

#endif /* GL2SHADERS_H_ */
