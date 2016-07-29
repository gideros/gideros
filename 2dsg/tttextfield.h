#ifndef TTTEXTFIELD_H
#define TTTEXTFIELD_H

#include "textfieldbase.h"

class TTFont;
struct TextureData;
class Application;

class TTTextField : public TextFieldBase
{
public:
	TTTextField(Application* application, TTFont* font);
    TTTextField(Application* application, TTFont* font, const char* text);
    TTTextField(Application* application, TTFont* font, const char* text, const char *sample);
	virtual ~TTTextField();

    virtual void setFont(FontBase *font);

	virtual void setText(const char* text);
	virtual const char* text() const;

	virtual void setTextColor(unsigned int color);
	virtual unsigned int textColor() const;

    virtual void setLetterSpacing(float letterSpacing);
    virtual float letterSpacing() const;

    virtual float lineHeight() const;

    virtual void setSample(const char* sample);
    virtual const char* sample() const;

private:
	void createGraphics();

	virtual void extraBounds(float* minx, float* miny, float* maxx, float* maxy) const;
    virtual void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);

	TextureData* data_;
	TTFont* font_;
	GraphicsBase graphicsBase_;
	unsigned int textColor_;
    float letterSpacing_;
	float minx_, miny_, maxx_, maxy_;
    int sminx, sminy, smaxx, smaxy;
};


#endif
