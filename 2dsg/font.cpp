#include "font.h"
#include "dib.h"
#include "ogl.h"
#include "color.h"
#include "graphicsbase.h"
#include <sstream>
#include <gfile.h>
#include <gstdio.h>
#include <application.h>
#include "texturemanager.h"
#include "giderosexception.h"
#include <string.h>
#include <utf8.h>
#include <algorithm>

#include "embeddedfont.inc"

Font::Font(Application* application) :
    BMFontBase(application)
{
	std::stringstream stream(fontGlymp);

	int doMipmaps_;
	stream >> doMipmaps_;

	while (1)
	{
		TextureGlyph glyph;
		int chr;
		stream >> chr;

		if (stream.eof())
			break;

		glyph.chr = chr;
		stream >> glyph.x >> glyph.y;
		stream >> glyph.width >> glyph.height;
		stream >> glyph.left >> glyph.top;
		stream >> glyph.advancex >> glyph.advancey;

        fontInfo_.textureGlyphs[chr] = glyph;
	}

    fontInfo_.kernings.clear();
    fontInfo_.isSetTextColorAvailable = true;
    fontInfo_.height = 10;
    fontInfo_.ascender = 8;

    sizescalex_ = 1;
    sizescaley_ = 1;
    uvscalex_ = 1;
    uvscaley_ = 1;

	Dib dib(application, 128, 128);

	for (int y = 0; y < 128; ++y)
		for (int x = 0; x < 128; ++x)
		{
			unsigned char c = 255 - fontImage[x + y * 128];
			unsigned char rgba[4] = {c, c, c, 1};
			dib.setPixel(x, y, rgba);
		}


	TextureParameters parameters;
	parameters.filter = eNearest;
	parameters.wrap = eClamp;
	parameters.grayscale = true;
	data_ = application->getTextureManager()->createTextureFromDib(dib, parameters);
}

Font::Font(Application *application, const char *glympfile, const char *imagefile, bool filtering, GStatus *status) : BMFontBase(application)
{
    try
    {
        constructor(glympfile, imagefile, filtering);
    }
    catch (GiderosException &e)
    {
        if (status)
            *status = e.status();
    }
}

void Font::constructor(const char *glympfile, const char *imagefile, bool filtering)
{
    data_ = NULL;

    float scale;
    const char *suffix = application_->getImageSuffix(imagefile, &scale);

    const char *ext = strrchr(glympfile, '.');
    if (ext == NULL)
        ext = glympfile + strlen(glympfile);

    std::string glympfilex = std::string(glympfile, ext - glympfile) + (suffix ? suffix : "") + ext;

    G_FILE *f = g_fopen(glympfilex.c_str(), "r");
    if (f)
        g_fclose(f);

    int format;
    if (f)
        format = getTextureGlyphsFormat(glympfilex.c_str());
    else
        format = getTextureGlyphsFormat(glympfile);

    TextureParameters parameters;
    parameters.filter = filtering ? eLinear : eNearest;
    parameters.wrap = eClamp;
    parameters.grayscale = (format == 0);
    data_ = application_->getTextureManager()->createTextureFromFile(imagefile, parameters);

    if (f)
    {
        switch (format)
        {
        case 0:
            readTextureGlyphsOld(glympfilex.c_str());
            break;
        case 1:
            readTextureGlyphsNew(glympfilex.c_str());
            break;
        }
        sizescalex_ = 1 / scale;
        sizescaley_ = 1 / scale;
        uvscalex_ = 1;
        uvscaley_ = 1;
    }
    else
    {
        switch (format)
        {
        case 0:
            readTextureGlyphsOld(glympfile);
            break;
        case 1:
            readTextureGlyphsNew(glympfile);
            break;
        }
        sizescalex_ = 1;
        sizescaley_ = 1;
        uvscalex_ = (float)data_->width / (float)data_->baseWidth;
        uvscaley_ = (float)data_->height / (float)data_->baseHeight;
    }
}

void Font::drawText(GraphicsBase* graphicsBase, const wchar32_t* text, float r, float g, float b, float letterSpacing, bool hasSample, float minx, float miny)
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
    if (fontInfo_.isSetTextColorAvailable)
        graphicsBase->setColor(r, g, b, 1);
    else
        graphicsBase->setColor(1, 1, 1, 1);
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

        x += letterSpacing / sizescalex_;
	}
}

static bool readLine(G_FILE *f, std::string *line)
{
    const int BUFFER_SIZE = 256;

    char buffer[BUFFER_SIZE];

    std::string line2;
    for (;;)
    {
        if (g_fgets(buffer, BUFFER_SIZE, f) == NULL)  // eof?
        {
            *line = line2;
            return !line2.empty();  // check whether read something
        }

        size_t len = strlen(buffer);
        if (len == 0 || buffer[len - 1] != '\n')
        {
            line2 += buffer;
        }
        else
        {
            buffer[--len] = 0;    // do not include '\n'
            if (len > 0 && buffer[len - 1] == '\r') // do not include '\r'
                buffer[len - 1] = 0;
            line2 += buffer;
            *line = line2;
            return true;    // read at least an '\n'
        }
    }
}

static bool startsWith(const std::string &str1, const std::string &str2)
{
    return (str1.compare(0, str2.size(), str2) == 0) && isspace(str1[str2.size()]);
}

static bool getArg(const std::string &str, const char *key, int *value)
{
    size_t pos = str.find(key);
    if (pos == std::string::npos)
        return false;

    pos += strlen(key);

    while (pos < str.size() && isspace(str[pos]))
           pos++;

    if (str[pos] != '=')
        return false;

    pos++;

    while (pos < str.size() && isspace(str[pos]))
        pos++;

    char *end;
    long result = strtol(str.c_str() + pos, &end, 10);

    if (*end == 0 || isspace(*end))
    {
        *value = result;
        return true;
    }

    return false;
}

