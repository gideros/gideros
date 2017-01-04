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

    void updateTexture();

	void setWidth(float width)
	{
		setDimensions(width,height_);
	}
	void setHeight(float height)
	{
		setDimensions(width_,height);
	}
	void setDimensions(float width,float height);
    void getDimensions(float &width, float &height)
    {
        width = width_;
        height = height_;
    }

    void setTexturePosition(float x,float y);
    void getTexturePosition(float &x, float &y)
    {
        x = x_;
        y = y_;
    }

    void setTextureScale(float sx,float sy);
    void getTextureScale(float &sx, float &sy)
    {
        sx = sx_;
        sy = sy_;
    }

	void setTexture(TextureBase *texture,int slot, const Matrix4* matrix = NULL);

    void setTextureMatrix(const Matrix4* matrix);

    void setGradient(int c1, float a1, int c2, float a2, int c3, float a3, int c4, float a4);
    bool hasGradient()
    {
        return !colors_.empty();
    }
    void getGradient(int &c1, float &a1, int &c2, float &a2, int &c3, float &a3, int &c4, float &a4)
    {
        c1 = c1_, a1 = a1_, c2 = c2_, a2 = a2_, c3 = c3_, a3 = a3_, c4 = c4_, a4 = a4_;
    }
    void setGradientWithAngle(int co1, float a1, int co2, float a2, float angle);
    void clearGradient()
    {
        colors_.clear();
    }

    void setStretching(bool isStretching)
    {
        isStretching_ = isStretching;
    }

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;
    int getMixedColor(int c1, int c2, float a);

	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
    VertexBuffer<unsigned char> colors_;
    TextureBase *texture_[PIXEL_MAX_TEXTURES];
	float r_, g_, b_, a_;
    int c1_, c2_, c3_, c4_;
    float a1_, a2_, a3_, a4_;
	float width_,height_;
    float x_, y_;
    float sx_, sy_;
	bool isWhite_;
    bool isStretching_;

};

#endif
