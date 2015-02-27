#ifndef APPLICATION_H
#define APPLICATION_H

#include "touch.h"
#include <set>
#include "orientation.h"
#include "texturemanager.h"
#include <math.h>
#include "ticker.h"
#include <vector>
#include <string>
#include <ginput.h>
#include <timercontainer.h>
#include <refptr.h>


class Font;
class Stage;
class Event;

class Application
{
public:
	Application();
	~Application();

	Stage* stage() const;
	
	void setOrientation(Orientation orientation);
	Orientation orientation() const;

	void setHardwareOrientation(Orientation orientation);
	Orientation hardwareOrientation() const;

    void setDeviceOrientation(Orientation orientation);
    Orientation getDeviceOrientation() const;
	
	void setResolution(int width, int height);
	void resolution(int* width, int* height);

	void initView();
	void releaseView();

	void enterFrame();
	void clearBuffers();
	void renderScene(int deltaFrameCount = -1);

	void mouseDown(int x, int y);
	void mouseUp(int x, int y);
	void mouseMove(int x, int y);

    void touchesBegin(ginput_TouchEvent *event);
    void touchesMove(ginput_TouchEvent *event);
    void touchesEnd(ginput_TouchEvent *event);
    void touchesCancel(ginput_TouchEvent *event);

	void keyDown(int keyCode);
	void keyUp(int keyCode);

	void setClearColorBuffer(bool b)
	{
		clearColorBuffer_ = b;
	}

	bool getClearColorBuffer() const
	{
		return clearColorBuffer_;
	}

	void broadcastEvent(Event* event);

	Font* getDefaultFont();

	void setScale(int scale)
	{
		scale_ = scale;
	}

	void setLogicalDimensions(int width, int height);

    void setLogicalScaleMode(LogicalScaleMode mode);
    LogicalScaleMode getLogicalScaleMode() const;

	int getLogicalWidth() const;
	int getLogicalHeight() const;
	int getHardwareWidth() const;
	int getHardwareHeight() const;

	void setImageScales(const std::vector<std::pair<std::string, float> >& imageScales);
	const std::vector<std::pair<std::string, float> >& getImageScales() const;

    const char *getImageSuffix(float *scale) const;

    const char *getImageSuffix(const char *file, float *scale) const;

	void addTicker(Ticker* ticker);
	void removeTicker(Ticker* ticker);

	void setBackgroundColor(float r, float g, float b);
	void getBackgroundColor(float* r, float* g, float* b);

	float getLogicalTranslateX() const;
	float getLogicalTranslateY() const;
	float getLogicalScaleX() const;
	float getLogicalScaleY() const;

	TextureManager* getTextureManager()
	{
		return &textureManager_;
	}

    TimerContainer *getTimerContainer()
    {
        return &timerContainer_;
    }

    void *createAutounrefPool();
    void deleteAutounrefPool(void *);
    void autounref(GReferenced *referenced);

    void setFieldOfView(float fov);

private:
	TextureManager textureManager_;
	Stage* stage_;
	Orientation orientation_;
	Orientation hardwareOrientation_;
	Orientation deviceOrientation_;
	int width_, height_;

	int logicalWidth_, logicalHeight_;
	LogicalScaleMode scaleMode_;
	float logicalScaleX_, logicalScaleY_;
	float logicalTranslateX_, logicalTranslateY_;
	float fov_;
	void calculateLogicalTransformation();
	void correctTouchPositionLogical(int* x, int* y);

	void correctTouchPosition(int* x, int* y);
	void correctTouchPositionHardware(int* x, int *y);
    void correctTouchPositions(ginput_TouchEvent *event);

	int nframe_;
	double time_;

	bool clearColorBuffer_;

	Font* defaultFont_;

	int scale_;

    std::vector<std::pair<std::string, float> > imageScales_;

    struct ImageScale
    {
        ImageScale(const char *suffix, float scale) :
            suffix(suffix),
            scale(scale),
            midscale(0)
        {
        }

        const char *suffix;
        float scale;
        float midscale;

        bool operator < (const ImageScale &other) const
        {
            return midscale > other.midscale;
        }
    };

    std::vector<ImageScale> imageScales2_;

	std::set<Ticker*> tickers_;
	bool tickersIteratorInvalid_;

	float backr_, backg_, backb_;

    TimerContainer timerContainer_;

    std::vector<std::vector<GReferenced*>*> unrefPool_;
    std::vector<std::vector<GReferenced*>*> unrefPoolTrash_;
};

#endif
