#include "tttextfield.h"
#include "ttfont.h"
#include "application.h"
#include "texturemanager.h"
#include "ogl.h"

TTTextField::TTTextField(Application* application, TTFont* font) : TextFieldBase(application)
{
	font_ = font;
	font_->ref();

	data_ = NULL;

	textColor_ = 0;

	letterSpacing_ = 0;

	graphicsBase_.getBounds(&minx_, &miny_, &maxx_, &maxy_);
}

TTTextField::TTTextField(Application* application, TTFont* font, const char* text) : TextFieldBase(application)
{
	font_ = font;
	font_->ref();

	data_ = NULL;

	text_ = text;
	updateWide();

	textColor_ = 0;

	letterSpacing_ = 0;

	createGraphics();
}

TTTextField::~TTTextField()
{
	if (data_)
	{
		application_->getTextureManager()->destroyTexture(data_);
		data_ = NULL;
	}

	font_->unref();
}

void TTTextField::createGraphics()
{
	if (data_)
	{
		application_->getTextureManager()->destroyTexture(data_);
		data_ = NULL;
	}

	if (wtext_.empty())
	{
		graphicsBase_.clear();
		graphicsBase_.getBounds(&minx_, &miny_, &maxx_, &maxy_);
		return;
	}

	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();

    int ascender = font_->getAscender();

	int minx, miny, maxx, maxy;
    Dib dib = font_->renderFont(wtext_.c_str(), letterSpacing_, &minx, &miny, &maxx, &maxy);

    int dx = minx - 1;
    int dy = miny - 1;

	TextureParameters parameters;
    if (font_->getSmoothing())
        parameters.filter = eLinear;
	data_ = application_->getTextureManager()->createTextureFromDib(dib, parameters);

	graphicsBase_.data = data_;

	graphicsBase_.mode = ShaderProgram::TriangleStrip;

	graphicsBase_.vertices.resize(4);
	graphicsBase_.vertices[0] = Point2f(dx / scalex,					dy / scaley);
	graphicsBase_.vertices[1] = Point2f((data_->width + dx) / scalex,	dy / scaley);
	graphicsBase_.vertices[2] = Point2f((data_->width + dx) / scalex,	(data_->height + dy) / scaley);
	graphicsBase_.vertices[3] = Point2f(dx / scalex,					(data_->height + dy) / scaley);

	float u = (float)data_->width / (float)data_->exwidth;
	float v = (float)data_->height / (float)data_->exheight;

	graphicsBase_.texcoords.resize(4);
	graphicsBase_.texcoords[0] = Point2f(0, 0);
	graphicsBase_.texcoords[1] = Point2f(u, 0);
	graphicsBase_.texcoords[2] = Point2f(u, v);
	graphicsBase_.texcoords[3] = Point2f(0, v);

	graphicsBase_.indices.resize(4);
	graphicsBase_.indices[0] = 0;
	graphicsBase_.indices[1] = 1;
	graphicsBase_.indices[2] = 3;
	graphicsBase_.indices[3] = 2;

	int r = (textColor_ >> 16) & 0xff;
	int g = (textColor_ >> 8) & 0xff;
	int b = textColor_ & 0xff;
	graphicsBase_.setColor(r / 255.f, g / 255.f, b / 255.f, 1);

    minx_ = minx / scalex;
    miny_ = miny / scaley;
    maxx_ = maxx / scalex;
    maxy_ = maxy / scaley;
}

void TTTextField::extraBounds(float* minx, float* miny, float* maxx, float* maxy) const
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

void TTTextField::doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey)
{
	graphicsBase_.draw(shader_);
}

void TTTextField::setText(const char* text)
{
	if (strcmp(text, text_.c_str()) == 0)
		return;

	text_ = text;
	updateWide();

	createGraphics();
}

const char* TTTextField::text() const
{
	return text_.c_str();
}

void TTTextField::setTextColor(unsigned int color)
{
	textColor_ = color;

	int r = (textColor_ >> 16) & 0xff;
	int g = (textColor_ >> 8) & 0xff;
	int b = textColor_ & 0xff;
	graphicsBase_.setColor(r / 255.f, g / 255.f, b / 255.f, 1);
}

unsigned int TTTextField::textColor() const
{
	return textColor_;
}

void TTTextField::setLetterSpacing(float letterSpacing)
{
	if (letterSpacing_ == letterSpacing)
		return;

	letterSpacing_ = letterSpacing;

	createGraphics();
}

float TTTextField::letterSpacing() const
{
	return letterSpacing_;
}

