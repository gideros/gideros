#ifndef FONTBASE_H
#define FONTBASE_H

#include <refptr.h>
#include <wchar32.h>
#include <vector>
#include <cstdint>

class Application;
class GraphicsBase;
class FontShaper;
typedef FontShaper *(*FontshaperBuilder_t)(void *fontData,size_t fontDataSize,int size,int xres,int yres);

class FontBase : public GReferenced
{
public:
    FontBase(Application *application) : application_(application), cacheVersion_(0), shaper_(NULL)
	{
	}

    virtual ~FontBase();

	enum Type
	{
		eFont,
		eTTFont,
        eTTBMFont,
		eCompositeFont,
	};

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy, std::string name="") = 0;
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1, std::string name="") = 0;
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1, std::string name="") = 0;
    virtual float getAscender() = 0;
    virtual float getDescender() = 0;
    virtual float getLineHeight() = 0;
    virtual float getSpaceSize() { return getAdvanceX(" ",0); };

	virtual Type getType() const = 0;
	int getCacheVersion() { return cacheVersion_; };

	struct FontSpec {
		std::string filename;
		float sizeMult;
	};

	struct GlyphLayout {
		int glyph; //Glyph number in font
		int srcIndex; //Original codepoint index in source text
		int offX,offY,advX,advY; //Draw offset and pen advance
        void *_private; //Private info for the font renderer
	};

#define TEXTSTYLEFLAG_COLOR         1
#define TEXTSTYLEFLAG_SKIPLAYOUT	2
#define TEXTSTYLEFLAG_RTL			4
#define TEXTSTYLEFLAG_LTR			8
#define TEXTSTYLEFLAG_SKIPSHAPING	16
#define TEXTSTYLEFLAG_FORCESHAPING	32
#define TEXTSTYLEFLAG_ITALIC        64
#define TEXTSTYLEFLAG_UNDERLINE     128
    struct ChunkStyle {
        int styleFlags;
        unsigned int color;
        std::string font;
        int8_t italic; //Shear angle, +/- 90°
        uint8_t underline_size;
        /* If positive, position from baseline to ascender of text, if negative position from baseline to descender of text
         * Useful values: 0=very bottom, 48=strikethru, -64=underline
         */
        int8_t underline_pos;
        bool operator==(const ChunkStyle &b) {
            return (styleFlags==b.styleFlags)&&
                    (font==b.font)&&
                    (color==b.color)&&
                    (italic==b.italic)&&
                    (underline_size==b.underline_size)&&
                    (underline_pos==b.underline_pos);
        }
    };
    struct ChunkLayout {
		std::string text;
		std::vector<struct GlyphLayout> shaped;
        float advX,advY; //advance from this chunk to next one, pen-wise
        float x,y; //bounds of this chunk
        float w,h;
        float dx,dy; //pen start position for this chunk
		float shapeScaleX,shapeScaleY;
		int line;
		wchar32_t sep;
		float sepl;
		int sepflags;
        int extrasize; //adjustement from original text size (breakchar or styles)
        ChunkStyle style;
	};
	struct TextLayout {
        TextLayout() : x(0),y(0),dx(0),dy(0),w(0),h(0),bh(0),mw(0),lines(0),styleFlags(0) { };
		float x,y;
        float dx,dy;
        float w,h,bh,mw,cw;
		int lines;
		int styleFlags;
		std::vector<struct ChunkLayout> parts;
        std::map<std::string,struct ChunkLayout> metricsCache;
        float letterSpacingCache;
        void clear() {
            x=y=dx=dy=w=h=bh=mw=cw=lines=styleFlags=0;
			parts.clear();
            metricsCache.clear();
		};
        void dispose()
        {
            parts.clear();
            metricsCache.clear();
        };
	};
	struct ChunkClass {
		std::string text;
		wchar32_t sep;
#define CHUNKCLASS_FLAG_BREAKABLE	1
#define CHUNKCLASS_FLAG_BREAK		2
#define CHUNKCLASS_FLAG_LTR			4
#define CHUNKCLASS_FLAG_RTL			8
#define CHUNKCLASS_FLAG_STYLE		16
		uint8_t textFlags;
		uint8_t sepFlags;
		uint16_t script;
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
        TLF_BREAKWORDS=128,
        TLF_REF_MASK=0xF00,
        TLF_REF_BASELINE=0x000,
        TLF_REF_TOP=0x100,
        TLF_REF_MIDDLE=0x200,
        TLF_REF_BOTTOM=0x300,
        TLF_REF_LINEBOTTOM=0x400,
        TLF_REF_LINETOP=0x500,
        TLF_REF_ASCENT=0x600,
        TLF_REF_DESCENT=0x700,
        TLF_REF_MEDIAN=0x800,
        TLF_LTR=(1<<12),
        TLF_NOSHAPING=(1<<13),
        TLF_NOBIDI=(1<<14),
        TLF_SINGLELINE=(1<<15),
        TLF_FORCESHAPING=(1<<16),
        TLF_DISPOSELAYOUT=(1<<17),
        TLF_TABLE=(1<<18),
    };

    struct TextLayoutParameters {
        TextLayoutParameters() : w(0),h(0),flags(TLF_NOWRAP),letterSpacing(0),lineSpacing(0),tabSpace(4),breakchar(""),alignx(0),aligny(0),aspect(100000) {}; //Very big aspect ratio
        float w,h;
        int flags;
        float letterSpacing;
        float lineSpacing;
        float tabSpace;
        std::string breakchar;
        float alignx,aligny;
        float aspect;
        bool operator <(const TextLayoutParameters & op2) const
        {
            if (w<op2.w) return true;
            if (flags<op2.flags) return true;
            if (letterSpacing<op2.letterSpacing) return true;
            if (lineSpacing<op2.lineSpacing) return true;
            if (tabSpace<op2.tabSpace) return true;
            if (breakchar<op2.breakchar) return true;
            if (alignx<op2.alignx) return true;
            if (aligny<op2.aligny) return true;
            if (aspect<op2.aspect) return true;
            if (!aligny) return false;
            if (h<op2.h) return true;
            return false;
        };
        bool operator ==(const TextLayoutParameters & op2) const {
            return (!((*this)<op2))&&(!(op2<(*this)));
        }
    };

    struct TextLayoutInput {
        TextLayoutParameters params;
        std::string text;
        bool operator <(const TextLayoutInput & op2) const
        {
            if (text<op2.text) return true;
            if (params<op2.params) return true;
            return false;
        };
        bool operator ==(const TextLayoutInput & op2) const {
            return (!((*this)<op2))&&(!(op2<(*this)));
        }
    };

    virtual void chunkMetrics(struct ChunkLayout &part, FontBase::TextLayoutParameters *params);
    size_t getCharIndexAtOffset(struct ChunkLayout &part, float offset, float letterSpacing, bool notFirst);
    virtual void layoutText(const char *text, TextLayoutParameters *params, TextLayout &tl);
