#include "viewport.h"
#include "ogl.h"
#include "application.h"

Viewport::Viewport(Application* application) : Sprite(application)
{
	content_ = NULL;
	hasProjection_=false;
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

void Viewport::setProjection(const Matrix4* matrix)
{
	if (matrix)
	{
		projection_=*matrix;
		hasProjection_=true;
	}
	else
	{
		projection_.identity();
		hasProjection_=false;
	}
}

void Viewport::doDraw(const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
	if (content_)
	{
		if (hasProjection_)
		{
			Matrix4 oldProj=ShaderEngine::Engine->getProjection();
			Matrix4 projection = ShaderEngine::Engine->getViewportProjection();
			int hw=application_->getHardwareWidth();
			int hh=application_->getHardwareHeight();
			float hs=application_->getScale();
			ShaderEngine::Engine->setViewport(0, 0, hw/hs, hh/hs);
			ShaderEngine::Engine->setProjection(projection_*projection);
			((Sprite*)content_)->draw(matrix_, sx,sy,ex,ey);
			ShaderEngine::Engine->setProjection(oldProj);
		}
		else
			((Sprite*)content_)->draw(t*matrix_, sx,sy,ex,ey);
	}
}
