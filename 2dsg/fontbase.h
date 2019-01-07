#ifndef FONTBASE_H
#define FONTBASE_H

#include <refptr.h>
#include <wchar32.h>
#include <vector>

class Application;
class GraphicsBase;

class FontBase : public GReferenced
{
public:
    FontBase(Application *application) : application_(application), cacheVersion_(0)
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
		eCompositeFont,
	};

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy) = 0;
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1) = 0;
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1) = 0;
    virtual float getAscender() = 0;
    virtual float getDescender() = 0;
    virtual float getLineHeight() = 0;

	virtual Type getType() const = 0;
	int getCacheVersion() { return cacheVersion_; };

	struct FontSpec {
		std::string filename;
		float sizeMult;
	};

#define TEXTSTYLEFLAG_COLOR         1
#define TEXTSTYLEFLAG_SKIPLAYOUT	2
    struct ChunkLayout {
		std::string text;
		float x,y;
		float w,h;
		float dx,dy;
		int line;
		char sep;
		float sepl;
		//Styling
		int styleFlags;
		unsigned int color;
	};
	struct TextLayout {
    	TextLayout() : styleFlags(0) { };
		float x,y;
        float w,h,bh;
		int lines;
		int styleFlags;
		std::vector<struct ChunkLayout> parts;
	};

	enum TextLayoutFlags {
		TLF_LEFT=0,
		TLF_RIGHT=1,
		TLF_CENTER=2,
		TLF_JUSTIFIED=4,
		TLF_TOP=0,
		TLF_BOTTOM=8,
		TLF_VCENTER=16,
		TLF_NOWRAP=32,
		TLF_RTL=64,
        TLF_REF_BASELINE=0,
        TLF_REF_TOP=128,
        TLF_REF_MIDDLE=256,
        TLF_REF_BOTTOM=512,
		TLF_BREAKWORDS=1024,
        TLF_REF_LINEBOTTOM=2048,
        TLF_REF_LINETOP=4096
	};

	struct TextLayoutParameters {
        TextLayoutParameters() : w(0),h(0),flags(TLF_NOWRAP),letterSpacing(0),lineSpacing(0),tabSpace(4),breakchar("") {};
		float w,h;
		int flags;
		float letterSpacing;
		float lineSpacing;
		float tabSpace;
		std::string breakchar;
	};
	virtual TextLayout layoutText(const char *text, TextLayoutParameters *params);
protected:
	void layoutHorizontal(FontBase::TextLayout *tl,int start, float w, float cw, float sw, float tabSpace, int flags,float letterSpacing, bool wrapped=false, int end=-1);
    Application *application_;
	int cacheVersion_;
};

class BMFontBase : public FontBase
{
public:
    BMFontBase(Application *application) : FontBase(application)
    {
    }

    virtual void drawText(std::vector<GraphicsBase> *graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny, TextLayout &l) = 0;
};

class CompositeFont : public BMFontBase
{
public:
	struct CompositeFontSpec {
		BMFontBase *font;
		float offsetX,offsetY;
		float colorA,colorR,colorG,colorB;
	};
    CompositeFont(Application *application, std::vector<CompositeFontSpec> fonts);
    virtual ~CompositeFont();

    virtual Type getType() const
    {
        return eCompositeFont;
    }

    virtual void drawText(std::vector<GraphicsBase> *graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l);
    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy);
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1);
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1);
    virtual float getAscender();
    virtual float getDescender();
    virtual float getLineHeight();
protected:
    std::vector<CompositeFontSpec> fonts_;
    struct FontInfo
    {
        float height;
        float ascender;
        float descender;
    } fontInfo_;
};

#endif
