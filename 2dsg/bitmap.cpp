#include "bitmap.h"
#include "ogl.h"

VertexBuffer<unsigned short> Bitmap::quad;
void Bitmap::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
    G_UNUSED(sx); G_UNUSED(sy); G_UNUSED(ex); G_UNUSED(ey);
	if (quad.empty())
	{
		quad.resize(4);
		quad[0] = 0;
		quad[1] = 1;
		quad[2] = 3;
		quad[3] = 2;
		quad.Update();
	}
	graphicsBase_.draw(getShader(graphicsBase_.getShaderType()),&quad);
}

void Bitmap::cloneFrom(Bitmap *s) {
    Sprite::cloneFrom(s);

    bitmapdata_ = s->bitmapdata_;
    if (bitmapdata_ != NULL)
        bitmapdata_->ref();

    texturebase_ = s->texturebase_;
    if (texturebase_)
        texturebase_->ref();

    anchorx_ = s->anchorx_;
    anchory_ = s->anchory_;
    dx_ = s->dx_;
    dy_ = s->dy_;
    minx_ = s->minx_;
    miny_ = s->miny_;
    maxx_ = s->maxx_;
    maxy_ = s->maxy_;

    graphicsBase_=s->graphicsBase_;
    graphicsBase_.cloned();
}

void Bitmap::updateBounds()
{
    if (bitmapdata_ != NULL)
    {
        float sizescalex = bitmapdata_->texture()->sizescalex;
        float sizescaley = bitmapdata_->texture()->sizescaley;

        minx_ = sizescalex * dx_;
        miny_ = sizescaley * dy_;
        maxx_ = sizescalex * (bitmapdata_->width + bitmapdata_->dx1 + bitmapdata_->dx2 + dx_);
        maxy_ = sizescaley * (bitmapdata_->height + bitmapdata_->dy1 + bitmapdata_->dy2 + dy_);
    }
    else if (texturebase_ != NULL)
    {
        float sizescalex = texturebase_->sizescalex;
        float sizescaley = texturebase_->sizescaley;

        minx_ = sizescalex * dx_;
        miny_ = sizescaley * dy_;
        maxx_ = sizescalex * (texturebase_->data->baseWidth + dx_);
        maxy_ = sizescaley * (texturebase_->data->baseHeight + dy_);
    }
}

void Bitmap::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = minx_;
    if (miny)
        *miny = miny_;
    if (maxx)
        *maxx = maxx_;
    if (maxy)
        *maxy = maxy_;
}

