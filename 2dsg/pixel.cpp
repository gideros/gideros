#include "pixel.h"
#include "ogl.h"
#include "color.h"

VertexBuffer<unsigned short> Pixel::quad;

Pixel::Pixel(Application *application) : Sprite(application)
{
    r_ = 1, g_ = 1, b_ = 1, a_ = 1;
    width_ = 1, height_ = 1;
    sx_ = 1, sy_ = 1;
    x_ = 0, y_ = 0;
    isStretching_ = false;
	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		texture_[t]=NULL;
	texcoords.resize(4);
	texcoords[0] = Point2f(0,0);
	texcoords[1] = Point2f(1,0);
	texcoords[2] = Point2f(1,1);
	texcoords[3] = Point2f(0,1);
	texcoords.Update();
	if (quad.empty())
	{
		quad.resize(4);
		quad[0] = 0;
		quad[1] = 1;
		quad[2] = 3;
		quad[3] = 2;
		quad.Update();
	}
}

void Pixel::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
    if (!a_ && !shader_) return;
	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		if (texture_[t])
			ShaderEngine::Engine->bindTexture(t,texture_[t]->data->id());
    ShaderProgram *shp=(texture_[0])?ShaderProgram::stdTexture:(
        colors_.empty()?ShaderProgram::stdBasic:ShaderProgram::stdColor);
    if (shader_) shp=shader_;

    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
    shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),texcoords.modified,&texcoords.bufferCache);
    if (!colors_.empty())
    {
        shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&colors_[0],colors_.size()/4,colors_.modified,&colors_.bufferCache);
        colors_.modified=false;
    }
    shp->drawElements(ShaderProgram::TriangleStrip, quad.size(), ShaderProgram::DUSHORT, &quad[0], quad.modified, &quad.bufferCache);
    vertices.modified = false;
	texcoords.modified = false;
	quad.modified = false;

	if (isWhite_ == false)
	{
		glPopColor();
	}
}

void Pixel::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = 0;
    if (miny)
        *miny = 0;
    if (maxx)
        *maxx = width_;
    if (maxy)
        *maxy = height_;
}


void Pixel::updateTexture()
{
    TextureBase* texture = texture_[0];

    float tw = texture->data->exwidth;
    float th = texture->data->exheight;

    float x = x_ / tw;
    float y = y_ / th;

    float w = width_ / tw / sx_;
    float h = height_ / th / sy_;

    if (isStretching_) {
        w = (float)(texture->data->width) / tw / sx_;
        h = (float)(texture->data->height) / th / sy_;
        x = 0.5 * w * (sx_ - 1) - x_ * w; // x_ is relative for stretching
        y = 0.5 * h * (sy_ - 1) - y_ * h; // y_ is relative for stretching
    }

    texcoords[0] = Point2f(x,y);
    texcoords[1] = Point2f(x+w,y);
    texcoords[2] = Point2f(x+w,y+h);
    texcoords[3] = Point2f(x,y+h);
    texcoords.Update();
}

void Pixel::setDimensions(float width,float height)
{
	width_=width;
	height_=height;
	vertices.resize(4);
    vertices[0] = Point2f(0,0);
    vertices[1] = Point2f(width_,0);
    vertices[2] = Point2f(width_,height_);
    vertices[3] = Point2f(0,height_);
	vertices.Update();
    if (texture_[0]) updateTexture();
}

void Pixel::setTexture(TextureBase *texture,int slot, const Matrix4* matrix)
{
    if (texture)
        texture->ref();
    if (texture_[slot])
        texture_[slot]->unref();
    texture_[slot] = texture;

    if (slot==0)
    {
        if (texture) updateTexture();
        if (matrix) for (int tc=0;tc<4;tc++)
			matrix->transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x,&texcoords[tc].y);
 		texcoords.Update();
 	}
}

void Pixel::setTextureMatrix(const Matrix4* matrix)
{
    for (int tc=0;tc<4;tc++)
        matrix->transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x, &texcoords[tc].y);
    texcoords.Update();
}

void Pixel::setTexturePosition(float x, float y)
{
    x_ = x;
    y_ = y;

    if (texture_[0]) updateTexture();
}

void Pixel::setTextureScale(float sx, float sy)
{
    sx_ = sx;
    sy_ = sy;

    if (texture_[0]) updateTexture();
}

void Pixel::setGradient(int c1, float a1, int c2, float a2, int c3, float a3, int c4, float a4)
{
    c1_ = c1, a1_ = a1, c2_ = c2, a2_ = a2, c3_ = c3, a3_ = a3, c4_ = c4, a4_ = a4;
    colors_.resize(16);
    colors_[0] = ((c1 >> 16) & 0xff) * a1;
    colors_[1] = ((c1 >> 8) & 0xff) * a1;
    colors_[2] = (c1 & 0xff) * a1;
    colors_[3] = 255 * a1;
    colors_[4] = ((c2 >> 16) & 0xff) * a2;
    colors_[5] = ((c2 >> 8) & 0xff) * a2;
    colors_[6] = (c2 & 0xff) * a2;
    colors_[7] = 255 * a2;
    colors_[8] = ((c3 >> 16) & 0xff) * a3;
    colors_[9] = ((c3 >> 8) & 0xff) * a3;
    colors_[10] = (c3 & 0xff) * a3;
    colors_[11] = 255 * a3;
    colors_[12] = ((c4 >> 16) & 0xff) * a4;
    colors_[13] = ((c4 >> 8) & 0xff) * a4;
    colors_[14] = (c4 & 0xff) * a4;
    colors_[15] = 255 * a4;
    colors_.Update();
}
