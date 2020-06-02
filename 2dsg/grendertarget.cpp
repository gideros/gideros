#include "grendertarget.h"
#include "application.h"
#include "sprite.h"
#include "ogl.h"
#include "color.h"
#include <gimage.h>

GRenderTarget::GRenderTarget(Application *application, int width, int height, Filter filter, Wrap wrap, bool selectScale, bool depth) :
    TextureBase(application)
{
    TextureParameters parameters;
    parameters.filter = filter;
    parameters.wrap = wrap;
    data = application->getTextureManager()->createRenderTarget(width, height, parameters, selectScale, depth);

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

    Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0, data->baseWidth, data->baseHeight, 0, -1, 1,true);
	ShaderEngine::Engine->setProjection(projection);

	return oldfbo;
}

void GRenderTarget::clear(unsigned int color, float a, int x, int y, int w, int h)
{
	if (!ShaderEngine::isReady())
		return;
	ShaderBuffer *oldfbo=NULL;

    float r = ((color >> 16) & 0xff) / 255.f;
    float g = ((color >> 8) & 0xff) / 255.f;
    float b = (color & 0xff) / 255.f;
	if ((w>=0)&&(h>=0))
	{
		oldfbo=prepareForDraw();
		ShaderEngine::Engine->pushClip(x,y,w,h);
		glPushColor();
		glMultColor(r * a, g * a, b * a, a);
	    ShaderProgram *shp=ShaderProgram::stdBasic;
	    float vertices[8]={(float)x,(float)y,(float)(x+w-1),(float)y,(float)x,(float)(y+h-1),(float)(x+w-1),(float)(y+h-1)};
	    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,vertices,4,true,NULL);
		shp->drawArrays(ShaderProgram::TriangleStrip, 0,4);
		glPopColor();
		ShaderEngine::Engine->popClip();
	}
	else
	{
		oldfbo=gtexture_BindRenderTarget(gtexture_RenderTargetGetFBO(data->gid));
		ShaderEngine::Engine->setViewport(0, 0, data->width, data->height);
		ShaderEngine::Engine->clearColor(r * a, g * a, b * a, a);
	}

    gtexture_BindRenderTarget(oldfbo);
}

void GRenderTarget::draw(const Sprite *sprite, const Matrix transform)
{
	if (!ShaderEngine::isReady())
		return;
	ShaderBuffer *oldfbo=prepareForDraw();

    ((Sprite*)sprite)->draw(transform, 0, 0, data->width, data->height);

    gtexture_BindRenderTarget(oldfbo);
}

void GRenderTarget::getPixels(int x,int y,int w,int h,void *buffer)
{
	if (!ShaderEngine::isReady())
		return;
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
