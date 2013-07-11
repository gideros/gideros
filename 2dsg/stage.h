#ifndef STAGE_H
#define STAGE_H

#include "sprite.h"
#include "touch.h"
#include <set>
#include "touchevent.h"

class Application;

class Stage : public Sprite
{
public:
    Stage(Application* application) : Sprite(application)
	{
		startTime_ = -1;
        isSpritesWithListenersDirty_ = true;
	}

	void enterFrame(int deltaFrameCount = -1);

    void mouseDown(int x, int y, float sx, float sy, float tx, float ty);
    void mouseUp(int x, int y, float sx, float sy, float tx, float ty);
    void mouseMove(int x, int y, float sx, float sy, float tx, float ty);

    void touchesBegin(ginput_TouchEvent *event, float sx, float sy, float tx, float ty);
    void touchesMove(ginput_TouchEvent *event, float sx, float sy, float tx, float ty);
    void touchesEnd(ginput_TouchEvent *event, float sx, float sy, float tx, float ty);
    void touchesCancel(ginput_TouchEvent *event, float sx, float sy, float tx, float ty);

	void keyDown(int keyCode);
	void keyUp(int keyCode);

    Application* application() const
    {
        return application_;
    }

	virtual bool isStage() const
	{
		return true;
	}

    void setSpritesWithListenersDirty()
    {
        isSpritesWithListenersDirty_ = true;
    }

private:
	double startTime_;
	double lastTime_;
	int lastFrameCount_;

private:
    void populateSpritesWithListeners();
    std::vector<Sprite*> spritesWithListeners_;
    bool isSpritesWithListenersDirty_;
    void dispatchToSpritesWithListeners(Event *event);
};

#endif
