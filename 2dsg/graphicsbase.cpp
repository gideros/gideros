#include "graphicsbase.h"
#include "ogl.h"
#include "color.h"

void GraphicsBase::clear()
{
	mode = GL_TRIANGLES;
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

		oglEnableClientState(GL_VERTEX_ARRAY);
		oglEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
		glTexCoordPointer(2, GL_FLOAT, 0, &texcoords[0]);

		oglDrawElements(mode, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);

		oglDisableClientState(GL_VERTEX_ARRAY);
		oglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		oglDisable(GL_TEXTURE_2D);

		oglEnableClientState(GL_VERTEX_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);

		oglDrawElements(mode, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);

		oglDisableClientState(GL_VERTEX_ARRAY);
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