protected:
    void layoutHorizontal(FontBase::TextLayout &tl,int start, float w, float cw, float sw, float tabSpace, FontBase::TextLayoutParameters *params, bool wrapped=false, int end=-1);
    void chunkMetricsCache(FontBase::TextLayout &tl,struct ChunkLayout &part, FontBase::TextLayoutParameters *params);
    Application *application_;
	int cacheVersion_;
    FontShaper *shaper_;
};

class FontShaper {
public:
    virtual bool shape(struct FontBase::ChunkLayout &part,std::vector<wchar32_t> &wtext)=0;
    virtual ~FontShaper() { }
};

class BMFontBase : public FontBase
{
public:
    BMFontBase(Application *application) : FontBase(application)
    {
    }

    virtual void drawText(std::vector<GraphicsBase> *graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny, TextLayout &l) = 0;
    virtual void preDraw() {}; //Called when some text is about to be actually rendered, to flush data to the GPU
};

class CompositeFont : public BMFontBase
{
public:
	struct CompositeFontSpec {
		BMFontBase *font;
		float offsetX,offsetY;
		float colorA,colorR,colorG,colorB;
        std::string name;
	};
    CompositeFont(Application *application, std::vector<CompositeFontSpec> fonts);
    virtual ~CompositeFont();

    virtual Type getType() const
    {
        return eCompositeFont;
    }

    virtual void chunkMetrics(struct ChunkLayout &part, FontBase::TextLayoutParameters *params);
    virtual void drawText(std::vector<GraphicsBase> *graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l);
    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy, std::string name="");
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1, std::string name="");
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1, std::string name="");
    virtual float getAscender();
    virtual float getDescender();
    virtual float getLineHeight();
    virtual float getSpaceSize();
    virtual void preDraw();
protected:
    size_t selectFont(std::string name);
    std::vector<CompositeFontSpec> fonts_;
    struct FontInfo
    {
        float height;
        float ascender;
        float descender;
        float spaceSize;
    } fontInfo_;
};

typedef bool (*TextClassifier_t)(std::vector<FontBase::ChunkClass> &chunks,std::string text);

#endif
