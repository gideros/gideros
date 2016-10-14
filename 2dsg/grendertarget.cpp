#include "grendertarget.h"
#include "application.h"
#include "sprite.h"
#include "ogl.h"
#include <gimage.h>

GRenderTarget::GRenderTarget(Application *application, int width, int height, Filter filter, Wrap wrap) :
    TextureBase(application)
{
    TextureParameters parameters;
    parameters.filter = filter;
    parameters.wrap = wrap;
    data = application->getTextureManager()->createRenderTarget(width, height, parameters);

    sizescalex = 1;
    sizescaley = 1;
    uvscalex = (float)data->width / (float)data->baseWidth;
    uvscaley = (float)data->height / (float)data->baseHeight;
}

GRenderTarget::~GRenderTarget()
{
}

ShaderBuffer *GRenderTarget::prepareForDraw()
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

	return oldfbo;
}

void GRenderTarget::clear(unsigned int color, float a, int x, int y, int w, int h)
{
	ShaderBuffer *oldfbo=NULL;

	if ((w>=0)&&(h>=0))
	{
		oldfbo=prepareForDraw();
		ShaderEngine::Engine->pushClip(x,y,w,h);
	}
	else
	{
		oldfbo=gtexture_BindRenderTarget(gtexture_RenderTargetGetFBO(data->gid));
		ShaderEngine::Engine->setViewport(0, 0, data->width, data->height);
	}

    float r = ((color >> 16) & 0xff) / 255.f;
    float g = ((color >> 8) & 0xff) / 255.f;
    float b = (color & 0xff) / 255.f;
	ShaderEngine::Engine->clearColor(r * a, g * a, b * a, a);
	if ((w>=0)&&(h>=0))
		ShaderEngine::Engine->popClip();

    gtexture_BindRenderTarget(oldfbo);
}

void GRenderTarget::draw(const Sprite *sprite, const Matrix transform)
{
	ShaderBuffer *oldfbo=prepareForDraw();

    ((Sprite*)sprite)->draw(transform, 0, 0, data->width, data->height);

    gtexture_BindRenderTarget(oldfbo);
}

void GRenderTarget::getPixels(int x,int y,int w,int h,void *buffer)
{
    ShaderBuffer *fbo=gtexture_RenderTargetGetFBO(data->gid);
    fbo->readPixels(x,y,w,h, ShaderTexture::FMT_RGBA, ShaderTexture::PK_UBYTE, buffer);
}

int GRenderTarget::save(const char *filename,int x,int y,int w,int h)
{
	unsigned char *buffer=(unsigned char *) malloc(w*h*4);
	getPixels(x,y,w,h,buffer);
	int ret=gimage_saveImage(filename,w,h,buffer);
	free(buffer);
	return ret;
}
