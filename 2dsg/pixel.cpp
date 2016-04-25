#include "pixel.h"
#include "ogl.h"
#include "color.h"

VertexBuffer<unsigned short> Pixel::quad;
VertexBuffer<Point2f> Pixel::texquad;
void Pixel::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
	if (quad.empty())
	{
		quad.resize(4);
		quad[0] = 0;
		quad[1] = 1;
		quad[2] = 3;
		quad[3] = 2;
		quad.Update();
	}
	if (texquad.empty())
	{
		texquad.resize(4);
		texquad[0] = Point2f(0,0);
		texquad[1] = Point2f(1,0);
		texquad[2] = Point2f(1,1);
		texquad[3] = Point2f(0,1);
		texquad.Update();
	}

	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	ShaderProgram *shp=shader_;
    if (!shp) shp=ShaderProgram::stdBasic;
    shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),vertices.modified,&vertices.bufferCache);
    shp->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texquad[0],texquad.size(),texquad.modified,&texquad.bufferCache);
    shp->drawElements(ShaderProgram::TriangleStrip, quad.size(), ShaderProgram::DUSHORT, &quad[0], quad.modified, &quad.bufferCache);
    vertices.modified = false;
	texquad.modified = false;
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
}
