#include <ttbmfont.h>

#include <ft2build.h>
#include FT_OUTLINE_H

#include <ftlibrarysingleton.h>

#include <gstdio.h>
#include <gstatus.h>
#include <giderosexception.h>
#include <application.h>
#include <texturepacker.h>
#include <dib.h>
#include <graphicsbase.h>
#include <utf8.h>
#include <algorithm>

static unsigned long read(	FT_Stream stream,
                            unsigned long offset,
                            unsigned char *buffer,
                            unsigned long count)
{
    G_FILE *fis = (G_FILE*)stream->descriptor.pointer;
    g_fseek(fis, offset, SEEK_SET);
    if (count == 0)
        return 0;
    return g_fread(buffer, 1, count, fis);
}

static void close(FT_Stream stream)
{
    G_FILE *fis = (G_FILE*)stream->descriptor.pointer;
    g_fclose(fis);
}

TTBMFont::TTBMFont(Application *application, const char *filename, float size, const char *chars, bool filtering, GStatus *status) : BMFontBase(application)
{
    try
    {
        constructor(filename, size, chars, filtering);
    }
    catch (GiderosException &e)
    {
        if (status)
            *status = e.status();
    }
}

void TTBMFont::constructor(const char *filename, float size, const char *chars, bool filtering)
{
    data_ = NULL;

    G_FILE *fis = g_fopen(filename, "rb");
    if (!fis)
        throw GiderosException(GStatus(6000, filename));		// Error #6000: %s: No such file or directory.

    FT_StreamRec stream;
    memset(&stream, 0, sizeof(stream));

    g_fseek(fis, 0, SEEK_END);
    stream.size = g_ftell(fis);
    g_fseek(fis, 0, SEEK_SET);
    stream.descriptor.pointer = fis;
    stream.read = read;
    stream.close = close;

    FT_Open_Args args;
    memset(&args, 0, sizeof(args));
    args.flags = FT_OPEN_STREAM;
    args.stream = &stream;

    FT_Error error;

    FT_Face face;

    error = FT_Open_Face(FT_Library_Singleton::instance(), &args, 0, &face);
    if (error)
        throw GiderosException(GStatus(6012, filename));		// Error #6012: %s: Error while reading font file.

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

    const int RESOLUTION = 72;
    error = FT_Set_Char_Size(face, 0L, (int)floor(size * 64 + 0.5f), (int)floor(RESOLUTION * scalex + 0.5f), (int)floor(RESOLUTION * scaley + 0.5f));

    if (error)
    {
        FT_Done_Face(face);
        throw GiderosException(GStatus(6017, filename));		// Error #6017: Invalid font size.
    }

    std::vector<wchar32_t> wchars;
    size_t len = utf8_to_wchar(chars, strlen(chars), NULL, 0, 0);
    if (len != 0)
    {
        wchars.resize(len);
        utf8_to_wchar(chars, strlen(chars), &wchars[0], len, 0);
    }

    fontInfo_.ascender = face->size->metrics.ascender >> 6;
    fontInfo_.height = face->size->metrics.height >> 6;

    std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;

    textureGlyphs.clear();

    for (size_t i = 0; i < len; ++i)
    {
        wchar32_t chr = wchars[i];

        FT_UInt glyphIndex = FT_Get_Char_Index(face, chr);
        if (glyphIndex == 0)	// 0 means `undefined character code'
            continue;

        error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
        if (error)
            continue;

        int top, left, width, height;
        if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

            bbox.xMin &= ~63;
            bbox.yMin &= ~63;
            bbox.xMax  = (bbox.xMax + 63) & ~63;
            bbox.yMax  = (bbox.yMax + 63) & ~63;

            width  = (bbox.xMax - bbox.xMin) >> 6;
            height = (bbox.yMax - bbox.yMin) >> 6;
            top = bbox.yMax >> 6;
            left = bbox.xMin >> 6;
        }
        else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
        {
            width = face->glyph->bitmap.width;
            height = face->glyph->bitmap.rows;
            top = face->glyph->bitmap_top;
            left = face->glyph->bitmap_left;
        }
        else
            continue;

        TextureGlyph textureGlyph;
        textureGlyph.chr = chr;
        textureGlyph.glyphIndex = glyphIndex;
        textureGlyph.top = top;
        textureGlyph.left = left;
        textureGlyph.width = width;
        textureGlyph.height = height;
        textureGlyph.advancex = face->glyph->advance.x;
        textureGlyph.advancey = face->glyph->advance.y;

        textureGlyphs[chr] = textureGlyph;
    }

    std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings = fontInfo_.kernings;

    kernings.clear();

    if (FT_HAS_KERNING(face))
    {
        std::map<wchar32_t, TextureGlyph>::iterator iter1, iter2, e = textureGlyphs.end();

        for (iter1 = textureGlyphs.begin(); iter1 != e; ++iter1)
            for (iter2 = textureGlyphs.begin(); iter2 != e; ++iter2)
            {
                const TextureGlyph &g1 = iter1->second;
                const TextureGlyph &g2 = iter2->second;

                FT_Vector delta;
                FT_Get_Kerning(face,
                               g1.glyphIndex, g2.glyphIndex,
                               FT_KERNING_DEFAULT, &delta);

                if (delta.x != 0)
                    kernings[std::make_pair(g1.chr, g2.chr)] = delta.x;
            }
    }

    TexturePacker *tp = createTexturePacker();

    tp->setTextureCount(textureGlyphs.size());
    std::map<wchar32_t, TextureGlyph>::iterator iter, e = textureGlyphs.end();
    for (iter = textureGlyphs.begin(); iter != e; ++iter)
        tp->addTexture(iter->second.width, iter->second.height);

    int width = 0, height = 0;
    tp->packTextures(&width, &height, 2, false);

    width = std::max(width, 1);
    height = std::max(height, 1);

    Dib dib(application_, width, height, true);
    unsigned char rgba[] = {255, 255, 255, 0};
    dib.fill(rgba);

    int i = 0;
    for (iter = textureGlyphs.begin(); iter != e; ++iter, ++i)
    {
        int xo, yo;
        int width, height;
        tp->getTextureLocation(i, &xo, &yo, &width, &height);

        FT_UInt glyph_index = FT_Get_Char_Index(face, iter->second.chr);
        if (glyph_index == 0)
            continue;

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue;

        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error)
            continue;

        FT_Bitmap &bitmap = face->glyph->bitmap;

        iter->second.x = xo;
        iter->second.y = yo;

        width = std::min(width, (int)bitmap.width);
        height = std::min(height, (int)bitmap.rows);

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int index = x + y * bitmap.pitch;
                int c = bitmap.buffer[index];

                dib.setAlpha(xo + x, yo + y, c);
            }
    }

    releaseTexturePacker(tp);

    FT_Done_Face(face);

    TextureParameters parameters;
    parameters.filter = filtering ? eLinear : eNearest;
    parameters.wrap = eClamp;
    data_ = application_->getTextureManager()->createTextureFromDib(dib, parameters);

    sizescalex_ = 1 / scalex;
    sizescaley_ = 1 / scaley;
    uvscalex_ = 1;
    uvscaley_ = 1;
}

