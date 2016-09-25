#ifndef GMESH_H
#define GMESH_H

#include <sprite.h>
#include <vector>
#include <texturebase.h>
#include "ogl.h"
#include "graphicsbase.h"

class Application;

#define MESH_MAX_TEXTURES 8
#define MESH_MAX_ARRAYS   8
class GMesh: public Sprite
{
public:
    GMesh(Application *application, bool is3d);
    virtual ~GMesh();
    bool is3d();

    void setVertex(int i, float x, float y, float z);
    void setIndex(int i, unsigned short index);
    void setColor(int i, unsigned int color, float alpha);
    void setTextureCoordinate(int i, float u, float v);

    void setVertexArray(const float *vertices, size_t size);
    void setIndexArray(const unsigned short *indices, size_t size);
    void setColorArray(const unsigned int *colors, const float *alphas, size_t size);
    void setTextureCoordinateArray(const float *textureCoordinates, size_t size);
    void setGenericArray(int index,const void *pointer, ShaderProgram::DataType type, int mult, int count);

    void resizeVertexArray(size_t size);
    void resizeIndexArray(size_t size);
    void resizeColorArray(size_t size);
    void resizeTextureCoordinateArray(size_t size);

    void clearVertexArray();
    void clearIndexArray();
    void clearColorArray();
    void clearTextureCoordinateArray();

    size_t getVertexArraySize() const;
    size_t getIndexArraySize() const;
    size_t getColorArraySize() const;
    size_t getTextureCoordinateArraySize() const;

    void getVertex(int i, float *x, float *y, float *z) const;
    void getIndex(int i, unsigned short *index) const;
    void getColor(int i, unsigned int *color, float *alpha) const;
    void getTextureCoordinate(int i, float *u, float *v) const;

    void setTexture(TextureBase *texture,int slot=0);
    void clearTexture(int slot=0);
    void setPrimitiveType(ShaderProgram::ShapeType type);

private:
    virtual void doDraw(const CurrentTransform &, float sx, float sy, float ex, float ey);
    virtual void childrenDrawn();
    virtual void extraBounds(float *minx, float *miny, float *maxx, float *maxy) const;

private:
    struct Color
    {
        unsigned int color;
        float alpha;
    };
    VertexBuffer<float> vertices_;
    VertexBuffer<unsigned short> indices_;
    VertexBuffer<unsigned char> colors_;
    std::vector<Color> originalColors_;
    VertexBuffer<float> textureCoordinates_;
    std::vector<float> originalTextureCoordinates_;
    struct _genArray
    {
    	void *ptr;
    	ShaderProgram::DataType type;
    	int mult;
    	int count;
    	ShaderBufferCache *cache;
    	bool modified;
    } genericArray[MESH_MAX_ARRAYS-3];

    TextureBase *texture_[MESH_MAX_TEXTURES];
    float sx_[MESH_MAX_TEXTURES], sy_[MESH_MAX_TEXTURES];

    float r_, g_, b_, a_;

    mutable float minx_, miny_, maxx_, maxy_;
    bool boundsDirty_;
    ShaderProgram::ShapeType meshtype_;
    bool mesh3d_;
};

#endif
