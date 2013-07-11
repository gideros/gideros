#ifndef BITMAPDATA_H
#define BITMAPDATA_H

#include "refptr.h"
#include "texturebase.h"

class BitmapData : public GReferenced
{
public:
	BitmapData(TextureBase* texture);
	BitmapData(TextureBase* texture, int x, int y, int width, int height, int dx1 = 0, int dy1 = 0, int dx2 = 0, int dy2 = 0);

	virtual ~BitmapData();

	TextureBase* texture() const
	{
		return texture_;
	}

	BitmapData* clone();

    void setRegion(int x, int y, int width, int height, int dx1, int dy1, int dx2, int dy2);
    void getRegion(int *x, int *y, int *width, int *height, int *dx1, int *dy1, int *dx2, int *dy2);

private:
	friend class Bitmap;
	friend class TexturePack;

	TextureBase* texture_;
	int x, y;
	int width, height;
	int dx1, dy1;
	int dx2, dy2;
	float u0, v0, u1, v1;

private:
	void initUV();
};


typedef BitmapData TextureRegion;


#endif
