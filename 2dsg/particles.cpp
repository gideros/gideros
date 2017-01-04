#include <platform.h>
#include <particles.h>
#include <color.h>
#include <application.h>
#include <glog.h>
#include <ogl.h>

Particles::Particles(Application *application) :
		Sprite(application) {
	texture_ = NULL;
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
	application->addTicker(this);
}

Particles::~Particles() {
	application_->removeTicker(this);
	if (texture_)
		texture_->unref();
}

void Particles::clearParticles() {
	ttl_.clear();
	points_.clear();
	speeds_.clear();
	colors_.clear();
}

int Particles::addParticle(float x, float y, float size, float angle, int ttl) {
	int s = -1;
	int k = 2;
	while (k < points_.size()) {
		if (points_[k] == 0) {
			s = k / 16;
			break;
		}
		k += 16;
	}
	if (s < 0) {
		s = ttl_.size();
		ttl_.resize(s + 1);
		points_.resize(s * 16 + 16);
		colors_.resize(s * 16 + 16);
		texcoords_.resize(s * 8 + 8);
		speeds_.resize(s * 4 + 4);
		decay_.resize(s * 4 + 4);
		originalColors_.resize(s + 1);
		indices_.resize(s*6 + 6);
		tag_.resize(s + 1);
	}
	for (int sb = 0; sb < 16; sb += 4) {
		points_[s * 16 + sb + 0] = x;
		points_[s * 16 + sb + 1] = y;
		points_[s * 16 + sb + 2] = size;
		points_[s * 16 + sb + 3] = angle;
		colors_[s * 16 + sb + 0] = 255;
		colors_[s * 16 + sb + 1] = 255;
		colors_[s * 16 + sb + 2] = 255;
		colors_[s * 16 + sb + 3] = 255;
	}
	texcoords_[s * 8 + 0] = 0.0;
	texcoords_[s * 8 + 1] = 0.0;
	texcoords_[s * 8 + 2] = 1.0;
	texcoords_[s * 8 + 3] = 0.0;
	texcoords_[s * 8 + 4] = 1.0;
	texcoords_[s * 8 + 5] = 1.0;
	texcoords_[s * 8 + 6] = 0.0;
	texcoords_[s * 8 + 7] = 1.0;
	indices_[s * 6 + 0] = s * 4;
	indices_[s * 6 + 1] = s * 4 + 1;
	indices_[s * 6 + 2] = s * 4 + 2;
	indices_[s * 6 + 3] = s * 4 + 0;
	indices_[s * 6 + 4] = s * 4 + 2;
	indices_[s * 6 + 5] = s * 4 + 3;
	ttl_[s] = ttl;
	speeds_[s * 4 + 0] = 0;
	speeds_[s * 4 + 1] = 0;
	speeds_[s * 4 + 2] = 0;
	speeds_[s * 4 + 3] = 0;
	decay_[s * 4 + 0] = 1;
	decay_[s * 4 + 1] = 1;
	decay_[s * 4 + 2] = 1;
	decay_[s * 4 + 3] = 1;
	originalColors_[s].color = 0xFFFFFF;
	originalColors_[s].alpha = 1;
	tag_[s]="";
	points_.Update();
	colors_.Update();
	texcoords_.Update();
	indices_.Update();
	boundsDirty_ = true;
	return s;
}

void Particles::removeParticle(int i) {
	setSize(i, 0);
	boundsDirty_ = true;
}

void Particles::setPosition(int i, float x, float y) {
	if (i >= ttl_.size())
		return;
	points_[i * 16] = x;
	points_[i * 16 + 1] = y;
	points_[i * 16 + 4] = x;
	points_[i * 16 + 5] = y;
	points_[i * 16 + 8] = x;
	points_[i * 16 + 9] = y;
	points_[i * 16 + 12] = x;
	points_[i * 16 + 13] = y;
	points_.Update();
}

void Particles::getPosition(int i, float *x, float *y) {
	if (i >= ttl_.size()) {
		*x = 0;
		*y = 0;
	} else {
		*x = points_[i * 16 + 0];
		*y = points_[i * 16 + 1];
	}
}

void Particles::setSize(int i, float size) {
	if (i >= ttl_.size())
		return;
	points_[i * 16 + 2] = size;
	points_[i * 16 + 4 + 2] = size;
	points_[i * 16 + 8 + 2] = size;
	points_[i * 16 + 12 + 2] = size;
	points_.Update();
}

float Particles::getSize(int i) {
	if (i >= ttl_.size())
		return 0;
	else
		return points_[i * 16 + 2];
}

void Particles::setAngle(int i, float angle) {
	if (i >= ttl_.size())
		return;
	points_[i * 16 + 3] = angle;
	points_[i * 16 + 4 + 3] = angle;
	points_[i * 16 + 8 + 3] = angle;
	points_[i * 16 + 12 + 3] = angle;
	points_.Update();
}

float Particles::getAngle(int i) {
	if (i >= ttl_.size())
		return 0;
	else
		return points_[i * 16 + 3];
}

void Particles::setTtl(int i, int ttl) {
	if (i >= ttl_.size())
		return;
	ttl_[i] = ttl;
}

int Particles::getTtl(int i) {
	if (i >= ttl_.size())
		return 0;
	return ttl_[i];
}

void Particles::setColor(int i, unsigned int color, float alpha) {
	if (i >= ttl_.size())
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
}

void Particles::setSpeed(int i, float vx, float vy, float vs, float va) {
	if (i >= ttl_.size())
		return;
	speeds_[i * 4] = vx;
	speeds_[i * 4 + 1] = vy;
	speeds_[i * 4 + 2] = vs;
	speeds_[i * 4 + 3] = va;
}