int Font::getTextureGlyphsFormat(const char *file)
{
    G_FILE *fis = g_fopen(file, "rt");

    if (!fis)
        throw GiderosException(GStatus(6000, file));	// Error #6000: %s: No such file or directory.

    bool old = (g_fgetc(fis) == '0');

    g_fclose(fis);

    return old ? 0 : 1;
}

void Font::readTextureGlyphsOld(const char* file)
{
	G_FILE* fis = g_fopen(file, "rt");

	if (!fis)
		throw GiderosException(GStatus(6000, file));	// Error #6000: %s: No such file or directory.

    fontInfo_.textureGlyphs.clear();

	int doMipmaps_;
	g_fscanf(fis, "%d", &doMipmaps_);
	while (1)
	{
		TextureGlyph glyph;
		int chr;
		g_fscanf(fis, "%d", &chr);

		if (g_feof(fis))
			break;

		glyph.chr = chr;
		g_fscanf(fis, "%d %d", &glyph.x, &glyph.y);
		g_fscanf(fis, "%d %d", &glyph.width, &glyph.height);
		g_fscanf(fis, "%d %d", &glyph.left, &glyph.top);
		g_fscanf(fis, "%d %d", &glyph.advancex, &glyph.advancey);

        fontInfo_.textureGlyphs[chr] = glyph;
	}

	g_fclose(fis);

    fontInfo_.kernings.clear();
    fontInfo_.isSetTextColorAvailable = true;


    // mimic height & ascender
    int ascender = 0;
    int descender = 0;
    std::map<wchar32_t, TextureGlyph>::iterator iter, e = fontInfo_.textureGlyphs.end();
    for (iter = fontInfo_.textureGlyphs.begin(); iter != e; ++iter)
    {
        ascender = std::max(ascender, iter->second.top);
        descender = std::max(descender, iter->second.height - iter->second.top);
    }

    fontInfo_.height = (ascender + descender) * 1.25;
    fontInfo_.ascender = ascender * 1.25;
}

void Font::readTextureGlyphsNew(const char *file)
{
    G_FILE *fis = g_fopen(file, "rt");

    if (!fis)
        throw GiderosException(GStatus(6000, file));	// Error #6000: %s: No such file or directory.

    std::string line;
    while (readLine(fis, &line))
    {
        if (startsWith(line, "common"))
        {
            if (!getArg(line, "lineHeight", &fontInfo_.height))
                goto error;
            if (!getArg(line, "base", &fontInfo_.ascender))
                goto error;

            fontInfo_.isSetTextColorAvailable = false;
            int alphaChnl, redChnl, greenChnl, blueChnl;
            if (getArg(line, "alphaChnl", &alphaChnl) &&
                getArg(line, "redChnl",   &redChnl)   &&
                getArg(line, "greenChnl", &greenChnl) &&
                getArg(line, "blueChnl",  &blueChnl))
            {
                if (alphaChnl == 0 && redChnl == 4 && greenChnl == 4 && blueChnl == 4)
                {
                    fontInfo_.isSetTextColorAvailable = true;
                }
            }
        }
        else if (startsWith(line, "char"))
        {
            TextureGlyph textureGlyph;

            if (!getArg(line, "id", &textureGlyph.chr))
                goto error;
            if (!getArg(line, "x", &textureGlyph.x))
                goto error;
            if (!getArg(line, "y", &textureGlyph.y))
                goto error;
            if (!getArg(line, "width", &textureGlyph.width))
                goto error;
            if (!getArg(line, "height", &textureGlyph.height))
                goto error;
            if (!getArg(line, "xoffset", &textureGlyph.left))
                goto error;
            if (!getArg(line, "yoffset", &textureGlyph.top))
                goto error;
            if (!getArg(line, "xadvance", &textureGlyph.advancex))
                goto error;

            textureGlyph.top = fontInfo_.ascender - textureGlyph.top;
            textureGlyph.advancex <<= 6;

            fontInfo_.textureGlyphs[textureGlyph.chr] = textureGlyph;
        }
        else if (startsWith(line, "kerning"))
        {
            wchar32_t first, second;
            int amount;

            if (!getArg(line, "first", &first))
                goto error;
            if (!getArg(line, "second", &second))
                goto error;
            if (!getArg(line, "amount", &amount))
                goto error;

            amount <<= 6;

            fontInfo_.kernings[std::make_pair(first, second)] = amount;
        }
    }

    g_fclose(fis);
    return;

error:
    g_fclose(fis);
    throw GiderosException(GStatus(6016, file));    // Error #6016: %s: Error while reading FNT file.
}

Font::~Font()
{
    if (data_)
        application_->getTextureManager()->destroyTexture(data_);
}

void Font::getBounds(const char *text, float letterSpacing, float *pminx, float *pminy, float *pmaxx, float *pmaxy)
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

        x += letterSpacing / sizescalex_;
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

float Font::getAdvanceX(const char *text, float letterSpacing, int size)
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

        x += letterSpacing / sizescalex_;
    }

    x += kerning(prev, wtext[size]) >> 6;

    return x * sizescalex_;
}

int Font::kerning(wchar32_t left, wchar32_t right) const
{
    std::map<std::pair<wchar32_t, wchar32_t>, int>::const_iterator iter;
    iter = fontInfo_.kernings.find(std::make_pair(left, right));
    return (iter != fontInfo_.kernings.end()) ? iter->second : 0;
}

float Font::getAscender()
{
    return fontInfo_.ascender * sizescaley_;
}

float Font::getLineHeight()
{
    return fontInfo_.height * sizescaley_;
}


