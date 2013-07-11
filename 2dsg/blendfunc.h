#ifndef BLENDFUNC_H
#define BLENDFUNC_H

#include "glcommon.h"

void glPushBlendFunc();
void glPopBlendFunc();
void glSetBlendFunc(GLenum sfactor, GLenum dfactor);

#endif
