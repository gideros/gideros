#include "blendfunc.h"
#include <stack>

struct BlendFunc
{
	BlendFunc() {}

	BlendFunc(GLenum sfactor, GLenum dfactor) : 
		sfactor(sfactor),
		dfactor(dfactor)
	{

	}

	GLenum sfactor;
	GLenum dfactor;
};

static std::stack<BlendFunc> blendFuncStack;

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
static BlendFunc currentBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
static BlendFunc currentBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

void glPushBlendFunc()
{
	blendFuncStack.push(currentBlendFunc);
}

void glPopBlendFunc()
{
	currentBlendFunc = blendFuncStack.top();
	blendFuncStack.pop();

	glBlendFunc(currentBlendFunc.sfactor, currentBlendFunc.dfactor);
}

void glSetBlendFunc(GLenum sfactor, GLenum dfactor)
{
	currentBlendFunc.sfactor = sfactor;
	currentBlendFunc.dfactor = dfactor;

	glBlendFunc(currentBlendFunc.sfactor, currentBlendFunc.dfactor);
}
