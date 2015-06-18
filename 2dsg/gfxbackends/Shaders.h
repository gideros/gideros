#ifndef SHADERS_H_INCLUDED
#define SHADERS_H_INCLUDED

#include "Matrices.h"
#include <stack>
#include <vector>
class BufferCache
{
};

class ShaderProgram
{
public:
	enum DataType {
		DBYTE, DUBYTE, DSHORT, DUSHORT, DINT, DFLOAT
	};
	enum ConstantType {
		CINT,CFLOAT,CFLOAT4,CMATRIX,CTEXTURE
	};
	enum ShapeType {
		Point,
		Lines,
		LineLoop,
		Triangles,
		TriangleFan,
		TriangleStrip
	};
    struct DataDesc {
    	const char *name;
    	DataType type;
    	unsigned char mult;
    	unsigned char slot;
    	unsigned short offset;
    };
    enum SystemConstant {
    	SysConst_None=0,
    	SysConst_WorldViewProjectionMatrix,
    	SysConst_Color,
    	SysConst_WorldInverseTransposeMatrix,
    	SysConst_WorldMatrix,
		SysConst_ParticleSize,
		SysConst_TextureInfo
    };
    enum ShaderFlags {
    	Flag_None=0,
    	Flag_NoDefaultHeader=1
    };
    struct ConstantDesc {
    	const char *name;
    	ConstantType type;
    	int mult;
    	SystemConstant sys;
    	bool vertexShader;
    	unsigned short offset;
    	void *_localPtr;
    };
	static ShaderProgram *stdBasic;
	static ShaderProgram *stdColor;
	static ShaderProgram *stdTexture;
	static ShaderProgram *stdTextureColor;
	enum StdData {
		DataVertex=0, DataColor=1, DataTexture=2
	};
    virtual void activate()=0;
    virtual void deactivate()=0;
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache)=0;
    virtual void setConstant(int index,ConstantType type, int mult,const void *ptr)=0;
    virtual void drawArrays(ShapeType shape, int first, unsigned int count)=0;
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, BufferCache *cache)=0;
    virtual bool isValid()=0;
    virtual const char *compilationLog()=0;
    void Retain();
    void Release();
    ShaderProgram();
    virtual ~ShaderProgram() { };
    int getSystemConstant(SystemConstant t);
    int getConstantByName(const char *name);
protected:
    int refCount;
    std::vector<ConstantDesc> uniforms;
    int sysconstmask;
    char sysconstidx[8];
    void shaderInitialized();
    virtual bool updateConstant(int index,ConstantType type, int mult,const void *ptr);
    static void *LoadShaderFile(const char *fname, const char *ext, long *len);
};

class ShaderTexture
{
public:
	virtual ~ShaderTexture() { };
	enum Format {
		FMT_ALPHA,
		FMT_RGB,
		FMT_RGBA,
		FMT_Y,
		FMT_YA
	};
	enum Packing {
		PK_UBYTE,
		PK_USHORT_565,
		PK_USHORT_4444,
		PK_USHORT_5551
	};
	enum Wrap {
		WRAP_CLAMP,
		WRAP_REPEAT
	};
	enum Filtering {
		FILT_LINEAR,
		FILT_NEAREST
	};
};

class ShaderBuffer
{
public:
	virtual ~ShaderBuffer() { };
	virtual void prepareDraw()=0;
	virtual void readPixels(int x,int y,int width,int height,ShaderTexture::Format format,ShaderTexture::Packing packing,void *data)=0;
};

class ShaderEngine
{
protected:
	//CONSTANTS
	Matrix4 oglProjection;
	Matrix4 oglVPProjection;
	Matrix4 oglModel;
	Matrix4 oglCombined;
	float constCol[4];
	//Scissor structs
	struct Scissor
	{
		Scissor() {}
		Scissor(int x,int y,int w,int h) : x(x), y(y), w(w), h(h) {}
		Scissor(const Scissor &p,int nx,int ny,int nw,int nh) : x(nx), y(ny), w(nw), h(nh)
		{
			int x2=x+w-1;
			int y2=y+h-1;
			int px2=p.x+p.w-1;
			int py2=p.y+p.h-1;
			if (p.x>x)
				x=p.x;
			if (p.y>y)
				y=p.y;
			if (px2<x2)
				x2=px2;
			if (py2<y2)
				y2=py2;
			w=x2+1-x;
			h=y2+1-y;
		}

		int x,y,w,h;
	};
	std::stack<Scissor> scissorStack;
public:
	enum BlendFactor
	{
		NONE,
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		//CONSTANT_COLOR,
		//ONE_MINUS_CONSTANT_COLOR,
		//CONSTANT_ALPHA,
		//ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
	};
	static ShaderEngine *Engine;
	virtual ~ShaderEngine() { };
	virtual void reset();
	virtual const char *getVersion()=0;
	virtual ShaderTexture *createTexture(ShaderTexture::Format format,ShaderTexture::Packing packing,int width,int height,const void *data,ShaderTexture::Wrap wrap,ShaderTexture::Filtering filtering)=0;
	virtual ShaderBuffer *createRenderTarget(ShaderTexture *texture)=0;
	virtual ShaderBuffer *setFramebuffer(ShaderBuffer *fbo)=0;
	virtual ShaderProgram *createShaderProgram(const char *vshader,const char *pshader,int flags,
	                     const ShaderProgram::ConstantDesc *uniforms, const ShaderProgram::DataDesc *attributes)=0;
	virtual void setViewport(int x,int y,int width,int height)=0;
	//Matrices
	virtual Matrix4 setFrustum(float l, float r, float b, float t, float n, float f);
	virtual Matrix4 setOrthoFrustum(float l, float r, float b, float t, float n, float f);
	virtual void setProjection(const Matrix4 p) { oglProjection=p; }
	virtual void setViewportProjection(const Matrix4 vp) { oglVPProjection=vp; }
	virtual void setModel(const Matrix4 m);
	virtual const Matrix4 getModel() { return oglModel; }
	//Attributes
	virtual void setColor(float r,float g,float b,float a);
	virtual void clearColor(float r,float g,float b,float a)=0;
	virtual void bindTexture(int num,ShaderTexture *texture)=0;
	virtual void setDepthTest(bool enable)=0;
	virtual void setBlendFunc(BlendFactor sfactor, BlendFactor dfactor)=0;
	//Clipping
	virtual void pushClip(float x,float y,float w,float h);
	virtual void popClip();
	virtual void setClip(int x,int y,int w,int h)=0;
	//Internal
	virtual void prepareDraw(ShaderProgram *program);
};

#endif

