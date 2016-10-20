#ifndef PIXEL_H
#define PIXEL_H

#include "sprite.h"
#include "graphicsbase.h"
#include "texturebase.h"

#define PIXEL_MAX_TEXTURES 8
class Pixel : public Sprite
{
	static VertexBuffer<unsigned short> quad;
public:
    Pixel(Application *application);

	void setColor(float r, float g, float b, float a)
	{
		r_ = r;
		g_ = g;
		b_ = b;
		a_ = a;

		isWhite_ = r == 1 && g == 1 && b == 1 && a == 1;
	}

	void getColor(float &r,float &g, float &b, float &a)
	{
		r=r_;
		g=g_;
		b=b_;
		a=a_;
	}


	virtual ~Pixel()
	{
	}

	void setWidth(float width)
	{
		setDimensions(width,height_);
	}
	void setHeight(float height)
	{
		setDimensions(width_,height);
	}
	void setDimensions(float width,float height);
	void setTexture(TextureBase *texture,int slot, const Matrix4* matrix = NULL);


private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
    TextureBase *texture_[PIXEL_MAX_TEXTURES];
	float r_, g_, b_, a_;
	float width_,height_;
	bool isWhite_;
};

#endif
