#include "gbitmapbatch.h"
#include "bitmapdata.h"

GBitmapArray::GBitmapArray(Application *application, BitmapData *bitmapdata) : Sprite(application)
{
    bitmapdata_ = bitmapdata;
    bitmapdata_->ref();

    texturebase_ = NULL;

    anchorx_ = 0;
    anchory_ = 0;
}

GBitmapArray::GBitmapArray(Application *application, TextureBase *texturebase) : Sprite(application)
{
    texturebase_ = texturebase;
    texturebase_->ref();

    bitmapdata_ = NULL;

    anchorx_ = 0;
    anchory_ = 0;
}

GBitmapArray::~GBitmapArray()
{

}

void GBitmapArray::setPosition(int i, float x, float y)
{

}

void GBitmapArray::setRotation(int i, float rotation)
{

}

void GBitmapArray::setScale(int i, float x, float y)
{

}

void GBitmapArray::setColorTransform(int i, float r, float g, float b, float a)
{

}

void GBitmapArray::setAlpha(int i, float a)
{

}

void GBitmapArray::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{

}

void GBitmapArray::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{

}

