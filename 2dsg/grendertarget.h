#pragma once

#include "texturebase.h"

class Application;
class Sprite;

class GRenderTarget : public TextureBase
{
public:
    GRenderTarget(Application *application, int width, int height, Filter filter);
    virtual ~GRenderTarget();

    void clear(unsigned int color, float a);

    void draw(const Sprite *sprite);
};
