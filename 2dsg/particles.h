#ifndef GPARTICLES_H
#define GPARTICLES_H

#include <sprite.h>
#include <vector>
#include <string>
#include <texturebase.h>
#include "ogl.h"
#include "ticker.h"

class Application;

#define PARTICLES_MAX_TEXTURES 8
class Particles: public Sprite, Ticker
{
public:
    Particles(Application *application, bool is3d, bool autoSort);
    virtual Sprite *clone() { Particles *clone=new Particles(application_,is3d,autoSort); clone->cloneFrom(this); return clone; }
    void cloneFrom(Particles *);
    virtual ~Particles();

    int addParticle(float x, float y, float z, float size, float angle, int ttl, float extra);
    void removeParticle(int i);
    void clearParticles();
    void setPosition(int i, float x, float y, float z);
    void getPosition(int i, float *x, float *y, float *z);
    void setSize(int i, float size);
    float getSize(int i);
    void setAngle(int i, float angle);
    float getAngle(int i);
    void setTtl(int i, int ttl);
    int getTtl(int i);
    void setColor(int i, unsigned int color, float alpha);
    void getColor(int i, unsigned int *color, float *alpha) const;
    void setSpeed(int i, float vx, float vy, float vz, float va, float decay);
    void getSpeed(int i, float *vx, float *vy, float *vz, float *va, float *decay) const;
    void setDecay(int i, float vx, float vy, float vz, float vc, float vs, float va);
    void getDecay(int i, float *vx, float *vy, float *vz, float *vc, float *vs,float *va) const;
    void setAcceleration(int i, float vx, float vy, float vz, float vc, float vs, float va);
    void getAcceleration(int i, float *vx, float *vy, float *vz, float *vc, float *vs,float *va) const;
    void setTag(int i, const char *tag);
    const char *getTag(int i) const;
    void setExtra(int i, float type);
    float getExtra(int i) const;
    void scaleParticles(float size,bool absolute);

    void setTexture(TextureBase *texture,int slot);
    void clearTexture(int slot);

    int getParticleCount() const { return ttl_.size(); };
    void setPaused(bool paused) { paused_=paused; };
    bool isPaused() { return paused_; };
    std::set<int> getDead();

    bool is3d;
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
    std::set<int> dead_;
    std::vector<Color> originalColors_;
    VertexBuffer<unsigned char> colors_; //r,g,b,a
    VertexBuffer<float> points_; //x,y,z,extra
    std::vector<float> speeds_; //vx,vy,vz,vs,va
    std::vector<float> decay_; //dx,dy,dz,dc,ds,da pos(speed), color(alpha), size (grow speed), angle (angular speed)
    std::vector<float> acceleration_; //dx,dy,dz,dc,ds,da pos(speed), color(alpha), size (grow speed), angle (angular speed)
    std::vector<float> ttl_; //time to live
    std::vector<std::string> tag_; //user tag
    VertexBuffer<float> texcoords_; //x,y,size,angle
    VertexBuffer<unsigned short> indices_;
    size_t particleCount;
    TextureBase *texture_[PARTICLES_MAX_TEXTURES];
    float sx_, sy_;

    float r_, g_, b_, a_;
    double lastTickTime_;

    mutable float minx_, miny_, maxx_, maxy_;
    bool boundsDirty_;
    bool paused_;
    bool autoSort;
};

#endif
