#ifndef TTBMFONT_H
#define TTBMFONT_H

#include <fontbase.h>
#include <wchar32.h>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

class GStatus;
class GraphicsBase;
struct TextureData;

class TTBMFont : public BMFontBase
{
public:
    TTBMFont(Application *application, const char *filename, float size, const char *chars, bool filtering, GStatus *status);
    virtual ~TTBMFont();

    virtual Type getType() const
    {
        return eTTBMFont;
    }

    virtual void drawText(GraphicsBase *graphicsBase, const wchar32_t *text, float r, float g, float b, float letterSpacing, bool hasSample, float minx, float miny);

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy);
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1);
    virtual float getAscender();
    virtual float getLineHeight();

private:
    void constructor(const char *filename, float size, const char *chars, bool filtering);
    int kerning(wchar32_t left, wchar32_t right) const;

private:
    struct TextureGlyph
    {
        wchar32_t chr;
        FT_UInt glyphIndex;
        int x, y;
        int width, height;
        int left, top;
        int advancex, advancey;
    };

    struct FontInfo
    {
        int height;
        int ascender;
        std::map<wchar32_t, TextureGlyph> textureGlyphs;
        std::map<std::pair<wchar32_t, wchar32_t>, int> kernings;
    };

    FontInfo fontInfo_;

    float sizescalex_;
    float sizescaley_;
    float uvscalex_;
    float uvscaley_;

    TextureData *data_;
};

#endif
