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
    void Clear() {
        std::vector<T>::clear();
        if (bufferCache)
            delete bufferCache;
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
        indices32=nullptr;
        clear();
    }
    GraphicsBase(const GraphicsBase &o)
    {
        *this=o;
    }
    GraphicsBase(GraphicsBase &&o) noexcept : mode(o.mode), data(o.data),
        indices(std::move(o.indices)), vertices(std::move(o.vertices)), texcoords(std::move(o.texcoords)),
        colors(std::move(o.colors)),  indices32(o.indices32), isSecondary(o.isSecondary),
        isWhite_(o.isWhite_), r_(o.r_), g_(o.g_), b_(o.b_), a_(o.a_)
    {
        o.indices32=nullptr;
    }
    ~GraphicsBase() {
        if (indices32)
            delete indices32;
        indices32=nullptr;
    }
    GraphicsBase& operator=(const GraphicsBase &o) {
        indices=o.indices;
        vertices=o.vertices;
        texcoords=o.texcoords;
        colors=o.colors;
        if (o.indices32)
        {
            indices32=new VertexBuffer<uint32_t>();
            indices32->assign(o.indices32->cbegin(),o.indices32->cend());
        }
        else
            indices32=nullptr;
        mode=o.mode;
        data=o.data;
        isSecondary=o.isSecondary;
        isWhite_=o.isWhite_;
        r_=o.r_;
        g_=o.g_;
        b_=o.b_;
        a_=o.a_;
        return *this;
    }

	ShaderEngine::StandardProgram getShaderType();
	void draw(ShaderProgram *shader = NULL, VertexBuffer<unsigned short> *commonIndices=NULL);

	void clear();
    void cloned() {
        indices.Cloned();
        vertices.Cloned();
        texcoords.Cloned();
        colors.Cloned();
        if (indices32)
            indices32->Cloned();
    }
    void enable32bitIndices()
    {
        if (!indices32)
        {
            indices32=new VertexBuffer<uint32_t>();
            indices32->resize(indices.size());
            for (size_t k=0;k<indices.size();k++)
                (*indices32)[k]=indices[k];
            indices.clear();
        }
    }
    void indicesSet(int index,uint32_t v) {
        if (indices32)
            (*indices32)[index]=v;
        else
            indices[index]=v;
    }
    void indicesResize(size_t s) {
        if (indices32)
            indices32->resize(s);
        else
            indices.resize(s);
    }
    void indicesUpdate() {
        if (indices32)
            indices32->Update();
        else
            indices.Update();
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
    VertexBuffer<uint32_t> *indices32;

    void getBounds(float* pminx, float* pminy, float* pmaxx, float* pmaxy) const;

    bool isSecondary;
private:
    bool isWhite_;
    float r_, g_, b_, a_;
};

#endif
