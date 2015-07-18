#ifndef TTFONT_H
#define TTFONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fontbase.h>
#include <dib.h>
#include <wchar32.h>
#include <gstatus.h>

class TTFont : public FontBase
{
public:
    TTFont(Application *application, const char *filename, float size, bool smoothing, GStatus *status);
    virtual ~TTFont();

	virtual Type getType() const
	{
		return eTTFont;
	}

    void getBounds(const wchar32_t *text, float letterSpacing, int *pminx, int *pminy, int *pmaxx, int *pmaxy) const;

    Dib renderFont(const wchar32_t *text, float letterSpacing, int *pminx, int *pminy, int *pmaxx, int *pmaxy) const;

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy) const;
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1) const;
    virtual float getAscender() const;
    virtual float getLineHeight() const;

    bool getSmoothing() const
    {
        return smoothing_;
    }

private:
    void constructor(const char *filename, float size, bool smoothing);
    int kerning(FT_UInt left, FT_UInt right) const;

private:
	FT_Face face_;
	int ascender_;
	int height_;
	FT_StreamRec stream_;
    bool smoothing_;
};

#endif
