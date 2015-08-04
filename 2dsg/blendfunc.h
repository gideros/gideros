#ifndef BLENDFUNC_H
#define BLENDFUNC_H

#include "ogl.h"

void glPushBlendFunc();
void glPopBlendFunc();
void glSetBlendFunc(ShaderEngine::BlendFactor sfactor, ShaderEngine::BlendFactor dfactor);

#endif
