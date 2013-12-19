#ifndef GSPRITEBATCH_H
#define GSPRITEBATCH_H

#include "sprite.h"

class Application;

class GSpriteBatch : public Sprite
{
public:
    GSpriteBatch(Application *application);
    virtual ~GSpriteBatch();

    void setSprite(Sprite *sprite);
    void setPosition(int i, float x, float y);
    void setRotation(int i, float rotation);
    void setScale(int i, float x, float y);
    void setColorTransform(int i, float r, float g, float b, float a);
    void setAlpha(int i, float a);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
    virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

private:
    Sprite *sprite_;
};

#endif
