#ifndef TEXTFIELDBASE_H
#define TEXTFIELDBASE_H

#include "sprite.h"

#include <string>
#include <wchar32.h>

class Application;

class TextFieldBase : public Sprite
{
public:
    TextFieldBase(Application *application) : Sprite(application) {}
    virtual ~TextFieldBase() {}

	virtual void setText(const char* text) = 0;
	virtual const char* text() const = 0;

	virtual void setTextColor(unsigned int color) = 0;
	virtual unsigned int textColor() const = 0;

    virtual void setLetterSpacing(float letterSpacing) = 0;
    virtual float letterSpacing() const = 0;

protected:
	void updateWide();

	std::string text_;
    std::basic_string<wchar32_t> wtext_;
};

#endif
