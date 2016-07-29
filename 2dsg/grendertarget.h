#pragma once

#include "texturebase.h"

class Application;
class Sprite;

class GRenderTarget : public TextureBase
{
    ShaderBuffer *prepareForDraw();
public:
    GRenderTarget(Application *application, int width, int height, Filter filter);
    virtual ~GRenderTarget();

    void clear(unsigned int color, float a, int x, int y, int w, int h);
    void draw(const Sprite *sprite);
    void getPixels(int x,int y,int w,int h,void *buffer);
    int save(const char *filename,int x,int y,int w,int h);
};
