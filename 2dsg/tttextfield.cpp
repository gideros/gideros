#include "tttextfield.h"
#include "ttfont.h"
#include "application.h"
#include "texturemanager.h"
#include "ogl.h"
#include <utf8.h>

TTTextField::TTTextField(Application* application, TTFont* font, const char* text, const char* sample, FontBase::TextLayoutParameters *layout) : TextFieldBase(application)
{
	font_ = font;
	if (font_)
		font_->ref();

	data_ = NULL;

	if (text)
		text_ = text;

	if (sample)
		sample_ = sample;

	textColor_ = 0;

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

    if (layout)
		layout_=*layout;

    FontBase::TextLayoutParameters empty;
    FontBase::TextLayout l;
    bool isRGB;
    font_->renderFont(sample_.c_str(), &empty, &sminx, &sminy, &smaxx, &smaxy, textColor_, isRGB, l);
    sminx = sminx/scalex;
    sminy = sminy/scaley;
    smaxx = smaxx/scalex;
    smaxy = smaxy/scaley;


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
	scaleChanged(); //Mark current scale as graphics scale
	if (data_)
	{
		application_->getTextureManager()->destroyTexture(data_);
		data_ = NULL;
	}

	if (text_.empty())
	{
		graphicsBase_.clear();
		graphicsBase_.getBounds(&minx_, &miny_, &maxx_, &maxy_);
		return;
	}

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

	TextureParameters parameters;
	float smoothing=font_->getSmoothing();
    if (smoothing!=0)
    {
        parameters.filter = eLinear;
        scalex/=smoothing;
        scaley/=smoothing;
    }


    int minx, miny, maxx, maxy;
    bool isRGB;
    FontBase::TextLayout l;
    Dib dib = font_->renderFont(text_.c_str(), &layout_, &minx, &miny, &maxx, &maxy,textColor_,isRGB,l);
    textwidth_=l.w;
    textheight_=l.bh;
    parameters.format=isRGB?eRGBA8888:eA8;


    if (!sample_.empty())
    {
        maxx = maxx - minx;
        minx = 0;
        miny = miny - sminy*scaley;
        maxy = maxy - sminy*scaley;
    }

    int dx = minx - 1;
    int dy = miny - 1;

	data_ = application_->getTextureManager()->createTextureFromDib(dib, parameters);

	graphicsBase_.data = data_;

	graphicsBase_.mode = ShaderProgram::TriangleStrip;

	graphicsBase_.vertices.resize(4);
	graphicsBase_.vertices[0] = Point2f(dx / scalex,					dy / scaley);
	graphicsBase_.vertices[1] = Point2f((data_->width + dx) / scalex,	dy / scaley);
	graphicsBase_.vertices[2] = Point2f((data_->width + dx) / scalex,	(data_->height + dy) / scaley);
	graphicsBase_.vertices[3] = Point2f(dx / scalex,					(data_->height + dy) / scaley);
	graphicsBase_.vertices.Update();

	float u = (float)data_->width / (float)data_->exwidth;
	float v = (float)data_->height / (float)data_->exheight;

	graphicsBase_.texcoords.resize(4);
	graphicsBase_.texcoords[0] = Point2f(0, 0);
	graphicsBase_.texcoords[1] = Point2f(u, 0);
	graphicsBase_.texcoords[2] = Point2f(u, v);
	graphicsBase_.texcoords[3] = Point2f(0, v);
	graphicsBase_.texcoords.Update();

	graphicsBase_.indices.resize(4);
	graphicsBase_.indices[0] = 0;
	graphicsBase_.indices[1] = 1;
	graphicsBase_.indices[2] = 3;
	graphicsBase_.indices[3] = 2;
	graphicsBase_.indices.Update();

	if (!isRGB)
	{
		int r = (textColor_ >> 16) & 0xff;
		int g = (textColor_ >> 8) & 0xff;
		int b = textColor_ & 0xff;
		graphicsBase_.setColor(r / 255.f, g / 255.f, b / 255.f, 1);
	}

    minx_ = minx/scalex;
    miny_ = miny/scaley;
    maxx_ = maxx/scalex;
    maxy_ = maxy/scaley;
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
    G_UNUSED(sx);
    G_UNUSED(sy);
    G_UNUSED(ex);
    G_UNUSED(ey);
    if (scaleChanged()) createGraphics();
	graphicsBase_.draw(shader_);
}

void TTTextField::setFont(FontBase* font)
{
    if (font->getType() != FontBase::eTTFont) return;

    font_->unref();
    font_ = static_cast<TTFont*>(font);;
    font_->ref();

    createGraphics();
}

void TTTextField::setText(const char* text)
{
	if (strcmp(text, text_.c_str()) == 0)
		return;

	text_ = text;

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
	if (layout_.letterSpacing == letterSpacing)
		return;

	layout_.letterSpacing = letterSpacing;

	createGraphics();
}

float TTTextField::letterSpacing() const
{
	return layout_.letterSpacing;
}

float TTTextField::lineHeight() const
{
    float scaley = application_->getLogicalScaleY();
    return sample_.empty()? 0 : smaxy - sminy;
}

void TTTextField::setSample(const char* sample)
{
    sample_ = sample;

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();
	float smoothing=font_->getSmoothing();
    if (smoothing!=0)
    {
        scalex/=smoothing;
        scaley/=smoothing;
    }

    FontBase::TextLayoutParameters empty;
    FontBase::TextLayout l;
    bool isRGB;
    font_->renderFont(sample, &empty, &sminx, &sminy, &smaxx, &smaxy,textColor_,isRGB,l);
    sminx = sminx/scalex;
    sminy = sminy/scaley;
    smaxx = smaxx/scalex;
    smaxy = smaxy/scaley;

    createGraphics();
}

const char* TTTextField::sample() const
{
    return sample_.c_str();
}

void TTTextField::setLayout(FontBase::TextLayoutParameters *l)
{
	if (l)
	{
		layout_=*l;
		createGraphics();
	}
}
