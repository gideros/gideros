#ifndef GMESH_H
#define GMESH_H

#include <sprite.h>
#include <vector>
#include <texturebase.h>

class Application;

class GMesh: public Sprite
{
public:
    GMesh(Application *application);
    virtual ~GMesh();

    void setVertex(int i, float x, float y);
    void setIndex(int i, unsigned short index);
    void setColor(int i, unsigned int color, float alpha);
    void setTextureCoordinate(int i, float u, float v);

    void setVertexArray(const float *vertices, size_t size);
    void setIndexArray(const unsigned short *indices, size_t size);
    void setColorArray(const unsigned int *colors, const float *alphas, size_t size);
    void setTextureCoordinateArray(const float *textureCoordinates, size_t size);

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

    void getVertex(int i, float *x, float *y) const;
    void getIndex(int i, unsigned short *index) const;
    void getColor(int i, unsigned int *color, float *alpha) const;
    void getTextureCoordinate(int i, float *u, float *v) const;

    void setTexture(TextureBase *texture);
    void clearTexture();

private:
    virtual void doDraw(const CurrentTransform &, float sx, float sy, float ex, float ey);
    virtual void extraBounds(float *minx, float *miny, float *maxx, float *maxy) const;

private:
    struct Color
    {
        unsigned int color;
        float alpha;
    };
    std::vector<float> vertices_;
    std::vector<unsigned short> indices_;
    std::vector<unsigned char> colors_;
    std::vector<Color> originalColors_;
    std::vector<float> textureCoordinates_;
    std::vector<float> originalTextureCoordinates_;

    TextureBase *texture_;

    float sx_, sy_;

    float r_, g_, b_, a_;

    mutable float minx_, miny_, maxx_, maxy_;
    bool boundsDirty_;
};


#endif
