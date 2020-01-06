#ifndef TTBMFONT_H
#define TTBMFONT_H

#include <fontbase.h>
#include <wchar32.h>
#include <map>
#include <texturepacker.h>
#include <dib.h>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

class GStatus;
class GraphicsBase;
struct TextureData;

class TTBMFont : public BMFontBase
{
public:
    TTBMFont(Application *application, std::vector<FontSpec> filenames, float size, const char *chars, float filtering, float outline, GStatus *status);
    virtual ~TTBMFont();

    virtual Type getType() const
    {
        return eTTBMFont;
    }

    virtual void drawText(std::vector<GraphicsBase> *graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l);

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy);
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1);
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1);
    virtual float getAscender();
    virtual float getDescender();
    virtual float getLineHeight();
    virtual bool shapeChunk(struct ChunkLayout &part,std::vector<wchar32_t> &wtext);
    virtual void chunkMetrics(struct ChunkLayout &part, float letterSpacing);
    virtual void preDraw();

private:
    struct TextureGlyph
    {
        wchar32_t chr;
        int x, y;
        int width, height;
        int left, top;
        int advancex, advancey;
        unsigned int texture;
    };

    void constructor(std::vector<FontSpec> filenames, float size, const char *chars, float filtering, float outline);
    int kerning(wchar32_t left, wchar32_t right) const;
    bool addGlyph(const wchar32_t chr);
    bool addFontGlyph(int facenum,FT_UInt glyph,wchar32_t chr);
    void ensureChars(const wchar32_t *text, int size);
    void ensureGlyphs(int facenum,const wchar32_t *text, int size);
    TextureGlyph *getCharGlyph(wchar32_t chr,int &facenum,FT_UInt &glyph);
    bool staticCharsetInit();
    void checkLogicalScale();

private:
    struct FontInfo
    {
        int height;
        int ascender;
        int descender;
        std::map<wchar32_t, FT_UInt> charGlyphs;
        std::map<wchar32_t, int> charFace;
        std::map<std::pair<wchar32_t, wchar32_t>, int> kernings;
    };

    struct FontFace
    {
        FT_Face face;
        FT_StreamRec stream;
        float sizeMult;
        std::map<FT_UInt, TextureGlyph> textureGlyphs;
    };

    std::vector<TextureData *> textureData_;
    TexturePacker *currentPacker_;
    Dib *currentDib_;
    bool dibDirty_;
    float filtering_;
    std::vector<FontFace> fontFaces_;
    std::string charset_;

    FontInfo fontInfo_;
    FT_Stroker stroker;

    float sizescalex_;
    float sizescaley_;
    float uvscalex_;
    float uvscaley_;
    float defaultSize_;
    float outlineSize_;
    float currentLogicalScaleX_,currentLogicalScaleY_;

};

#endif
