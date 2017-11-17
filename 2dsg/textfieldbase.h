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
    TextFieldBase(Application *application) : Sprite(application), layout_(), lscalex_(0),lscaley_(0) {}
    virtual ~TextFieldBase() {}

    virtual void setFont(FontBase* font) = 0;

	virtual void setText(const char* text) = 0;
	virtual const char* text() const = 0;

	virtual void setTextColor(unsigned int color) = 0;
	virtual unsigned int textColor() const = 0;

    virtual void setLetterSpacing(float letterSpacing) = 0;
    virtual float letterSpacing() const = 0;

    virtual float lineHeight() const = 0;

    virtual void setSample(const char* sample) = 0;
    virtual const char* sample() const = 0;

    virtual void setLayout(FontBase::TextLayoutParameters *l=NULL)=0;
    virtual FontBase::TextLayoutParameters getLayout() { return layout_; }

	bool scaleChanged();

protected:
	std::string text_;
    std::string sample_;
    FontBase::TextLayoutParameters layout_;
    float lscalex_,lscaley_;
};

#endif
