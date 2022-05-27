#ifndef GRAPHICSBASE_H
#define GRAPHICSBASE_H

#include <vector>
#include "point.h"
#include "ogl.h"
#include "texturemanager.h"
template <typename T> class VertexBuffer : public std::vector<T>
{
public:
	ShaderBufferCache *bufferCache;
	bool modified;
	void Update()
	{
		modified=true;
	}
    void Cloned() {
        bufferCache=NULL;
        modified=true;
    }
	VertexBuffer()
	{
		bufferCache=NULL;
		modified=true;
	}
	~VertexBuffer()
	{
		if (bufferCache)
			delete bufferCache;
	}
};

class GraphicsBase
{
public:
	GraphicsBase()
	{
		clear();
	}

	ShaderEngine::StandardProgram getShaderType();
	void draw(ShaderProgram *shader = NULL, VertexBuffer<unsigned short> *commonIndices=NULL);

	void clear();
    void cloned() {
        indices.Cloned();
        vertices.Cloned();
        texcoords.Cloned();
        colors.Cloned();
    }

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
	VertexBuffer<unsigned short> indices;
	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
    VertexBuffer<unsigned char> colors;

    void getBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const;

private:
	float r_, g_, b_, a_;
	bool isWhite_;
};

#endif
