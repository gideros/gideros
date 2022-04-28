#pragma once

#include "texturebase.h"
#include "Matrices.h"

class Application;
class Sprite;

class GRenderTarget : public TextureBase
{
    ShaderBuffer *prepareForDraw();
public:
    GRenderTarget(Application *application, int width, int height, TextureParameters &parameters, bool selectScale=false, bool depth=false);
    virtual ~GRenderTarget();

    void clear(unsigned int color, float a, int x, int y, int w, int h);
    void draw(const Sprite *sprite, const Matrix4 transform);
    void getPixels(int x,int y,int w,int h,void *buffer);
    void resize(int width, int height, float scaleX, float scaleY);
    int save(const char *filename,int x,int y,int w,int h);
};
