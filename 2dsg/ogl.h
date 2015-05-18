#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "glcommon.h"
#include "Matrices.h"

void oglBindTexture(GLenum target, GLuint texture);
void oglForceBindTexture(GLenum target, GLuint texture);
Matrix4 oglGetModelMatrix();
void oglLoadMatrixf(const Matrix4 m);
void oglSetProjection(const Matrix4 m);
void oglViewportProjection(const Matrix4 m);
void oglViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void oglEnable(GLenum cap);
void oglDisable(GLenum cap);
void oglInitialize(unsigned int sw,unsigned int sh);
void oglCleanup();
void oglReset();
void oglColor4f(float,float,float,float);
void oglPushScissor(float x,float y,float w,float h);
void oglPopScissor();

void resetBindTextureCount();
int getBindTextureCount();

int getVertexArrayCount();
int getTextureCoordArrayCount();

void resetClientStateCount();
int getClientStateCount();

void resetTexture2DStateCount();
int getTexture2DStateCount();

Matrix4 setOrthoFrustum(float l, float r, float b, float t, float n, float f);
Matrix4 setFrustum(float l, float r, float b, float t, float n, float f);

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
		CINT,CFLOAT,CFLOAT4,CMATRIX
	};
	enum ShapeType {
		Point,
		Lines,
		LineLoop,
		Triangles,
		TriangleFan,
		TriangleStrip
	};
	static ShaderProgram *stdBasic;
	static ShaderProgram *stdColor;
	static ShaderProgram *stdTexture;
	static ShaderProgram *stdTextureColor;
	enum StdData {
		DataVertex=0, DataColor=1, DataTexture=2
	};
	enum StdConstant {
		ConstantMatrix=0, ConstantColor=1
	};
    virtual void activate()=0;
    virtual void deactivate()=0;
    virtual void setData(int index,DataType type,int mult,const void *ptr,unsigned int count, bool modified, BufferCache **cache)=0;
    virtual void setConstant(int index,ConstantType type,const void *ptr)=0;
    virtual void drawArrays(ShapeType shape, int first, unsigned int count)=0;
    virtual void drawElements(ShapeType shape, unsigned int count, DataType type, const void *indices, bool modified, BufferCache *cache)=0;
    virtual ~ShaderProgram() { };

};

#endif

