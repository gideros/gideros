#ifndef PIXEL_H
#define PIXEL_H

#include "sprite.h"
#include "graphicsbase.h"
#include "texturebase.h"
#include "bitmapdata.h"

#define PIXEL_MAX_TEXTURES 8
class Pixel : public Sprite
{
	static VertexBuffer<unsigned short> quad;
	static VertexBuffer<unsigned short> ninepatch;
public:
    Pixel(Application *application);
    virtual Sprite *clone() { Pixel *clone=new Pixel(application_); clone->cloneFrom(this); return clone; }
    void cloneFrom(Pixel *);

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


    virtual ~Pixel();

	void setWidth(float width)
	{
		setDimensions(width,height_);
	}
	void setHeight(float height)
	{
		setDimensions(width_,height);
	}

	bool setDimensions(float width,float height,bool forLayout=false);
    void getDimensions(float &width, float &height)
    {
        width = width_;
        height = height_;
    }
    void getMinimumSize(float &w,float &h,bool preferred) { G_UNUSED(preferred); w=minw_; h=minh_; }

	void setAnchorPoint(float x, float y);
	void getAnchorPoint(float* x, float* y) const;

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
	void setTextureRegion(BitmapData *bitmapdata,int slot);
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

    void setNinePatch(float vl,float vr,float vt,float vb,float tl,float tr,float tt,float tb);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;
    int getMixedColor(int c1, int c2, float a1, float a2, float a, float &ao);
    void updateTexture();
    void updateVertices();

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
    bool isNinePatch_;
    Matrix4 tmatrix_;
    float insetv_t_,insetv_b_,insetv_r_,insetv_l_;
    float insett_t_,insett_b_,insett_r_,insett_l_;
    float minw_,minh_;
	float tx_,ty_,tw_,th_; //Texture coordinates in texels
	float anchorx_, anchory_;
};

#endif
