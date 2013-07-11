#include "textfield.h"
#include "color.h"

TextField::TextField(Application *application) : TextFieldBase(application)
{
	font_ = 0;

	setTextColor(0x000000);

	letterSpacing_ = 0;

	createGraphics();
}

TextField::TextField(Application *application, BMFontBase* font) : TextFieldBase(application)
{
	font_ = font;
	if (font_ != 0)
		font_->ref();

	setTextColor(0x000000);

	letterSpacing_ = 0;

	createGraphics();
}

TextField::TextField(Application *application, BMFontBase* font, const char* text) : TextFieldBase(application)
{
	text_ = text;
	updateWide();

	font_ = font;
	if (font_ != 0)
		font_->ref();

	setTextColor(0x000000);

	letterSpacing_ = 0;

	createGraphics();
}

void TextField::setFont(Font* font)
{
	if (font_ == font)
		return;

	if (font != 0)
		font->ref();
	if (font_ != 0)
		font_->unref();
	font_ = font;

	createGraphics();
}

void TextField::setText(const char* text)
{
	if (strcmp(text, text_.c_str()) == 0)
		return;

	text_ = text;
	updateWide();

	createGraphics();
}

const char* TextField::text() const
{
	return text_.c_str();
}

void TextField::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
{
    if (minx)
        *minx = minx_;

    if (miny)
        *miny = miny_;

    if (maxx)
        *maxx = maxx_;

    if (maxy)
        *maxy = maxy_;
}

void TextField::setTextColor(unsigned int color)
{
	textColor_ = color;

	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	r_ = r / 255.f;
	g_ = g / 255.f;
	b_ = b / 255.f;

	createGraphics();
}

unsigned int TextField::textColor() const
{
	return textColor_;
}

void TextField::setLetterSpacing(float letterSpacing)
{
	letterSpacing_ = letterSpacing;
	
	createGraphics();
}

float TextField::letterSpacing() const
{
	return letterSpacing_;
}

void TextField::createGraphics()
{
    if (font_ == NULL)
        graphicsBase_.clear();
    else
        font_->drawText(&graphicsBase_, wtext_.c_str(), r_, g_, b_, letterSpacing_);

    graphicsBase_.getBounds(&minx_, &miny_, &maxx_, &maxy_);
}

void TextField::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
	graphicsBase_.draw();
}
