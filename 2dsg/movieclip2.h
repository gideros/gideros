#ifndef MOVIECLIP_H
#define MOVIECLIP_H

#include "sprite.h"
#include <vector>
#include <map>
#include "enterframeevent.h"

class Application;

enum TweenType
{
	eEaseLinear = eStringIdLinear,
	eEaseInQuad,
	eEaseOutQuad,
	eEaseInOutQuad,
	eEaseOutInQuad,
	eEaseInCubic,
	eEaseOutCubic,
	eEaseInOutCubic,
	eEaseOutInCubic,
	eEaseInQuart,
	eEaseOutQuart,
	eEaseInOutQuart,
	eEaseOutInQuart,
	eEaseInQuint,
	eEaseOutQuint,
	eEaseInOutQuint,
	eEaseOutInQuint,
	eEaseInSine,
	eEaseOutSine,
	eEaseInOutSine,
	eEaseOutInSine,
	eEaseInExpo,
	eEaseOutExpo,
	eEaseInOutExpo,
	eEaseOutInExpo,
	eEaseInCirc,
	eEaseOutCirc,
	eEaseInOutCirc,
	eEaseOutInCirc,
	eEaseInElastic,
	eEaseOutElastic,
	eEaseInOutElastic,
	eEaseOutInElastic,
	eEaseInBack,
	eEaseOutBack,
	eEaseInOutBack,
	eEaseOutInBack,
	eEaseOutBounce,
	eEaseInBounce,
	eEaseInOutBounce,
	eEaseOutInBounce,
	eEaseInCurve,
	eEaseOutCurve,
	eEaseSineCurve,
	eEaseCosineCurve,
};

struct Parameter
{
	Parameter() {}

	Parameter(int param, float start, float end, TweenType tweenType = eEaseLinear);
	Parameter(const char* strparam, float start, float end, TweenType tweenType = eEaseLinear);
	Parameter(int param, float start, float end, const char* tweenType);
	Parameter(const char* strparam, float start, float end, const char* tweenType);

	int param;
	float start, end;
	double (*tweenFunction)(double);
};

class MovieClip : public Sprite
{
public:
    enum Type
    {
        eFrame,
        eTime
    };

    MovieClip(Type type, Application *application);
	virtual ~MovieClip();

	// start >= 1 && end >= 1 && start <= end
	void addFrame(int start, int end, Sprite* sprite, const std::vector<Parameter>& parameters, GStatus* status = NULL);

	void setStopAction(int frame);
	void setGotoAction(int frame, int destframe);
	void clearAction(int frame);

	void finalize();
	
	void play();
	void stop();
	void gotoAndPlay(int frame);
	void gotoAndStop(int frame);
	
private:
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;

private:
    void oneFrame();
    void nextFrame(EnterFrameEvent* event);
    void gotoFrame(int frame);
	void interpolateParameters();

    Type type_;
    double prevClock_;

	int frame_;
	int maxframe_;
	bool playing_;
	bool passoneframe_;

	struct Frame
	{
		int start;
		int end;
		Sprite* sprite;
		std::vector<Parameter> parameters;
	};

	std::vector<Frame> frames_;

	std::map<int, std::vector<Frame*> > allFrames_;			// indexed with start
	std::map<int, std::vector<Frame*> > activeFrames_;		// indexed with end

	std::map<int, int> actions_;

	std::map<Sprite*, int> counts_;
	void addChild2(Sprite* sprite);
	void removeChild2(Sprite* sprite);

	std::vector<Sprite*> currentSprites_;
};

#endif
