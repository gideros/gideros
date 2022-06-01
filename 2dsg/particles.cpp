#include <platform.h>
#include <particles.h>
#include <color.h>
#include <application.h>
#include <glog.h>
#include <ogl.h>

Particles::Particles(Application *application, bool is3d, bool autosort) :
		Sprite(application) {
    for (int t=0;t<PARTICLES_MAX_TEXTURES;t++)
        texture_[t]=NULL;
    sx_ = 1;
    sy_ = 1;
    r_ = 1;
    g_ = 1;
	b_ = 1;
	a_ = 1;
	boundsDirty_ = false;
	paused_ = false;
	minx_ = miny_ = 1e30;
	maxx_ = maxy_ = -1e30;
	lastTickTime_=0;
	particleCount=0;
	application->addTicker(this);
    this->is3d=is3d;
    autoSort=autosort;
}

void Particles::cloneFrom(Particles *s)
{
    Sprite::cloneFrom(s);

    r_ = s->r_, g_ = s->g_, b_ = s->b_, a_ = s->a_;
    sx_ = s->sx_, sy_ = s->sy_;
    boundsDirty_=s->boundsDirty_;
    paused_=s->paused_;
    minx_ = s->minx_;
    miny_ = s->miny_;
    maxx_ = s->maxx_;
    maxy_ = s->maxy_;
    lastTickTime_=s->lastTickTime_;
    particleCount=s->particleCount;
    for (int t=0;t<PARTICLES_MAX_TEXTURES;t++)
        if ((texture_[t]=s->texture_[t])!=NULL)
                texture_[t]->ref();

    originalColors_=s->originalColors_;
    colors_.assign(s->colors_.cbegin(),s->colors_.cend());
    colors_.Update();
    points_.assign(s->points_.cbegin(),s->points_.cend());
    points_.Update();
    speeds_=s->speeds_;
    decay_=s->decay_;
    acceleration_=s->acceleration_;
    ttl_=s->ttl_;
    tag_=s->tag_;
    dead_=s->dead_;
    texcoords_.assign(s->texcoords_.cbegin(),s->texcoords_.cend());
    texcoords_.Update();
    indices_.assign(s->indices_.cbegin(),s->indices_.cend());
    indices_.Update();
    is3d=s->is3d;
    autoSort=s->autoSort;
}

Particles::~Particles() {
	application_->removeTicker(this);
    for (int t=0;t<PARTICLES_MAX_TEXTURES;t++)
        if (texture_[t])
            texture_[t]->unref();
}

