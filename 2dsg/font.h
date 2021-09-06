#ifndef FONT_H
#define FONT_H

#include "fontbase.h"
#include <map>
#include <wchar32.h>
#include <gstatus.h>

class GraphicsBase;
struct TextureData;

class Font : public BMFontBase
{
public:
    Font(Application *application);
    Font(Application *application, const char *glympfile, const char *imagefile, bool filtering, GStatus *status);
	virtual ~Font();

	virtual Type getType() const
	{
		return eFont;
	}

    virtual void drawText(std::vector<GraphicsBase> * graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l);

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy, std::string name="");
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1, std::string name="");
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1, std::string name="");
    virtual float getAscender();
    virtual float getDescender();
    virtual float getLineHeight();

private:
    void constructor(const char *glympfile, const char *imagefile, bool filtering);
    int kerning(wchar32_t left, wchar32_t right) const;

private:
	struct TextureGlyph
	{
        wchar32_t chr;
		int x, y;
		int width, height;
		int left, top;
		int advancex, advancey;
	};

    struct FontInfo
    {
        int height;
        int ascender;
        int descender;
        bool isSetTextColorAvailable;
        std::map<wchar32_t, TextureGlyph> textureGlyphs;
        std::map<std::pair<wchar32_t, wchar32_t>, int> kernings;
    };

    FontInfo fontInfo_;

    float sizescalex_;
    float sizescaley_;
    float uvscalex_;
    float uvscaley_;

    TextureData *data_;

    int getTextureGlyphsFormat(const char *filename);
    void readTextureGlyphsOld(const char *filename);
    void readTextureGlyphsNew(const char *filename);
};


#endif
