#include "graphicsbase.h"
#include "ogl.h"
#include "color.h"
#include <algorithm>

void GraphicsBase::clear()
{
	mode = ShaderProgram::Triangles;
	r_ = g_ = b_ = a_ = 1;
	isWhite_ = true;
	data = NULL;
	indices.clear();
	vertices.clear();
	texcoords.clear();
}

void GraphicsBase::draw()
{
	if (indices.empty())
		return;

	if (isWhite_ == false)
	{
		glPushColor();
		glMultColor(r_, g_, b_, a_);
	}

	if (data)
	{
		oglEnable(GL_TEXTURE_2D);

        oglBindTexture(GL_TEXTURE_2D, data->id());

        ShaderProgram::stdTexture->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),true,NULL);
        ShaderProgram::stdTexture->setData(ShaderProgram::DataTexture,ShaderProgram::DFLOAT,2,&texcoords[0],texcoords.size(),true,NULL);
        ShaderProgram::stdTexture->drawElements(mode,indices.size(), ShaderProgram::DUSHORT, &indices[0],true, NULL);
	}
	else
	{
		oglDisable(GL_TEXTURE_2D);

        ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2,&vertices[0],vertices.size(),true,NULL);
        ShaderProgram::stdBasic->drawElements(mode,indices.size(), ShaderProgram::DUSHORT, &indices[0],true, NULL);
	}

	if (isWhite_ == false)
	{
		glPopColor();
	}
}

void GraphicsBase::getBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const
{
	float minx = 1e30;
	float miny = 1e30;
	float maxx = -1e30;
	float maxy = -1e30;

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float x = vertices[i].x;
		float y = vertices[i].y;

		minx = std::min(minx, x);
		miny = std::min(miny, y);
		maxx = std::max(maxx, x);
		maxy = std::max(maxy, y);
	}

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}
