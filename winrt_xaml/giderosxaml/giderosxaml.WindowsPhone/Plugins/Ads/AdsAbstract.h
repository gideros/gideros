#ifndef ADSABSTRACT_H
#define ADSABSTRACT_H
typedef struct gads_Parameter;
class AdsAbstract
{
public:
	virtual void setKey(gads_Parameter *params) = 0;

	virtual void enableTesting() = 0;

	virtual void loadAd(gads_Parameter *params) = 0;

	virtual void showAd(gads_Parameter *params) = 0;

	virtual void hideAd(const char* type) = 0;

	virtual void setAlignment(const char *hor, const char *verr) = 0;

	virtual void setX(int x) = 0;

	virtual void setY(int y) = 0;

	virtual int getX() = 0;

	virtual int getY() = 0;

	virtual int getWidth() = 0;

	virtual int getHeight() = 0;

};
#endif