void Bitmap::setCoords()
{
	if (bitmapdata_ != NULL)
	{
		graphicsBase_.data = bitmapdata_->texture()->data;

		graphicsBase_.mode = ShaderProgram::TriangleStrip;

        float sizescalex = bitmapdata_->texture()->sizescalex;
        float sizescaley = bitmapdata_->texture()->sizescaley;

		graphicsBase_.vertices.resize(4);
        graphicsBase_.vertices[0] = Point2f(sizescalex * (bitmapdata_->dx1 + dx_),                      sizescaley * (bitmapdata_->dy1 + dy_));
        graphicsBase_.vertices[1] = Point2f(sizescalex * (bitmapdata_->dx1 + bitmapdata_->width + dx_),	sizescaley * (bitmapdata_->dy1 + dy_));
        graphicsBase_.vertices[2] = Point2f(sizescalex * (bitmapdata_->dx1 + bitmapdata_->width + dx_),	sizescaley * (bitmapdata_->dy1 + bitmapdata_->height + dy_));
        graphicsBase_.vertices[3] = Point2f(sizescalex * (bitmapdata_->dx1 + dx_),                      sizescaley * (bitmapdata_->dy1 + bitmapdata_->height + dy_));
		graphicsBase_.vertices.Update();

		graphicsBase_.texcoords.resize(4);
		graphicsBase_.texcoords[0] = Point2f(bitmapdata_->u0, bitmapdata_->v0);
		graphicsBase_.texcoords[1] = Point2f(bitmapdata_->u1, bitmapdata_->v0);
		graphicsBase_.texcoords[2] = Point2f(bitmapdata_->u1, bitmapdata_->v1);
		graphicsBase_.texcoords[3] = Point2f(bitmapdata_->u0, bitmapdata_->v1);
		graphicsBase_.texcoords.Update();		
	}
	else if (texturebase_ != NULL)
	{
		graphicsBase_.data = texturebase_->data;

		graphicsBase_.mode = ShaderProgram::TriangleStrip;

        float sizescalex = texturebase_->sizescalex;
        float sizescaley = texturebase_->sizescaley;

		graphicsBase_.vertices.resize(4);
        graphicsBase_.vertices[0] = Point2f(sizescalex * dx_,                                   sizescaley * dy_);
        graphicsBase_.vertices[1] = Point2f(sizescalex * (texturebase_->data->baseWidth + dx_), sizescaley * dy_);
        graphicsBase_.vertices[2] = Point2f(sizescalex * (texturebase_->data->baseWidth + dx_), sizescaley * (texturebase_->data->baseHeight + dy_));
        graphicsBase_.vertices[3] = Point2f(sizescalex * dx_,                                   sizescaley * (texturebase_->data->baseHeight + dy_));
		graphicsBase_.vertices.Update();

		float u = (float)texturebase_->data->width / (float)texturebase_->data->exwidth;
		float v = (float)texturebase_->data->height / (float)texturebase_->data->exheight;

		graphicsBase_.texcoords.resize(4);
		graphicsBase_.texcoords[0] = Point2f(0, 0);
		graphicsBase_.texcoords[1] = Point2f(u, 0);
		graphicsBase_.texcoords[2] = Point2f(u, v);
		graphicsBase_.texcoords[3] = Point2f(0, v);
		graphicsBase_.texcoords.Update();
	}
}

void Bitmap::setAnchorPoint(float x, float y)
{
	anchorx_ = x;
	anchory_ = y;

	if (bitmapdata_ != NULL)
	{
		float rx = anchorx_ * (bitmapdata_->width + bitmapdata_->dx1 + bitmapdata_->dx2);
		float ry = anchory_ * (bitmapdata_->height + bitmapdata_->dy1 + bitmapdata_->dy2);
#if 0
		dx_ = floor(-rx + 0.5f);
		dy_ = floor(-ry + 0.5f);
#else
        dx_ = -rx;
        dy_ = -ry;
#endif
	}
	else if (texturebase_ != NULL)
	{
		float rx = anchorx_ * texturebase_->data->baseWidth;
		float ry = anchory_ * texturebase_->data->baseHeight;
#if 0
		dx_ = floor(-rx + 0.5f);
		dy_ = floor(-ry + 0.5f);										// NOTE: -floor(ry + 0.5f) also gives the same result
#else
        dx_ = -rx;
        dy_ = -ry;
#endif
	}

	setCoords();
    updateBounds();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Bitmap::getAnchorPoint(float* x, float* y) const
{
	if (x)
		*x = anchorx_;
	if (y)
		*y = anchory_;
}

void Bitmap::setTextureRegion(BitmapData *bitmapdata)
{
    BitmapData *originalbitmapdata = bitmapdata_;
    TextureBase *originaltexturebase = texturebase_;

    bitmapdata_ = bitmapdata;
    bitmapdata_->ref();
    texturebase_ = NULL;

    setAnchorPoint(anchorx_, anchory_);  // here setAnchorPoint updates geometry and bounds

    if (originalbitmapdata)
        originalbitmapdata->unref();
    if (originaltexturebase)
        originaltexturebase->unref();
}

void Bitmap::setTexture(TextureBase *texturebase)
{
    BitmapData *originalbitmapdata = bitmapdata_;
    TextureBase *originaltexturebase = texturebase_;

    bitmapdata_ = NULL;
    texturebase_ = texturebase;
    texturebase_->ref();

    setAnchorPoint(anchorx_, anchory_);  // here setAnchorPoint updates geometry and bounds

    if (originalbitmapdata)
        originalbitmapdata->unref();
    if (originaltexturebase)
        originaltexturebase->unref();
}
