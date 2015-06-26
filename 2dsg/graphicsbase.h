#ifndef GRAPHICSBASE_H
#define GRAPHICSBASE_H

#include <vector>
#include "point.h"
#include "ogl.h"
#include "texturemanager.h"

class GraphicsBase
{
public:
	GraphicsBase()
	{
		clear();
	}

	void draw(ShaderProgram *shader=NULL);

	void clear();

	void setColor(float r, float g, float b, float a)
	{
		r_ = r;
		g_ = g;
		b_ = b;
		a_ = a;

		isWhite_ = r == 1 && g == 1 && b == 1 && a == 1;
	}

	ShaderProgram::ShapeType mode;
	TextureData* data;
	std::vector<unsigned short> indices;
	std::vector<Point2f> vertices;
	std::vector<Point2f> texcoords;

    void getBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const;

private:
	float r_, g_, b_, a_;
	bool isWhite_;
};

#endif
