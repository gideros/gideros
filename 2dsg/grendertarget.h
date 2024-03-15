#pragma once

#include "texturebase.h"
#include "Matrices.h"

class Application;
class Sprite;

class GRenderTarget : public TextureBase
{
	struct SavedContext {
		ShaderBuffer *oldfbo;
		int ovx,ovy,ovw,ovh;
		Matrix4 oldProj;
		Matrix4 oldView;
		Matrix4 oldModel;
	};
	bool depthRt_;
    void prepareForDraw(SavedContext &ctx,bool inflow,bool capture);
    void finishDraw(SavedContext &ctx);
    void clearInt(unsigned int color, float a, int x, int y, int w, int h,bool inflow,bool capture);
    void drawInt(const Sprite *sprite, const Matrix4 transform,bool inflow,bool capture);
public:
    GRenderTarget(Application *application, int width, int height, TextureParameters &parameters, bool selectScale=false, bool depth=false);
    virtual ~GRenderTarget();

    void clear(unsigned int color, float a, int x, int y, int w, int h,bool inflow=false,bool capture=false);
    void draw(const Sprite *sprite, const Matrix4 transform,bool inflow=false,bool capture=false);
    void generateMipmap();
    void getPixels(int x,int y,int w,int h,void *buffer);
    void resize(int width, int height, float scaleX, float scaleY);
    int save(const char *filename,int x,int y,int w,int h);
};
