#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include "sprite.h"

class Application;

class SplashScreen : public Sprite
{
public:
	SplashScreen(Application* application);
	virtual ~SplashScreen();

	bool isFinished() const;

private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

private:
	GraphicsBase graphicsBase1_;
	GraphicsBase graphicsBase2_;
	TextureData* data1_;
	TextureData* data2_;
	double startTime_;
};

#endif
