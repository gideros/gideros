#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "sprite.h"

class Application;

class Viewport : public Sprite
{
public:
	Viewport(Application* application);
	virtual ~Viewport();
	void setContent(Sprite *s);
	void setTransform(const Matrix4* matrix = NULL);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
    Sprite * content_;
	Matrix4 matrix_;
};

#endif