TTBMFont::~TTBMFont()
{
    if (data_)
        application_->getTextureManager()->destroyTexture(data_);
}

void TTBMFont::drawText(GraphicsBase* graphicsBase, const wchar32_t* text, float r, float g, float b, float letterSpacing, bool hasSample, float minx, float miny)
{
    int size = 0;
    for (const wchar32_t *t = text; *t; ++t, ++size)
        ;

    if (size == 0)
    {
        graphicsBase->clear();
        return;
    }

    graphicsBase->data = data_;
    graphicsBase->setColor(r, g, b, 1);
    graphicsBase->vertices.resize(size * 4);
    graphicsBase->texcoords.resize(size * 4);
    graphicsBase->indices.resize(size * 6);
    graphicsBase->vertices.Update();
    graphicsBase->texcoords.Update();
    graphicsBase->indices.Update();

    float x = -minx/sizescalex_, y = -miny/sizescaley_;

    if (hasSample) {
        std::map<wchar32_t, TextureGlyph>::const_iterator iter = fontInfo_.textureGlyphs.find(text[0]);
        const TextureGlyph &textureGlyph = iter->second;
        x = -textureGlyph.left;
        //y *= application_->getLogicalScaleY();
    }

    wchar32_t prev = 0;

    for (int i = 0; i < size; ++i)
    {
        std::map<wchar32_t, TextureGlyph>::const_iterator iter = fontInfo_.textureGlyphs.find(text[i]);

        if (iter == fontInfo_.textureGlyphs.end())
            continue;

        const TextureGlyph &textureGlyph = iter->second;

        int width = textureGlyph.width;
        int height = textureGlyph.height;
        int left = textureGlyph.left;
        int top = textureGlyph.top;

        x += kerning(prev, text[i]) >> 6;
        prev = text[i];

        float x0 = x + left;
        float y0 = y - top;

        float x1 = x + left + width;
        float y1 = y - top + height;

        graphicsBase->vertices[i * 4 + 0] = Point2f(sizescalex_ * x0, sizescaley_ * y0);
        graphicsBase->vertices[i * 4 + 1] = Point2f(sizescalex_ * x1, sizescaley_ * y0);
        graphicsBase->vertices[i * 4 + 2] = Point2f(sizescalex_ * x1, sizescaley_ * y1);
        graphicsBase->vertices[i * 4 + 3] = Point2f(sizescalex_ * x0, sizescaley_ * y1);

        float u0 = (float)textureGlyph.x / (float)data_->exwidth;
        float v0 = (float)textureGlyph.y / (float)data_->exheight;
        float u1 = (float)(textureGlyph.x + width) / (float)data_->exwidth;
        float v1 = (float)(textureGlyph.y + height) / (float)data_->exheight;

        u0 *= uvscalex_;
        v0 *= uvscaley_;
        u1 *= uvscalex_;
        v1 *= uvscaley_;

        graphicsBase->texcoords[i * 4 + 0] = Point2f(u0, v0);
        graphicsBase->texcoords[i * 4 + 1] = Point2f(u1, v0);
        graphicsBase->texcoords[i * 4 + 2] = Point2f(u1, v1);
        graphicsBase->texcoords[i * 4 + 3] = Point2f(u0, v1);

        graphicsBase->indices[i * 6 + 0] = i * 4 + 0;
        graphicsBase->indices[i * 6 + 1] = i * 4 + 1;
        graphicsBase->indices[i * 6 + 2] = i * 4 + 2;
        graphicsBase->indices[i * 6 + 3] = i * 4 + 0;
        graphicsBase->indices[i * 6 + 4] = i * 4 + 2;
        graphicsBase->indices[i * 6 + 5] = i * 4 + 3;

        x += textureGlyph.advancex >> 6;

        x += (int)(letterSpacing / sizescalex_);
    }
}

