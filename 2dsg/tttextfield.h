#ifndef TTTEXTFIELD_H
#define TTTEXTFIELD_H

#include "textfieldbase.h"
#include "ttfont.h"

struct TextureData;
class Application;

class TTTextField : public TextFieldBase
{
public:
    TTTextField(Application* application, TTFont* font=NULL, const char* text=NULL, const char *sample=NULL, FontBase::TextLayoutParameters *layout=NULL);
    virtual Sprite *clone() { TTTextField *clone=new TTTextField(application_); clone->cloneFrom(this); return clone; }
    void cloneFrom(TTTextField *);
    virtual ~TTTextField();

    virtual void setFont(FontBase *font);
    FontBase *getFont() { return font_; };

	virtual void setText(const char* text);
	virtual const char* text() const;

    virtual void setTextColor(float r,float g,float b,float a);
    virtual void textColor(float &r,float &g,float &b,float &a);

    virtual void setLetterSpacing(float letterSpacing);
    virtual float letterSpacing() const;

    virtual float lineHeight() const;

    virtual void setSample(const char* sample);
    virtual const char* sample() const;

    virtual void setLayout(FontBase::TextLayoutParameters *l=NULL);

private:
	void createGraphics();

	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

	TextureData* data_;
	TTFont* font_;
	GraphicsBase graphicsBase_;
    float a_, r_, g_, b_;
    float minx_, miny_, maxx_, maxy_;
    int sminx, sminy, smaxx, smaxy;
    int styleFlags_;
};


#endif
