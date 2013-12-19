#include "gspritebatch.h"

GSpriteBatch::GSpriteBatch(Application *application) : Sprite(application)
{
    sprite_ = NULL;
}

GSpriteBatch::~GSpriteBatch()
{
    if (sprite_)
        sprite_->unref();
}

void GSpriteBatch::setSprite(Sprite *sprite)
{
    if (sprite)
        sprite->ref();
    if (sprite_)
        sprite_->unref();
    sprite_ = sprite;
}

void GSpriteBatch::setPosition(int i, float x, float y)
{

}

void GSpriteBatch::setRotation(int i, float rotation)
{

}

void GSpriteBatch::setScale(int i, float x, float y)
{

}

void GSpriteBatch::setColorTransform(int i, float r, float g, float b, float a)
{

}

void GSpriteBatch::setAlpha(int i, float a)
{

}

void GSpriteBatch::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{

}

void GSpriteBatch::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{

}

