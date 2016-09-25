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
	void setProjection(const Matrix4* matrix = NULL);
	Sprite *getContent() { return content_; };
	Matrix4 getTransform() { return matrix_; };
	Matrix4 getProjection() { return projection_; };
	void lookAt(float eyex, float eyey, float eyez,
			float centerx, float centery, float centerz,
			float upx, float upy, float upz);
	void lookAngles(float eyex, float eyey, float eyez,float ax, float ay, float az);

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
    Sprite * content_;
	Matrix4 matrix_;
	Matrix4 projection_;
	bool hasProjection_;
};

#endif
