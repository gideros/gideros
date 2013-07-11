#include "bitmapdata.h"
#include <math.h>

BitmapData::BitmapData(TextureBase* texture)
{
	//printf("TextureRegion()\n");
	texture_ = texture;
	texture_->ref();

	x = 0;
	y = 0;
	width = texture->data->baseWidth;
	height = texture->data->baseHeight;
	dx1 = 0;
	dy1 = 0;
	dx2 = 0;
	dy2 = 0;

	initUV();
}

BitmapData::BitmapData(TextureBase* texture, int x, int y, int width, int height, int dx1/* = 0*/, int dy1/* = 0*/, int dx2/* = 0*/, int dy2/* = 0*/) :
	x(x), y(y),
	width(width), height(height),
	dx1(dx1), dy1(dy1),
	dx2(dx2), dy2(dy2)
{
	//printf("TextureRegion()\n");
	texture_ = texture;
	texture_->ref();

	initUV();
}

BitmapData::~BitmapData()
{
	//printf("~TextureRegion()\n");
	texture_->unref();
}

void BitmapData::initUV()
{
	u0 = (float)x / (float)texture_->data->exwidth;
	v0 = (float)y / (float)texture_->data->exheight;
	u1 = (float)(x + width) / (float)texture_->data->exwidth;
	v1 = (float)(y + height) / (float)texture_->data->exheight;

	// uv scale
	{
		float scalex = texture_->uvscalex;
		float scaley = texture_->uvscaley;

        u0 *= scalex;
        v0 *= scaley;
        u1 *= scalex;
        v1 *= scaley;
	}
}

BitmapData* BitmapData::clone()
{
	return new BitmapData(texture_, x, y, width, height, dx1, dy1, dx2, dy2);
}

void BitmapData::setRegion(int x, int y, int width, int height, int dx1, int dy1, int dx2, int dy2)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->dx1 = dx1;
    this->dy1 = dy1;
    this->dx2 = dx2;
    this->dy2 = dy2;

    initUV();
}

void BitmapData::getRegion(int *x, int *y, int *width, int *height, int *dx1, int *dy1, int *dx2, int *dy2)
{
    if (x)
        *x = this->x;
    if (y)
        *y = this->y;
    if (width)
        *width = this->width;
    if (height)
        *height = this->height;
    if (dx1)
        *dx1 = this->dx1;
    if (dy1)
        *dy1 = this->dy1;
    if (dx2)
        *dx2 = this->dx2;
    if (dy2)
        *dy2 = this->dy2;
}
