#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "glcommon.h"

void oglBindTexture(GLenum target, GLuint texture);
void oglForceBindTexture(GLenum target, GLuint texture);
void oglDrawArrays(GLenum mode, GLint first, GLsizei count);
void oglDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void oglEnableClientState(GLenum array);
void oglDisableClientState(GLenum array);

void oglEnable(GLenum cap);
void oglDisable(GLenum cap);

void resetBindTextureCount();
int getBindTextureCount();

int getVertexArrayCount();
int getTextureCoordArrayCount();

void resetClientStateCount();
int getClientStateCount();

void resetTexture2DStateCount();
int getTexture2DStateCount();

void oglReset();

#endif

