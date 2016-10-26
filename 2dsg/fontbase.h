#ifndef FONTBASE_H
#define FONTBASE_H

#include <refptr.h>
#include <wchar32.h>

class Application;
class GraphicsBase;


class FontBase : public GReferenced
{
public:
    FontBase(Application *application) : application_(application)
	{
	}

    virtual ~FontBase()
    {
    }

	enum Type
	{
		eFont,
		eTTFont,
        eTTBMFont,
	};

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy) = 0;
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1) = 0;
    virtual float getAscender() = 0;
    virtual float getLineHeight() = 0;

	virtual Type getType() const = 0;

protected:
    Application *application_;
};

class BMFontBase : public FontBase
{
public:
    BMFontBase(Application *application) : FontBase(application)
    {
    }

    virtual void drawText(GraphicsBase *graphicsBase, const wchar32_t *text, float r, float g, float b, float letterSpacing, bool hasSample, float minx, float miny) = 0;
};


#endif
