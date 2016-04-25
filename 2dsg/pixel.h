#ifndef PIXEL_H
#define PIXEL_H

#include "sprite.h"
#include "graphicsbase.h"

class Pixel : public Sprite
{
	static VertexBuffer<unsigned short> quad;
	static VertexBuffer<Point2f> texquad;
public:
    Pixel(Application *application) : Sprite(application)
	{
    	setColor(1,1,1,1);
    	setDimensions(1,1);
	}

	void setColor(float r, float g, float b, float a)
	{
		r_ = r;
		g_ = g;
		b_ = b;
		a_ = a;

		isWhite_ = r == 1 && g == 1 && b == 1 && a == 1;
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

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

	VertexBuffer<Point2f> vertices;
	float r_, g_, b_, a_;
	float width_,height_;
	bool isWhite_;
};

#endif