void TTBMFont::getBounds(const char *text, float letterSpacing, float *pminx, float *pminy, float *pmaxx, float *pmaxy)
{
    float minx = 1e30;
    float miny = 1e30;
    float maxx = -1e30;
    float maxy = -1e30;

    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0)
    {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    float x = 0, y = 0;
    wchar32_t prev = 0;
    for (std::size_t i = 0; i < wtext.size(); ++i)
    {
        std::map<wchar32_t, TextureGlyph>::const_iterator iter = fontInfo_.textureGlyphs.find(wtext[i]);

        if (iter == fontInfo_.textureGlyphs.end())
            continue;

        const TextureGlyph &textureGlyph = iter->second;

        int width = textureGlyph.width;
        int height = textureGlyph.height;
        int left = textureGlyph.left;
        int top = textureGlyph.top;

        x += kerning(prev, wtext[i]) >> 6;
        prev = wtext[i];

        float x0 = x + left;
        float y0 = y - top;

        float x1 = x + left + width;
        float y1 = y - top + height;

        minx = std::min(minx, sizescalex_ * x0);
        minx = std::min(minx, sizescalex_ * x1);
        miny = std::min(miny, sizescaley_ * y0);
        miny = std::min(miny, sizescaley_ * y1);
        maxx = std::max(maxx, sizescalex_ * x0);
        maxx = std::max(maxx, sizescalex_ * x1);
        maxy = std::max(maxy, sizescaley_ * y0);
        maxy = std::max(maxy, sizescaley_ * y1);

        x += textureGlyph.advancex >> 6;

        x += (int)(letterSpacing / sizescalex_);
    }

    if (pminx)
        *pminx = minx;
    if (pminy)
        *pminy = miny;
    if (pmaxx)
        *pmaxx = maxx;
    if (pmaxy)
        *pmaxy = maxy;
}

float TTBMFont::getAdvanceX(const char *text, float letterSpacing, int size)
{
    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0)
    {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    if (size < 0 || size > wtext.size())
        size = wtext.size();

    wtext.push_back(0);

    float x = 0;
    wchar32_t prev = 0;
    for (int i = 0; i < size; ++i)
    {
        std::map<wchar32_t, TextureGlyph>::const_iterator iter = fontInfo_.textureGlyphs.find(wtext[i]);

        if (iter == fontInfo_.textureGlyphs.end())
            continue;

        const TextureGlyph &textureGlyph = iter->second;

        x += kerning(prev, wtext[i]) >> 6;
        prev = wtext[i];

        x += textureGlyph.advancex >> 6;

        x += (int)(letterSpacing / sizescalex_);
    }

    x += kerning(prev, wtext[size]) >> 6;

    return x * sizescalex_;
}

int TTBMFont::kerning(wchar32_t left, wchar32_t right) const
{
    std::map<std::pair<wchar32_t, wchar32_t>, int>::const_iterator iter;
    iter = fontInfo_.kernings.find(std::make_pair(left, right));
    return (iter != fontInfo_.kernings.end()) ? iter->second : 0;
}

float TTBMFont::getAscender()
{
    return fontInfo_.ascender * sizescaley_;
}

float TTBMFont::getLineHeight()
{
    return fontInfo_.height * sizescaley_;
}
