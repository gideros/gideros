#include <gmesh.h>
#include <ogl.h>
#include <color.h>

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
	minx_ = miny_ = 1e30;
	maxx_ = maxy_ = -1e30;
	application->addTicker(this);
}

Particles::~Particles() {
	application_->removeTicker(this);
	if (texture_)
		texture_->unref();
}

void Particles::clearParticles()
{
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
			s = k / 4;
			break;
		}
		k += 4;
	}
	if (s < 0) {
		s = ttl_.size();
		ttl_.resize(s + 1);
		points_.resize(s * 4 + 4);
		speeds_.resize(s * 4 + 4);
		colors_.resize(s * 4 + 4);
	}
	points_[s * 4 + 0] = x;
	points_[s * 4 + 1] = y;
	points_[s * 4 + 2] = size;
	points_[s * 4 + 3] = angle;
	ttl_[s] = ttl;
	speeds_[s * 4 + 0] = 0;
	speeds_[s * 4 + 1] = 0;
	speeds_[s * 4 + 2] = 0;
	speeds_[s * 4 + 3] = 0;
	colors_[s * 4 + 0] = 1;
	colors_[s * 4 + 1] = 1;
	colors_[s * 4 + 2] = 1;
	colors_[s * 4 + 3] = 1;
	points_.Update();
	speeds_.Update();
	colors_.Update();
	ttl_.Update();
	boundsDirty_ = true;
	return s;
}

void Particles::removeParticle(int i) {
	if (i > ttl_.size())
		return;
	points_[i * 4 + 2] = 0;
	points_.Update();
	boundsDirty_ = true;
}

void Particles::setColor(int i, unsigned int color, float alpha) {
	if (i > ttl_.size())
		return;
	originalColors_[i].color = color;
	originalColors_[i].alpha = alpha;
	alpha = std::min(std::max(alpha, 0.f), 1.f);

	unsigned int r = ((color >> 16) & 0xff);
	unsigned int g = ((color >> 8) & 0xff);
	unsigned int b = (color & 0xff);
	unsigned int a = 255 * alpha;

	colors_[i * 4] = r;
	colors_[i * 4 + 1] = g;
	colors_[i * 4 + 2] = b;
	colors_[i * 4 + 3] = a;
	colors_.Update();
}

void Particles::setSpeed(int i, float vx, float vy, float va, float decay) {
	if (i > ttl_.size())
		return;
	speeds_[i * 4] = vx;
	speeds_[i * 4 + 1] = vy;
	speeds_[i * 4 + 2] = va;
	speeds_[i * 4 + 3] = decay;
	speeds_.Update();
}

void Particles::getSpeed(int i, float *vx, float *vy, float *va,
		float *decay) const {
	if (i > ttl_.size()) {
		if (vx)
			*vx = 0;
		if (vy)
			*vy = 0;
		if (va)
			*va = 0;
		if (decay)
			*decay = 0;
		return;
	}
	if (vx)
		*vx = speeds_[i * 4];
	if (vy)
		*vy = speeds_[i * 4 + 1];
	if (va)
		*va = speeds_[i * 4 + 2];
	if (decay)
		*decay = speeds_[i * 4 + 3];
}

void Particles::tick() {
	for (int i = 0; i < ttl_.size(); i++) {
		if (points_[i * 4 + 2] != 0) {
			points_[i*4]+=speeds_[i*4];
			points_[i*4+1]+=speeds_[i*4+1];
			points_[i*4+3]+=speeds_[i*4+2];
			speeds_[i*4]*=speeds_[i*4+3];
			speeds_[i*4+1]*=speeds_[i*4+3];
			speeds_[i*4+2]*=speeds_[i*4+3];
			if (ttl_[i] > 0) {
				ttl_[i]--;
				if (!ttl_[i])
					removeParticle(i);
			}
		}
	}
	points_.Update();
	speeds_.Update();
	ttl_.Update();
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
	if (texture_)
		ShaderEngine::Engine->bindTexture(0, texture_->data->id());

	if (shader_)
		p = shader_;
	p->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 4, &points_[0],
			points_.size() / 4, points_.modified, &points_.bufferCache);
	points_.modified = false;

	p->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE, 4, &colors_[0],
			colors_.size() / 4, colors_.modified, &colors_.bufferCache);
	colors_.modified = false;

	p->drawArrays(ShaderProgram::Point, 0, ttl_.size());
}

void Particles::extraBounds(float *minx, float *miny, float *maxx,
		float *maxy) const {
	if (boundsDirty_) {
		minx_ = miny_ = 1e30;
		maxx_ = maxy_ = -1e30;

		for (size_t i = 0; i < ttl_.size(); i++) {
			if (points_[i * 4 + 2] != 0) {
				float x = points_[i * 4];
				float y = points_[i * 4 + 1];

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