void Particles::clearParticles() {
	particleCount=0;
    dead_.clear();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

int Particles::addParticle(float x, float y, float z, float size, float angle, int ttl, float extra) {
	int s = -1;
	int k = 2;
	int kmax=particleCount*16;
	while (k < kmax) {
        if (texcoords_[k] == 0) {
			s = k / 16;
			break;
		}
		k += 16;
	}
	if (s < 0) {
        size_t tsize=ttl_.size();
		if (tsize==particleCount) {
			//Not enough room in vectors: double if large enough or kick start
			tsize+=tsize?tsize:16;
			ttl_.resize(tsize);
			points_.resize(tsize * 16);
			colors_.resize(tsize * 16);
            texcoords_.resize(tsize * 16);
            speeds_.resize(tsize * 5);
            decay_.resize(tsize * 6);
            acceleration_.resize(tsize * 6);
            originalColors_.resize(tsize);
			indices_.resize(tsize * 6);
			tag_.resize(tsize);
		}
		s=particleCount++;
	}
	for (int sb = 0; sb < 16; sb += 4) {
		points_[s * 16 + sb + 0] = x;
		points_[s * 16 + sb + 1] = y;
        points_[s * 16 + sb + 2] = z;
        points_[s * 16 + sb + 3] = extra;
        texcoords_[s * 16 + sb + 2] = size;
        texcoords_[s * 16 + sb + 3] = angle;
        colors_[s * 16 + sb + 0] = 255;
		colors_[s * 16 + sb + 1] = 255;
		colors_[s * 16 + sb + 2] = 255;
		colors_[s * 16 + sb + 3] = 255;
	}
    texcoords_[s * 16 + 0] = 0.0;
    texcoords_[s * 16 + 1] = 0.0;
    texcoords_[s * 16 + 4] = 1.0;
    texcoords_[s * 16 + 5] = 0.0;
    texcoords_[s * 16 + 8] = 1.0;
    texcoords_[s * 16 + 9] = 1.0;
    texcoords_[s * 16 + 12] = 0.0;
    texcoords_[s * 16 + 13] = 1.0;
	indices_[s * 6 + 0] = s * 4;
	indices_[s * 6 + 1] = s * 4 + 1;
	indices_[s * 6 + 2] = s * 4 + 2;
	indices_[s * 6 + 3] = s * 4 + 0;
	indices_[s * 6 + 4] = s * 4 + 2;
	indices_[s * 6 + 5] = s * 4 + 3;
	ttl_[s] = ttl;
    speeds_[s * 5 + 0] = 0;
    speeds_[s * 5 + 1] = 0;
    speeds_[s * 5 + 2] = 0;
    speeds_[s * 5 + 3] = 0;
    speeds_[s * 5 + 4] = 0;
    decay_[s * 6 + 0] = 1;
    decay_[s * 6 + 1] = 1;
    decay_[s * 6 + 2] = 1;
    decay_[s * 6 + 3] = 1;
    decay_[s * 6 + 4] = 1;
    decay_[s * 6 + 5] = 1;
    acceleration_[s * 6 + 0] = 0;
    acceleration_[s * 6 + 1] = 0;
    acceleration_[s * 6 + 2] = 0;
    acceleration_[s * 6 + 3] = 0;
    acceleration_[s * 6 + 4] = 0;
    acceleration_[s * 6 + 5] = 0;
    originalColors_[s].color = 0xFFFFFF;
	originalColors_[s].alpha = 1;
	tag_[s]="";
	points_.Update();
	colors_.Update();
	texcoords_.Update();
	indices_.Update();
	boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
	return s;
}

void Particles::removeParticle(int i) {
	setSize(i, 0);
	boundsDirty_ = true;
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Particles::setPosition(int i, float x, float y, float z) {
    if (i >= (int)particleCount)
		return;
	points_[i * 16] = x;
    points_[i * 16 + 1] = y;
    points_[i * 16 + 2] = z;
    points_[i * 16 + 4] = x;
	points_[i * 16 + 5] = y;
    points_[i * 16 + 6] = z;
    points_[i * 16 + 8] = x;
	points_[i * 16 + 9] = y;
    points_[i * 16 + 10] = z;
    points_[i * 16 + 12] = x;
	points_[i * 16 + 13] = y;
    points_[i * 16 + 14] = z;
    points_.Update();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Particles::getPosition(int i, float *x, float *y, float *z) {
    if (i >= (int)particleCount) {
		*x = 0;
		*y = 0;
        *z = 0;
	} else {
		*x = points_[i * 16 + 0];
        *y = points_[i * 16 + 1];
        *z = points_[i * 16 + 2];
    }
}

void Particles::setSize(int i, float size) {
    if (i >= (int)particleCount)
		return;
    texcoords_[i * 16 + 2] = size;
    texcoords_[i * 16 + 4 + 2] = size;
    texcoords_[i * 16 + 8 + 2] = size;
    texcoords_[i * 16 + 12 + 2] = size;
    texcoords_.Update();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

void Particles::scaleParticles(float size,bool absolute) {
	for (size_t i=0;i<(particleCount*4);i++) {
        if (texcoords_[i*4+2]!=0) {
			if (absolute)
                texcoords_[i*4+2]=size;
			else
                texcoords_[i*4+2]*=size;
		}
	}
    texcoords_.Update();
	invalidate(INV_GRAPHICS|INV_BOUNDS);
}

float Particles::getSize(int i) {
    if (i >= (int)particleCount)
		return 0;
	else
        return texcoords_[i * 16 + 2];
}

void Particles::setAngle(int i, float angle) {
    if (i >= (int)particleCount)
		return;
    texcoords_[i * 16 + 3] = angle;
    texcoords_[i * 16 + 4 + 3] = angle;
    texcoords_[i * 16 + 8 + 3] = angle;
    texcoords_[i * 16 + 12 + 3] = angle;
    texcoords_.Update();
	invalidate(INV_GRAPHICS);
}

float Particles::getAngle(int i) {
    if (i >= (int)particleCount)
		return 0;
	else
        return texcoords_[i * 16 + 3];
}

void Particles::setTtl(int i, int ttl) {
    if (i >= (int)particleCount)
		return;
	ttl_[i] = ttl;
}

int Particles::getTtl(int i) {
    if (i >= (int)particleCount)
		return 0;
	return ttl_[i];
}

void Particles::setColor(int i, unsigned int color, float alpha) {
    if (i >= (int)particleCount)
		return;
	originalColors_[i].color = color;
	originalColors_[i].alpha = alpha;
	alpha = std::min(std::max(alpha, 0.f), 1.f);

	unsigned int r = ((color >> 16) & 0xff) * alpha;
	unsigned int g = ((color >> 8) & 0xff) * alpha;
	unsigned int b = (color & 0xff) * alpha;
	unsigned int a = 255 * alpha;

	for (int k = 0; k < 16; k += 4) {
		colors_[i * 16 + k] = r;
		colors_[i * 16 + k + 1] = g;
		colors_[i * 16 + k + 2] = b;
		colors_[i * 16 + k + 3] = a;
	}
	colors_.Update();
	invalidate(INV_GRAPHICS);
}

void Particles::setSpeed(int i, float vx, float vy, float vz, float vs, float va) {
    if (i >= (int)particleCount)
		return;
    speeds_[i * 5] = vx;
    speeds_[i * 5 + 1] = vy;
    speeds_[i * 5 + 2] = vz;
    speeds_[i * 5 + 3] = vs;
    speeds_[i * 5 + 4] = va;
}

void Particles::getSpeed(int i, float *vx, float *vy, float *vz, float *vs,
		float *va) const {
    if (i >= (int)particleCount) {
		if (vx)
			*vx = 0;
        if (vy)
            *vy = 0;
        if (vz)
            *vz = 0;
        if (va)
			*va = 0;
		if (vs)
			*vs = 0;
		return;
	}
	if (vx)
        *vx = speeds_[i * 5];
	if (vy)
        *vy = speeds_[i * 5 + 1];
    if (vz)
        *vz = speeds_[i * 5 + 2];
    if (vs)
        *vs = speeds_[i * 5 + 3];
    if (va)
        *va = speeds_[i * 5 + 4];
}

void Particles::setDecay(int i, float vx, float vy, float vz, float vc, float vs, float va) {
    if (i >= (int)particleCount)
        return;
    decay_[i * 6] = vx;
    decay_[i * 6 + 1] = vy;
    decay_[i * 6 + 2] = vz;
    decay_[i * 6 + 3] = vc;
    decay_[i * 6 + 4] = vs;
    decay_[i * 6 + 5] = va;
}

void Particles::getDecay(int i, float *vx, float *vy, float *vz, float *vc, float *vs,
        float *va) const {
    if (i >= (int)particleCount) {
        if (vx)
            *vx = 0;
        if (vy)
            *vy = 0;
        if (vz)
            *vz = 0;
        if (vc)
            *vc = 0;
        if (vs)
            *vs = 0;
        if (va)
            *va = 0;
        return;
    }
    if (vy)
        *vx = decay_[i * 6];
    if (vy)
        *vy = decay_[i * 6 + 1];
    if (vz)
        *vz = decay_[i * 6 + 2];
    if (vc)
        *vc = decay_[i * 6 + 3];
    if (vs)
        *vs = decay_[i * 6 + 4];
    if (va)
        *va = decay_[i * 6 + 5];
}

void Particles::setAcceleration(int i, float vx, float vy, float vz, float vc, float vs, float va) {
    if (i >= (int)particleCount)
        return;
    acceleration_[i * 6] = vx;
    acceleration_[i * 6 + 1] = vy;
    acceleration_[i * 6 + 2] = vz;
    acceleration_[i * 6 + 3] = vc;
    acceleration_[i * 6 + 4] = vs;
    acceleration_[i * 6 + 5] = va;
}

void Particles::getAcceleration(int i, float *vx, float *vy, float *vz, float *vc, float *vs,
        float *va) const {
    if (i >= (int)particleCount) {
        if (vx)
            *vx = 0;
        if (vy)
            *vy = 0;
        if (vz)
            *vz = 0;
        if (vc)
            *vc = 0;
        if (vs)
            *vs = 0;
        if (va)
            *va = 0;
        return;
    }
    if (vy)
        *vx = acceleration_[i * 6];
    if (vy)
        *vy = acceleration_[i * 6 + 1];
    if (vz)
        *vz = acceleration_[i * 6 + 2];
    if (vc)
        *vc = acceleration_[i * 6 + 3];
    if (vs)
        *vs = acceleration_[i * 6 + 4];
    if (va)
        *va = acceleration_[i * 6 + 5];
}

void Particles::setTag(int i, const char *tag)
{
    if (i >= (int)particleCount)
        return;
    tag_[i]=tag?tag:"";
}

const char *Particles::getTag(int i) const
{
    if (i >= (int)particleCount)
        return NULL;
    return tag_[i].c_str();
}

void Particles::setExtra(int i, float extra)
{
    if (i >= (int)particleCount)
        return;
    points_[i * 16 + 3] = extra;
    points_[i * 16 + 7] = extra;
    points_[i * 16 + 11] = extra;
    points_[i * 16 + 15] = extra;
    points_.Update();
    invalidate(INV_GRAPHICS|INV_BOUNDS);
}

float Particles::getExtra(int i) const
{
    if (i >= (int)particleCount)
        return NULL;
    return points_[i * 16 + 3];
}

extern "C" {
int g_getFps();
}


void Particles::tick() {
	float nframes=1;
	double iclk=application_->getTimerContainer()->getTimer();
	if (lastTickTime_>0)
	{
		int fps=g_getFps();
		if (!fps)
			fps=60;
		else fps=(fps<0)?-fps:fps;
		nframes=(iclk-lastTickTime_)*fps;
	}
	lastTickTime_=iclk;
	if (paused_) return;

    for (auto it=dead_.begin();it!=dead_.cend();it++)
        removeParticle(*it);
    dead_.clear();

	int changes=INV_GRAPHICS;
	for (size_t i = 0; i < particleCount; i++) {
        if (texcoords_[i * 16 + 2] != 0) {
			bool remove=false;
            if (nframes&&(speeds_[i * 5]||speeds_[i * 5 + 1]||speeds_[i * 5 + 2])) changes|=INV_BOUNDS;
            float nx = points_[i * 16] + speeds_[i * 5]*nframes;
            float ny = points_[i * 16 + 1] + speeds_[i * 5 + 1]*nframes;
            float nz = points_[i * 16 + 2] + speeds_[i * 5 + 2]*nframes;
            float ns = texcoords_[i * 16 + 2] + speeds_[i * 5 + 3]*nframes;
            float na = texcoords_[i * 16 + 3] + speeds_[i * 5 + 4]*nframes;
            if ((ttl_[i]<=0)&&(fabs(ns)<0.1))
				remove=true;
			for (int k = 0; k < 16; k += 4) {
				points_[i * 16 + k] = nx;
                points_[i * 16 + k + 1] = ny;
                points_[i * 16 + k + 2] = nz;
                texcoords_[i * 16 + k + 2] = ns;
                texcoords_[i * 16 + k + 3] = na;
			}
            points_.Update();
            texcoords_.Update();
            speeds_[i * 5] *= pow(decay_[i * 6 + 0],nframes);
            speeds_[i * 5 + 1] *= pow(decay_[i * 6 + 1],nframes);
            speeds_[i * 5 + 2] *= pow(decay_[i * 6 + 2],nframes);
            speeds_[i * 5 + 3] *= pow(decay_[i * 6 + 4],nframes);
            speeds_[i * 5 + 4] *= pow(decay_[i * 6 + 5],nframes);
            speeds_[i * 5] += acceleration_[i * 6 + 0]*nframes;
            speeds_[i * 5 + 1] += acceleration_[i * 6 + 1]*nframes;
            speeds_[i * 5 + 2] += acceleration_[i * 6 + 2]*nframes;
            speeds_[i * 5 + 3] += acceleration_[i * 6 + 4]*nframes;
            speeds_[i * 5 + 4] += acceleration_[i * 6 + 5]*nframes;
            if ((decay_[i * 6 + 3]!=1)||(acceleration_[i * 6 + 3])) //alpha decay
			{
				int color=originalColors_[i].color;
				float alpha=originalColors_[i].alpha;
                alpha=alpha*pow(decay_[i * 6 + 3],nframes);
                alpha += acceleration_[i * 6 + 3]*nframes;
                originalColors_[i].alpha=alpha;
				alpha = std::min(std::max(alpha, 0.f), 1.f);

				unsigned int r = ((color >> 16) & 0xff) * alpha;
				unsigned int g = ((color >> 8) & 0xff) * alpha;
				unsigned int b = (color & 0xff) * alpha;
				unsigned int a = 255 * alpha;

				for (int k = 0; k < 16; k += 4) {
					colors_[i * 16 + k] = r;
					colors_[i * 16 + k + 1] = g;
					colors_[i * 16 + k + 2] = b;
					colors_[i * 16 + k + 3] = a;
				}
				colors_.Update();
			}
			if (ttl_[i] > 0) {
				ttl_[i]=ttl_[i]-nframes;
				if (ttl_[i]<=0)
				{
					ttl_[i]=0;
					remove=true;
				}
			}
			if (remove)
                dead_.insert(i);
		}
	}
	invalidate(changes);
}

std::set<int> Particles::getDead() {
    return dead_;
}

void Particles::getColor(int i, unsigned int *color, float *alpha) const {
	*color = originalColors_[i].color;
	*alpha = originalColors_[i].alpha;
}

void Particles::setTexture(TextureBase *texture,int slot) {
    if (texture)
        texture->ref();
    if (texture_[slot])
        texture_[slot]->unref();
    texture_[slot] = texture;

    if (slot==0)
    {
        if (texture) {
            sx_ = texture->uvscalex / texture->data->exwidth;
            sy_ = texture->uvscaley / texture->data->exheight;
        } else {
            sx_ = 1;
            sy_ = 1;
        }
    }
	invalidate(INV_GRAPHICS);
}

void Particles::clearTexture(int slot) {
    setTexture(NULL,slot);
}

void Particles::doDraw(const CurrentTransform &, float sx, float sy, float ex,
		float ey) {
    G_UNUSED(sx); G_UNUSED(sy); G_UNUSED(ex); G_UNUSED(ey);
    if (!particleCount)
    	return;
    if (ttl_.size() == 0)
		return;

    for (int t=0;t<PARTICLES_MAX_TEXTURES;t++)
        if (texture_[t])
            ShaderEngine::Engine->bindTexture(t,texture_[t]->data->id());
    ShaderProgram *p = getShader(ShaderEngine::STDP_PARTICLES,(texture_[0]?ShaderEngine::STDPV_TEXTURED:0)|(is3d?ShaderEngine::STDPV_3D:0));
    if (is3d&&autoSort) {
        //Sort particles according to distance (nearest last)
        Matrix4 vm=ShaderEngine::Engine->getView();
        Matrix4 mm=ShaderEngine::Engine->getModel();
        Matrix4 tv=vm*mm;
        float cx=0,cy=0,cz=0;
        tv.inverseTransformPoint(cx,cy,cz,&cx,&cy,&cz);
        size_t ss=ttl_.size();
        int *idx=new int[ss];
        float *dist=new float[ss];
        for (size_t s=0;s<ss;s++) {
            idx[s]=s;
            float dx=points_[s * 16 + 0]-cx;
            float dy=points_[s * 16 + 1]-cy;
            float dz=points_[s * 16 + 2]-cz;
            dist[s]=sqrtf(dx*dx+dy*dy+dz*dz);
        }
        std::sort(idx,idx+ss,[&](const int & a, const int & b) -> bool
        {
            return dist[a] > dist[b];
        });

        for (size_t s=0;s<ss;s++) {
            int sd=idx[s];
            indices_[s * 6 + 0] = sd * 4;
            indices_[s * 6 + 1] = sd * 4 + 1;
            indices_[s * 6 + 2] = sd * 4 + 2;
            indices_[s * 6 + 3] = sd * 4 + 0;
            indices_[s * 6 + 4] = sd * 4 + 2;
            indices_[s * 6 + 5] = sd * 4 + 3;
        }
        indices_.Update();
        delete[] idx;
        delete[] dist;
    }

	float textureInfo[4] = { 0, 0, 0, 0 };
    if (texture_[0]) {
        textureInfo[0]=(float)texture_[0]->data->width / (float)texture_[0]->data->exwidth;
        textureInfo[1]=(float)texture_[0]->data->height / (float)texture_[0]->data->exheight;
        textureInfo[2]=1.0/texture_[0]->data->exwidth;
        textureInfo[3]=1.0/texture_[0]->data->exheight;
	}
	int sc = p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
	if (sc >= 0)
		p->setConstant(sc, ShaderProgram::CFLOAT4, 1, textureInfo);

	p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 4, &points_[0],
			points_.size() / 4, points_.modified, &points_.bufferCache);
	points_.modified = false;

    p->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 4,
            &texcoords_[0], texcoords_.size() / 4, texcoords_.modified,
			&texcoords_.bufferCache);
	texcoords_.modified = false;

	p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4, &colors_[0],
			colors_.size() / 4, colors_.modified, &colors_.bufferCache);
	colors_.modified = false;

    p->drawElements(ShaderProgram::Triangles, ttl_.size()*6, ShaderProgram::DUSHORT, &indices_[0],indices_.modified,&indices_.bufferCache);
    indices_.modified=false;
}

void Particles::extraBounds(float *minx, float *miny, float *maxx,
		float *maxy) const {
	if (boundsDirty_) {
		minx_ = miny_ = 1e30;
		maxx_ = maxy_ = -1e30;

		for (size_t i = 0; i < particleCount; i++) {
			if (points_[i * 16 + 2] != 0) {
				float x = points_[i * 16];
				float y = points_[i * 16 + 1];

				minx_ = std::min(minx_, x);
				miny_ = std::min(miny_, y);
				maxx_ = std::max(maxx_, x);
				maxy_ = std::max(maxy_, y);
			}
		}
	}

	if (minx)
		*minx = minx_;
	if (miny)
		*miny = miny_;
	if (maxx)
		*maxx = maxx_;
	if (maxy)
		*maxy = maxy_;
}

