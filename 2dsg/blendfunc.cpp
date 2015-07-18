#include "blendfunc.h"
#include <stack>

struct BlendFunc
{
	BlendFunc() {}

	BlendFunc(ShaderEngine::BlendFactor sfactor, ShaderEngine::BlendFactor dfactor) :
		sfactor(sfactor),
		dfactor(dfactor)
	{

	}

	ShaderEngine::BlendFactor sfactor;
	ShaderEngine::BlendFactor dfactor;
};

static std::stack<BlendFunc> blendFuncStack;

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
static BlendFunc currentBlendFunc(ShaderEngine::ONE, ShaderEngine::ONE_MINUS_SRC_ALPHA);
#else
static BlendFunc currentBlendFunc(ShaderEngine::SRC_ALPHA, ShaderEngine::ONE_MINUS_SRC_ALPHA);
#endif

void glPushBlendFunc()
{
	blendFuncStack.push(currentBlendFunc);
}

void glPopBlendFunc()
{
	currentBlendFunc = blendFuncStack.top();
	blendFuncStack.pop();

	ShaderEngine::Engine->setBlendFunc(currentBlendFunc.sfactor, currentBlendFunc.dfactor);
}

void glSetBlendFunc(ShaderEngine::BlendFactor sfactor, ShaderEngine::BlendFactor dfactor)
{
	currentBlendFunc.sfactor = sfactor;
	currentBlendFunc.dfactor = dfactor;

	ShaderEngine::Engine->setBlendFunc(currentBlendFunc.sfactor, currentBlendFunc.dfactor);
}
