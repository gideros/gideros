#include "viewport.h"
#include "ogl.h"
#include "application.h"
#include "screen.h"

ScreenManager *ScreenManager::manager=NULL; //Move this elsewhere if screen API evolves
std::set<Screen *> Screen::screens;

Screen::Screen(Application *application) {
	application_=application;
	content_=NULL;
	clearA_=1; clearR_=0; clearG_=0; clearB_=0;
	screens.insert(this);
};

Screen::~Screen()
{
	screens.erase(screens.find(this));
	setContent((Sprite *)NULL);
};

void Screen::setContent(Sprite *s)
{
    if (s)
    	s->ref();
    if (content_)
        content_->unref();
    content_ = s;
    setVisible(content_!=NULL);
}

void Screen::clear(unsigned int color, float a)
{
    clearR_ = ((color >> 16) & 0xff) / 255.f;
    clearG_ = ((color >> 8) & 0xff) / 255.f;
    clearB_ = (color & 0xff) / 255.f;
    clearA_ = a;
}

void Screen::draw(const Matrix transform)
{
	int sw,sh;
	getSize(sw,sh);
    ShaderEngine::Engine->reset();
	ShaderEngine::Engine->setViewport(0, 0, sw,sh);
    Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0, sw,sh, 0, -1, 1,false);
	ShaderEngine::Engine->setProjection(projection);
	if (clearA_!=0)
		ShaderEngine::Engine->clearColor(clearR_ * clearA_, clearG_ * clearA_, clearB_ * clearA_, clearA_);
	if (content_)
		content_->draw(transform, 0, 0, sw,sh);
}

void ScreenManager::tick() {
	for (std::set<Screen *>::iterator it=Screen::screens.begin();it!=Screen::screens.end();it++)
		(*it)->tick();
}
