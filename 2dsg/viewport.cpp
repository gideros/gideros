#include "viewport.h"
#include "ogl.h"
#include "application.h"

Viewport::Viewport(Application* application) : Sprite(application)
{
	content_ = NULL;
}

Viewport::~Viewport()
{
	setContent(NULL);
}

void Viewport::setContent(Sprite *s)
{
    if (s)
    	s->ref();
    if (content_)
        content_->unref();
    content_ = s;
}

void Viewport::setTransform(const Matrix4* matrix)
{
	if (matrix)
		matrix_=*matrix;
	else
		matrix_.identity();
}


void Viewport::doDraw(const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	if (content_)
	{
		((Sprite*)content_)->draw(t*matrix_, sx,sy,ex,ey);
	}
}
