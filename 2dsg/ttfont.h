#ifndef TTFONT_H
#define TTFONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fontbase.h>
#include <dib.h>
#include <wchar32.h>
#include <gstatus.h>
#include <prpath.h>
#include <map>

class TTFont : public FontBase
{
public:
    TTFont(Application *application, std::vector<FontSpec> filenames, float size, float smoothing, GStatus *status);
    virtual ~TTFont();

	virtual Type getType() const
	{
		return eTTFont;
	}

    void getBounds(const wchar32_t *text, float letterSpacing, int *pminx, int *pminy, int *pmaxx, int *pmaxy);

    Dib renderFont(const char *text, TextLayoutParameters *layout, int *pminx, int *pminy, int *pmaxx, int *pmaxy);

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy);
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1);
    virtual float getAscender();
    virtual float getLineHeight();

    float getSmoothing() const
    {
        return smoothing_;
    }
    FT_Face getFace(int chr, FT_UInt &glpyhIndex);
private:
    void constructor(std::vector<FontSpec> filenames, float size, float smoothing);
    int kerning(FT_Face face, FT_UInt left, FT_UInt right) const;

private:
    struct FontFace
    {
        FT_Face face;
        FT_StreamRec stream;
        float sizeMult;
    };
    std::vector<FontFace> fontFaces_;
	int ascender_;
	int height_;
    float smoothing_;
    float currentLogicalScaleX_,currentLogicalScaleY_;
    float defaultSize_;
    void checkLogicalScale();
    struct GlyphData
    {
    	FT_Face			face;
    	FT_UInt 		glyph;
    	int				advX;
    	int 			top;
    	int 			left;
    	unsigned int	height;
    	unsigned int    width;
    	unsigned char *	bitmap;
    	int             pitch;
    };
    std::map<wchar32_t,GlyphData> glyphCache_;
};

#endif
