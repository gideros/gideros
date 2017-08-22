/*
 * window.h
 *
 *  Created on: 9 août 2017
 *      Author: Nicolas
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include "Matrices.h"
#include "refptr.h"
#include "ticker.h"

class Application;
class Sprite;

class Screen : public GReferenced {
	float clearA_,clearR_,clearG_,clearB_;
protected:
	Application *application_;
	Sprite *content_;
    void draw(const Matrix4 transform);
    virtual void setVisible(bool) {};
public:
    static std::set<Screen *> screens;
    enum State {
    	NORMAL=0,
    	MINIMIZED=1,
    	MAXIMIZED=2,
    	FULLSCREEN=4,
    	ACTIVE=8,
		HIDDEN=16,
		CLOSED=32
    };
	virtual void setSize(int w,int h) { G_UNUSED(w); G_UNUSED(h); };
	virtual void getSize(int &w,int &h) { w=0; h=0; };
	virtual void setPosition(int w,int h) { G_UNUSED(w); G_UNUSED(h); };
	virtual void getPosition(int &w,int &h) { w=0; h=0; };
	virtual void getMaxSize(int &w,int &h) { w=0; h=0; };
    void clear(unsigned int color, float a);
	virtual int getId()=0;
	virtual void setState(int state) { G_UNUSED(state); };
	virtual int getState() { return 0; };
	virtual void tick() {};
	Screen(Application *application);
	virtual ~Screen();
	void setContent(Sprite *content);
};

class ScreenManager {
public:
	static ScreenManager *manager;
	void tick();
	virtual Screen *openScreen(Application *application,int id)=0;
	virtual void screenDestroyed() {};
	virtual ~ScreenManager() { };
};



#endif /* SCREEN_H_ */
