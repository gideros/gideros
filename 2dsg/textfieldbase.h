#ifndef TEXTFIELDBASE_H
#define TEXTFIELDBASE_H

#include "sprite.h"

#include <string>
#include "font.h"
#include <wchar32.h>

class Application;

class TextFieldBase : public Sprite
{
public:
    TextFieldBase(Application *application) : Sprite(application), layout_(),
        lscalex_(0),lscaley_(0),lfontCacheVersion_(-1),textlayout_(), prefWidth_(-1), prefHeight_(-1) {}
    void cloneFrom(TextFieldBase *);
    virtual void applyGhost(Sprite *parent,GhostSprite *);
    virtual ~TextFieldBase() {}

    virtual void setFont(FontBase* font) = 0;
    virtual FontBase *getFont() = 0;

	virtual void setText(const char* text) = 0;
	virtual const char* text() const = 0;

    virtual void setTextColor(float r,float g,float b,float a) = 0;
    virtual void textColor(float &r,float &g,float &b,float &a) = 0;

    virtual void setLetterSpacing(float letterSpacing) = 0;
    virtual float letterSpacing() const = 0;

    virtual float lineHeight() const = 0;

    virtual void setSample(const char* sample) = 0;
    virtual const char* sample() const = 0;

    virtual void setLayout(FontBase::TextLayoutParameters *l=NULL)=0;
    virtual FontBase::TextLayoutParameters getLayout() { return layout_; }

	bool scaleChanged();
    virtual bool setDimensions(float w,float h,bool forLayout=false);
    virtual void getDimensions(float &w,float &h);
    void getMinimumSize(float &w,float &h,bool preferred);
    bool optimizeSize(float &w,float &h);
    void getPointFromTextPos(size_t ri,float &cx,float &cy, int &cline);
    size_t getTextPosFromPoint(float &cx,float &cy);

protected:
	std::string text_;
    std::string sample_;
    FontBase::TextLayoutParameters layout_;
    float lscalex_,lscaley_;
    int lfontCacheVersion_;
    FontBase::TextLayout textlayout_; //Currently displayed text layout
    float prefWidth_,prefHeight_;
public:
    std::string styCache_color;
    std::string styCache_font;
    std::string styCache_text;
};

class GhostTextFieldBase : public GhostSprite {
public:
    GhostTextFieldBase(Sprite *m);
    virtual ~GhostTextFieldBase();
    std::string text;
};
#endif
