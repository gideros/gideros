#ifndef GSPRITEBATCH_H
#define GSPRITEBATCH_H

#include "sprite.h"

class Application;
class BitmapData;
class TextureBase;

class GBitmapArray : public Sprite
{
public:
    GBitmapArray(Application *application, BitmapData *bitmapdata);
    GBitmapArray(Application *application, TextureBase *texturebase);
    virtual ~GBitmapArray();

    void setTextureRegion(BitmapData *bitmapdata);
    void setTexture(TextureBase *texturebase);

    void setAnchorPoint(float x, float y);
    void getAnchorPoint(float *x, float *y) const;

    void setPosition(int i, float x, float y);
    void setRotation(int i, float rotation);
    void setScale(int i, float x, float y);
    void setColorTransform(int i, float r, float g, float b, float a);
    void setAlpha(int i, float a);
    void setVisible(int i, bool visible);

    void getPosition(int i, float *x, float *y) const;
    float getRotation(int i) const;
    void getScale(int i, float *x, float *y) const;
    void getColorTransform(int i, float *r, float *g, float *b, float *a) const;
    float getAlpha(int i) const;
    const void getVisible(int i) const;

    size_t getSize() const;
    void setSize(size_t size);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
    virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

private:
    BitmapData  *bitmapdata_;
    TextureBase *texturebase_;

    float anchorx_, anchory_;
};

#endif
