#ifndef GMESH_H
#define GMESH_H

#include <sprite.h>
#include <vector>
#include <texturebase.h>
#include "ogl.h"
#include "ticker.h"

class Application;

class Particles: public Sprite, Ticker
{
public:
    Particles(Application *application);
    virtual ~Particles();

    int addParticle(float x, float y, float size, float angle, int ttl);
    void removeParticle(int i);
    void setColor(int i, unsigned int color, float alpha);
    void getColor(int i, unsigned int *color, float *alpha) const;
    void setSpeed(int i, float vx, float vy, float va, float decay);
    void getSpeed(int i, float *vx, float *vy, float *va, float *decay) const;

    void setTexture(TextureBase *texture);
    void clearTexture();

    int getParticleCount() const { return ttl_.size(); };

private:
    virtual void doDraw(const CurrentTransform &, float sx, float sy, float ex, float ey);
    virtual void extraBounds(float *minx, float *miny, float *maxx, float *maxy) const;
    virtual void tick();
private:
    struct Color
    {
        unsigned int color;
        float alpha;
    };
    std::vector<Color> originalColors_;
    VertexBuffer<unsigned char> colors_; //r,g,b,a
    VertexBuffer<float> points_; //x,y,size,angle
    VertexBuffer<float> speeds_; //vx,vy,va,decay
    VertexBuffer<int> ttl_; //time to live
    TextureBase *texture_;
    float sx_, sy_;

    float r_, g_, b_, a_;

    mutable float minx_, miny_, maxx_, maxy_;
    bool boundsDirty_;
};

#endif
