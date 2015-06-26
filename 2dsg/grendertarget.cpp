#include "grendertarget.h"
#include "application.h"
#include "sprite.h"
#include "ogl.h"

GRenderTarget::GRenderTarget(Application *application, int width, int height, Filter filter) :
    TextureBase(application)
{
    TextureParameters parameters;
    parameters.filter = filter;
    data = application->getTextureManager()->createRenderTarget(width, height, parameters);

    sizescalex = 1;
    sizescaley = 1;
    uvscalex = (float)data->width / (float)data->baseWidth;
    uvscaley = (float)data->height / (float)data->baseHeight;
}

GRenderTarget::~GRenderTarget()
{
}

void GRenderTarget::clear(unsigned int color, float a)
{
	ShaderBuffer *fbo=gtexture_BindRenderTarget(gtexture_RenderTargetGetFBO(data->gid));
	ShaderEngine::Engine->setViewport(0, 0, data->width, data->height);

    float r = ((color >> 16) & 0xff) / 255.f;
    float g = ((color >> 8) & 0xff) / 255.f;
    float b = (color & 0xff) / 255.f;
	ShaderEngine::Engine->clearColor(r * a, g * a, b * a, a);

    gtexture_BindRenderTarget(fbo);
}

void GRenderTarget::draw(const Sprite *sprite)
{
    ShaderEngine::Engine->reset();
    ShaderBuffer *fbo=gtexture_RenderTargetGetFBO(data->gid);
	ShaderBuffer *oldfbo=gtexture_BindRenderTarget(fbo);

	fbo->prepareDraw();
	ShaderEngine::Engine->setViewport(0, 0, data->width, data->height);

	// The WINSTORE glOrtho (for Direct3D) is what you would expect. The OpenGL call needs to be inverted in y-direction
    Matrix4 projection;

#ifdef WINSTORE
	projection = ShaderEngine::Engine->setOrthoFrustum(0, data->baseWidth, data->baseHeight, 0, -1, 1);
#else
    projection = ShaderEngine::Engine->setOrthoFrustum(0, data->baseWidth, 0, data->baseHeight, -1, 1);
#endif

	ShaderEngine::Engine->setProjection(projection);

    CurrentTransform currentTransform;
    ((Sprite*)sprite)->draw(currentTransform, 0, 0, data->width, data->height);

    gtexture_BindRenderTarget(oldfbo);
}
