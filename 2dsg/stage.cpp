#include "stage.h"
#include "mouseevent.h"
#include "touchevent.h"
#include "enterframeevent.h"
#include "platform.h"
#include "keyboardevent.h"
#include <application.h>

#include <stack>

void Stage::mouseDown(int x, int y, int button, float sx, float sy, float tx, float ty)
{
    MouseEvent event(MouseEvent::MOUSE_DOWN, x, y, sx, sy, tx, ty);
    event.button = button;
    dispatchToSpritesWithListeners(&event);
}

void Stage::mouseUp(int x, int y, int button, float sx, float sy, float tx, float ty)
{
    MouseEvent event(MouseEvent::MOUSE_UP, x, y, sx, sy, tx, ty);
    event.button = button;
    dispatchToSpritesWithListeners(&event);
}

void Stage::mouseMove(int x, int y, int button, float sx, float sy, float tx, float ty)
{
    MouseEvent event(MouseEvent::MOUSE_MOVE, x, y, sx, sy, tx, ty);
    event.button = button;
    dispatchToSpritesWithListeners(&event);
}

void Stage::mouseHover(int x, int y, int button, float sx, float sy, float tx, float ty)
{
    MouseEvent event(MouseEvent::MOUSE_HOVER, x, y, sx, sy, tx, ty);
    event.button = button;
    dispatchToSpritesWithListeners(&event);
}

void Stage::mouseWheel(int x, int y, float sx, float sy, float tx, float ty, int wheel)
{
    MouseEvent event(MouseEvent::MOUSE_WHEEL, x, y, sx, sy, tx, ty);
    event.wheel=wheel;
    dispatchToSpritesWithListeners(&event);
}

void Stage::enterFrame(int deltaFrameCount, double lastFrameRenderTime)
{
    void *pool = application_->createAutounrefPool();

	double t = iclock();

	if (startTime_ < 0)
	{
		startTime_ = t;
		lastTime_ = t;
		lastFrameCount_ = 0;
	}

	double time = t - startTime_;
	double deltaTime = t - lastTime_;
	int frameCount = time * 60;
	
	if (deltaFrameCount < 0)
		deltaFrameCount = frameCount - lastFrameCount_;

	lastTime_ = t;
	lastFrameCount_ = frameCount;

	static std::vector<Sprite*> v;
	v.resize(allSpritesWithListeners_.size());
	{
		int i = 0;
		std::set<Sprite*>::iterator iter = allSpritesWithListeners_.begin(), end = allSpritesWithListeners_.end();
		for (; iter != end; ++iter, ++i)
			v[i] = *iter;
	}

	//printf("%d\n", v.size());

//	v.insert(v.end(), allSprites_.begin(), allSprites_.end());

	for (std::size_t i = 0; i < v.size(); ++i)
    {
		v[i]->ref();
        application_->autounref(v[i]);
    }

    EnterFrameEvent event(EnterFrameEvent::ENTER_FRAME, frameCount, deltaFrameCount, time, deltaTime, lastFrameRenderTime);
    for (std::size_t i = 0; i < v.size(); ++i)
	{
		Sprite* ptr = v[i];
		// Prevent garbage collected object. (otherwise crash)
		// Reference counter of proper object must be greater than 1. (because of above loop)
		// MovieClip occasionally made that crash, but other object types have possibility.
        if (1 < ptr->refCount())
        {
            ptr->dispatchEvent(&event);
        }
	}

    application_->deleteAutounrefPool(pool);
}

void Stage::touchesBegin(ginput_TouchEvent *event, float sx, float sy, float tx, float ty)
{
    TouchEvent event2(TouchEvent::TOUCHES_BEGIN, event, sx, sy, tx, ty);
    dispatchToSpritesWithListeners(&event2);
}

void Stage::touchesMove(ginput_TouchEvent *event, float sx, float sy, float tx, float ty)
{
    TouchEvent event2(TouchEvent::TOUCHES_MOVE, event, sx, sy, tx, ty);
    dispatchToSpritesWithListeners(&event2);
}

void Stage::touchesEnd(ginput_TouchEvent *event, float sx, float sy, float tx, float ty)
{
    TouchEvent event2(TouchEvent::TOUCHES_END, event, sx, sy, tx, ty);
    dispatchToSpritesWithListeners(&event2);
}

void Stage::touchesCancel(ginput_TouchEvent *event, float sx, float sy, float tx, float ty)
{
    TouchEvent event2(TouchEvent::TOUCHES_CANCEL, event, sx, sy, tx, ty);
    dispatchToSpritesWithListeners(&event2);
}

void Stage::keyDown(int keyCode, int realCode)
{
    KeyboardEvent event(KeyboardEvent::KEY_DOWN, keyCode, realCode,"");
    dispatchToSpritesWithListeners(&event);
}

void Stage::keyUp(int keyCode, int realCode)
{
    KeyboardEvent event(KeyboardEvent::KEY_UP, keyCode, realCode,"");
    dispatchToSpritesWithListeners(&event);
}

void Stage::keyChar(const char *code)
{
    KeyboardEvent event(KeyboardEvent::KEY_CHAR, 0, 0, code);
    dispatchToSpritesWithListeners(&event);
}

void Stage::populateSpritesWithListeners()
{
    spritesWithListeners_.clear();

    static std::stack<Sprite*> stack;

    stack.push(this);

    while (stack.empty() == false)
    {
        Sprite* sprite = stack.top();
        stack.pop();

        if (sprite->hasEventListener(MouseEvent::MOUSE_DOWN)     ||
            sprite->hasEventListener(MouseEvent::MOUSE_MOVE)     ||
            sprite->hasEventListener(MouseEvent::MOUSE_UP)       ||
            sprite->hasEventListener(MouseEvent::MOUSE_WHEEL)    ||
            sprite->hasEventListener(TouchEvent::TOUCHES_BEGIN)  ||
            sprite->hasEventListener(TouchEvent::TOUCHES_MOVE)   ||
            sprite->hasEventListener(TouchEvent::TOUCHES_END)    ||
            sprite->hasEventListener(TouchEvent::TOUCHES_CANCEL) ||
            sprite->hasEventListener(KeyboardEvent::KEY_DOWN)    ||
            sprite->hasEventListener(KeyboardEvent::KEY_CHAR)    ||
            sprite->hasEventListener(KeyboardEvent::KEY_UP))
        {
            spritesWithListeners_.push_back(sprite);
        }

        for (int i = sprite->childCount() - 1; i >= 0; --i)
            stack.push(sprite->child(i));
    }

    std::reverse(spritesWithListeners_.begin(), spritesWithListeners_.end());
}

void Stage::dispatchToSpritesWithListeners(Event *event)
{
    void *pool = application_->createAutounrefPool();

    if (isSpritesWithListenersDirty_)
    {
        populateSpritesWithListeners();
        isSpritesWithListenersDirty_ = false;
    }

    for (std::size_t i = 0; i < spritesWithListeners_.size(); ++i)
    {
        spritesWithListeners_[i]->ref();
        application_->autounref(spritesWithListeners_[i]);
    }

    for (std::size_t i = 0; i < spritesWithListeners_.size(); ++i)
    {
        if (event->propagationStopped())
            break;

        spritesWithListeners_[i]->dispatchEvent(event);
    }

    application_->deleteAutounrefPool(pool);
}

