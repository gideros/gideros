#include "color.h"
#include <stack>
#include "ogl.h"

struct Color
{
	Color() {}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	float r, g, b, a;
};

static std::stack<Color> colorStack;
static Color currentColor(1, 1, 1, 1);

static inline void setColor(float r, float g, float b, float a)
{
	if (!ShaderEngine::Engine) return;
#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
	ShaderEngine::Engine->setColor(r * a, g * a, b * a, a);
#else
	ShaderEngine::Engine->setColor(r, g, b, a);
#endif
}

void glPushColor()
{
	colorStack.push(currentColor);
}

void glPopColor()
{
	currentColor = colorStack.top();
	colorStack.pop();

    setColor(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
}

void glMultColor(float r, float g, float b, float a)
{
	currentColor.r *= r;
	currentColor.g *= g;
	currentColor.b *= b;
	currentColor.a *= a;

    setColor(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
}

void glSetColor(float r, float g, float b, float a)
{
	currentColor.r = r;
	currentColor.g = g;
	currentColor.b = b;
	currentColor.a = a;

    setColor(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
}

void glGetColor(float *r, float *g, float *b, float *a)
{
    *r = currentColor.r;
    *g = currentColor.g;
    *b = currentColor.b;
    *a = currentColor.a;
}

int colorStackSize()
{
	return (int)colorStack.size();
}
