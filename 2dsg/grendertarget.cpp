#include "grendertarget.h"
#include "application.h"
#include "sprite.h"
#include "ogl.h"
#include "color.h"
#include "blendfunc.h"
#include <gimage.h>

GRenderTarget::GRenderTarget(Application *application, int width, int height, TextureParameters &parameters, bool selectScale, bool depth) :
    TextureBase(application)
{
	depthRt_=depth;
	RENDER_DO([&]{
		data = application->getTextureManager()->createRenderTarget(width, height, parameters, selectScale, depth);

		sizescalex = 1;
		sizescaley = 1;
		uvscalex = (float)data->width / (float)data->baseWidth;
		uvscaley = (float)data->height / (float)data->baseHeight;
		gtexture_RenderTargetGetFBO(data->gid)->setScale(uvscalex,uvscaley);
	});
}

GRenderTarget::~GRenderTarget()
{
}

void GRenderTarget::prepareForDraw(SavedContext &ctx, bool inflow,bool capture)
{
	if (!inflow)
		ShaderEngine::Engine->reset();
	ShaderEngine::Engine->getViewport(ctx.ovx,ctx.ovy,ctx.ovw,ctx.ovh);
	if (capture)
		resize(ctx.ovw,ctx.ovh,1,1);

    ShaderBuffer *fbo=gtexture_RenderTargetGetFBO(data->gid);
    ctx.oldfbo=gtexture_BindRenderTarget(fbo);

    ShaderEngine::DepthStencil dsCurrent=ShaderEngine::Engine->pushDepthStencil();
    if (!capture) {
		dsCurrent.dTest=false;
		dsCurrent.dMask=true;
		dsCurrent.sRef=0;
		dsCurrent.sMask=0xFF;
		dsCurrent.sWMask=0xFF;
		dsCurrent.sClearValue=0;
		dsCurrent.sClear=false;
		dsCurrent.sFail=ShaderEngine::STENCIL_KEEP;
		dsCurrent.dFail=ShaderEngine::STENCIL_KEEP;
		dsCurrent.dPass=ShaderEngine::STENCIL_KEEP;
		dsCurrent.sFunc=ShaderEngine::STENCIL_DISABLE;
		dsCurrent.cullMode=ShaderEngine::CULL_NONE;
		ShaderEngine::Engine->setDepthStencil(dsCurrent);
    }

	fbo->prepareDraw();

    ctx.oldProj=ShaderEngine::Engine->getProjection();
    ctx.oldView=ShaderEngine::Engine->getView();
    ctx.oldModel=ShaderEngine::Engine->getModel();

	ShaderEngine::Engine->setViewport(0, 0, data->width, data->height);

	if (!capture)
	{
		Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0, data->baseWidth, data->baseHeight, 0, -1, 1,true);
		ShaderEngine::Engine->setProjection(projection);
	}
}

void GRenderTarget::finishDraw(SavedContext &ctx)
{
    ShaderEngine::Engine->setView(ctx.oldView);
    ShaderEngine::Engine->setProjection(ctx.oldProj);
    ShaderEngine::Engine->setViewport(ctx.ovx,ctx.ovy,ctx.ovw,ctx.ovh);
    ShaderEngine::Engine->setModel(ctx.oldModel);

    ShaderEngine::Engine->popDepthStencil();
    gtexture_BindRenderTarget(ctx.oldfbo);
}

void GRenderTarget::resize(int width, int height, float scaleX, float scaleY)
{
    if ((data->baseWidth==width)&&(data->baseHeight==height)) return;
	RENDER_DO([&]{
		TextureData *data2 = application_->getTextureManager()->createRenderTarget(width, height, data->parameters, false, depthRt_);
		application_->getTextureManager()->destroyTexture(data);
		data=data2;
		gtexture_RenderTargetGetFBO(data->gid)->setScale(scaleX,scaleY);
	});
}

void GRenderTarget::clearInt(unsigned int color, float a, int x, int y, int w, int h,bool inflow,bool capture)
{
	if (!ShaderEngine::isReady())
		return;

	SavedContext ctx;
	prepareForDraw(ctx,inflow,capture);

	float r = ((color >> 16) & 0xff) / 255.f;
	float g = ((color >> 8) & 0xff) / 255.f;
	float b = (color & 0xff) / 255.f;
	if ((w>=0)&&(h>=0))
	{
		Matrix modelMat;
		ShaderEngine::Engine->setModel(modelMat);
		glPushBlendFunc();
		glSetBlendFunc(ShaderEngine::ONE,ShaderEngine::ZERO);
		glPushColor();
		glMultColor(r * a, g * a, b * a, a);
		ShaderProgram *shp=ShaderProgram::stdBasic;
		float vertices[8]={(float)x,(float)y,(float)(x+w),(float)y,(float)x,(float)(y+h),(float)(x+w),(float)(y+h)};
		shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,vertices,4,true,NULL);
		shp->drawArrays(ShaderProgram::TriangleStrip, 0,4);
		glPopColor();
		glPopBlendFunc();
	}
	else
	{
		ShaderEngine::Engine->clearColor(r * a, g * a, b * a, a);
	}

	finishDraw(ctx);
}

void GRenderTarget::clear(unsigned int color, float a, int x, int y, int w, int h,bool inflow,bool capture)
{
	RENDER_DO([&]{
        clearInt(color,a,x,y,w,h,inflow,capture);
    });
}

void GRenderTarget::drawInt(const Sprite *sprite, const Matrix transform,bool inflow,bool capture)
{
    if (!ShaderEngine::isReady())
        return;

    SavedContext ctx;
    prepareForDraw(ctx,inflow,capture);

    ((Sprite*)sprite)->draw(transform, 0, 0, data->width, data->height);

    finishDraw(ctx);
}

void GRenderTarget::draw(const Sprite *sprite, const Matrix transform,bool inflow,bool capture)
{
	RENDER_DO([&]{
        drawInt(sprite,transform, inflow,capture);
    });
}

void GRenderTarget::generateMipmap() {
	if (data->parameters.filter==eLinearMipmap)
		RENDER_DO([&]{
			data->id()->generateMipmap();
		});
}

void GRenderTarget::getPixels(int x,int y,int w,int h,void *buffer)
{
	if (!ShaderEngine::isReady())
		return;
	RENDER_DO([&]{
		ShaderBuffer *fbo=gtexture_RenderTargetGetFBO(data->gid);
		fbo->readPixels(x,y,w,h, ShaderTexture::FMT_RGBA, ShaderTexture::PK_UBYTE, buffer);
	});
}

int GRenderTarget::save(const char *filename,int x,int y,int w,int h)
{
	unsigned char *buffer=(unsigned char *) malloc(w*h*4);
	getPixels(x,y,w,h,buffer);
	int ret=gimage_saveImage(filename,w,h,buffer);
	free(buffer);
	return ret;
}
