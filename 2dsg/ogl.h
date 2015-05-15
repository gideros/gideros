#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "glcommon.h"
#include "Matrices.h"

enum OGLClientState {
	VertexArray,
	TextureArray,
	ColorArray
};

void oglBindTexture(GLenum target, GLuint texture);
void oglForceBindTexture(GLenum target, GLuint texture);
void oglDrawArrays(GLenum mode, GLint first, GLsizei count);
void oglDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, bool modified, GLuint *cache);
void oglEnableClientState(enum OGLClientState array);
void oglDisableClientState(enum OGLClientState array);
void oglArrayPointer(enum OGLClientState array,int mult,GLenum type,const void *ptr,GLsizei count, bool modified, GLuint *cache);
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


#endif

