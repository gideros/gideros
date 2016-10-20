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
	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	for (int t=0;t<PIXEL_MAX_TEXTURES;t++)
		if (texture_[t])
			ShaderEngine::Engine->bindTexture(t,texture_[t]->data->id());
	ShaderProgram *shp=(texture_[0])?ShaderProgram::stdTexture:ShaderProgram::stdBasic;
	if (shader_)
		shp=shader_;

    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
    shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),texcoords.modified,&texcoords.bufferCache);
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

    float w = width_ / texture->data->exwidth / sx_;
    float h = height_ / texture->data->exheight / sy_;

    float x = x_ / (float)(texture->data->width);
    float y = y_ / (float)(texture->data->height);

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
    	float sx=1,sy=1;
    	if (texture)
    	{
    		sx = ((float)texture->data->width) / texture->data->exwidth;
    		sy = ((float)texture->data->height) / texture->data->exheight;
    	}
 		texcoords[0] = Point2f(0,0);
 		texcoords[1] = Point2f(sx,0);
 		texcoords[2] = Point2f(sx,sy);
 		texcoords[3] = Point2f(0,sy);
 		if (matrix)
 	    for (int tc=0;tc<4;tc++)
			matrix->transformPoint(texcoords[tc].x, texcoords[tc].y, &texcoords[tc].x,&texcoords[tc].y);
 		texcoords.Update();
 	}
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