void Particles::getSpeed(int i, float *vx, float *vy, float *vs,
		float *va) const {
	if (i >= ttl_.size()) {
		if (vx)
			*vx = 0;
		if (vy)
			*vy = 0;
		if (va)
			*va = 0;
		if (vs)
			*vs = 0;
		return;
	}
	if (vx)
		*vx = speeds_[i * 4];
	if (vy)
		*vy = speeds_[i * 4 + 1];
	if (vs)
		*vs = speeds_[i * 4 + 2];
	if (va)
		*va = speeds_[i * 4 + 3];
}

void Particles::setDecay(int i, float vp, float vc, float vs, float va) {
	if (i >= ttl_.size())
		return;
	decay_[i * 4] = vp;
	decay_[i * 4 + 1] = vc;
	decay_[i * 4 + 2] = vs;
	decay_[i * 4 + 3] = va;
}

void Particles::getDecay(int i, float *vp, float *vc, float *vs,
		float *va) const {
	if (i >= ttl_.size()) {
		if (vp)
			*vp = 0;
		if (vc)
			*vc = 0;
		if (vs)
			*vs = 0;
		if (va)
			*va = 0;
		return;
	}
	if (vp)
		*vp = decay_[i * 4];
	if (vc)
		*vc = decay_[i * 4 + 1];
	if (vs)
		*vs = decay_[i * 4 + 2];
	if (va)
		*va = decay_[i * 4 + 3];
}

void Particles::setTag(int i, const char *tag)
{
	if (i >= ttl_.size())
		return;
	tag_[i]=tag?tag:"";
}

const char *Particles::getTag(int i) const
{
	if (i >= ttl_.size())
		return NULL;
	return tag_[i].c_str();
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
	for (unsigned int i = 0; i < ttl_.size(); i++) {
		if (points_[i * 16 + 2] != 0) {
			bool remove=false;
			float nx = points_[i * 16] + speeds_[i * 4]*nframes;
			float ny = points_[i * 16 + 1] + speeds_[i * 4 + 1]*nframes;
			float ns = points_[i * 16 + 2] + speeds_[i * 4 + 2]*nframes;
			float na = points_[i * 16 + 3] + speeds_[i * 4 + 3]*nframes;
			if (abs(ns)<0.1)
				remove=true;
			for (int k = 0; k < 16; k += 4) {
				points_[i * 16 + k] = nx;
				points_[i * 16 + k + 1] = ny;
				points_[i * 16 + k + 2] = ns;
				points_[i * 16 + k + 3] = na;
			}
			points_.Update();
			speeds_[i * 4] *= pow(decay_[i * 4 + 0],nframes);
			speeds_[i * 4 + 1] *= pow(decay_[i * 4 + 0],nframes);
			speeds_[i * 4 + 2] *= pow(decay_[i * 4 + 2],nframes);
			speeds_[i * 4 + 3] *= pow(decay_[i * 4 + 3],nframes);
			if (decay_[i * 4 + 1]!=1) //alpha decay
			{
				int color=originalColors_[i].color;
				float alpha=originalColors_[i].alpha;
				alpha=alpha*pow(decay_[i * 4 + 1],nframes);
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
				removeParticle(i);
		}
	}
}

void Particles::getColor(int i, unsigned int *color, float *alpha) const {
	*color = originalColors_[i].color;
	*alpha = originalColors_[i].alpha;
}

void Particles::setTexture(TextureBase *texture) {
	if (texture)
		texture->ref();
	if (texture_)
		texture_->unref();
	texture_ = texture;

	float psx = sx_;
	float psy = sy_;

	if (texture_) {
		sx_ = texture_->uvscalex / texture_->data->exwidth;
		sy_ = texture_->uvscaley / texture_->data->exheight;
	} else {
		sx_ = 1;
		sy_ = 1;
	}
}

void Particles::clearTexture() {
	setTexture(NULL);
}

void Particles::doDraw(const CurrentTransform &, float sx, float sy, float ex,
		float ey) {
	if (ttl_.size() == 0)
		return;

	ShaderProgram *p = ShaderProgram::stdParticles;
	if (shader_)
		p = shader_;
	float textureInfo[4] = { 0, 0, 0, 0 };
	if (texture_) {
		ShaderEngine::Engine->bindTexture(0, texture_->data->id());
		textureInfo[0] = (float) texture_->data->width
				/ (float) texture_->data->exwidth;
		textureInfo[1] = (float) texture_->data->height
				/ (float) texture_->data->exheight;
		textureInfo[2] = 1.0 / texture_->data->exwidth;
		textureInfo[3] = 1.0 / texture_->data->exheight;
	}
	int sc = p->getSystemConstant(ShaderProgram::SysConst_TextureInfo);
	if (sc >= 0)
		p->setConstant(sc, ShaderProgram::CFLOAT4, 1, textureInfo);

	p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 4, &points_[0],
			points_.size() / 4, points_.modified, &points_.bufferCache);
	points_.modified = false;

	p->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
			&texcoords_[0], texcoords_.size() / 2, texcoords_.modified,
			&texcoords_.bufferCache);
	texcoords_.modified = false;

	p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4, &colors_[0],
			colors_.size() / 4, colors_.modified, &colors_.bufferCache);
	colors_.modified = false;

    p->drawElements(ShaderProgram::Triangles, indices_.size(), ShaderProgram::DUSHORT, &indices_[0],indices_.modified,&indices_.bufferCache);
    indices_.modified=false;
}

void Particles::extraBounds(float *minx, float *miny, float *maxx,
		float *maxy) const {
	if (boundsDirty_) {
		minx_ = miny_ = 1e30;
		maxx_ = maxy_ = -1e30;

		for (size_t i = 0; i < ttl_.size(); i++) {
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

