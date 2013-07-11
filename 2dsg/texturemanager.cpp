#include "texturemanager.h"
#include <gtexture.h>
#include <gimage.h>
#include <gpath.h>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <dib.h>
#include <glog.h>
#include "glcommon.h"
#include <application.h>

unsigned int TextureData::id()
{
    return gtexture_getInternalId(gid);
}

static void append(std::vector<char>& buffer, const void *data, size_t len)
{
    size_t s = buffer.size();
    buffer.resize(s + len);
    memcpy(&buffer[s], data, len);
}

static void append(std::vector<char>& buffer, const TextureParameters& parameters)
{
    append(buffer, &parameters.filter, sizeof(parameters.filter));
    append(buffer, &parameters.wrap, sizeof(parameters.wrap));
    append(buffer, &parameters.maketransparent, sizeof(parameters.maketransparent));
    append(buffer, &parameters.transparentcolor, sizeof(parameters.transparentcolor));
    append(buffer, &parameters.grayscale, sizeof(parameters.grayscale));
}

TextureManager::TextureManager(Application* application) :
    application_(application)
{
}

TextureManager::~TextureManager()
{
}

TextureData* TextureManager::createTextureFromFile(const char* filename, const TextureParameters& parameters)
{
    int flags = gpath_getDriveFlags(gpath_getPathDrive(filename));

    std::vector<char> sig;
    if (flags & GPATH_RO)
    {
        append(sig, filename, strlen(filename) + 1);
        append(sig, parameters);
    }
    else
    {
        if (flags & GPATH_REAL)
        {
            struct stat s;
            stat(gpath_transform(filename), &s);

            append(sig, filename, strlen(filename) + 1);
            append(sig, parameters);
            append(sig, &s.st_mtime, sizeof(s.st_mtime));
        }
    }

    int wrap = 0;
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GTEXTURE_CLAMP;
        break;
    case eRepeat:
        wrap = GTEXTURE_REPEAT;
        break;
    }

    int filter = 0;
    switch (parameters.filter)
    {
    case eNearest:
        filter = GTEXTURE_NEAREST;
        break;
    case eLinear:
        filter = GTEXTURE_LINEAR;
        break;
    }

    int format = 0;
    int type = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB888:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB565:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_SHORT_5_6_5;
        break;
    case eRGBA4444:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_4_4_4_4;
        break;
    case eRGBA5551:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_5_5_5_1;
        break;
    }

    if (!sig.empty())
    {
        g_id gid = gtexture_reuse(format, type, wrap, filter, &sig[0], sig.size());
        if (gid != 0)
        {
            TextureData* internal = (TextureData*)gtexture_getUserData(gid);
            TextureData* data = new TextureData(*internal);
            data->gid = gid;

            return data;
        }
    }

    Dib dib(application_, filename, true, true, parameters.maketransparent, parameters.transparentcolor);

    if (parameters.grayscale)
        dib.convertGrayscale();

#if PREMULTIPLIED_ALPHA
    dib.premultiplyAlpha();
#endif

    g_id gid = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, dib.data(), &sig[0], sig.size());
        break;
    case eRGB888:
    {
        std::vector<unsigned char> data = dib.to888();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], &sig[0], sig.size());
        break;
    }
    case eRGB565:
    {
        std::vector<unsigned short> data = dib.to565();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], &sig[0], sig.size());
        break;
    }
    case eRGBA4444:
    {
        std::vector<unsigned short> data = dib.to4444();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], &sig[0], sig.size());
        break;
    }
    case eRGBA5551:
    {
        std::vector<unsigned short> data = dib.to5551();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], &sig[0], sig.size());
        break;
    }
    }

    TextureData* data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = dib.originalWidth();
    data->height = dib.originalHeight();
    data->exwidth = dib.width();
    data->exheight = dib.height();
    data->baseWidth = dib.baseOriginalWidth();
    data->baseHeight = dib.baseOriginalHeight();
    data->baseExWidth = dib.baseWidth();
    data->baseExHeight = dib.baseHeight();

    TextureData* internal = new TextureData(*data);
    gtexture_setUserData(gid, internal);

    return data;
}

TextureData* TextureManager::createTextureFromDib(const Dib& dib, const TextureParameters& parameters)
{
    int wrap = 0;
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GTEXTURE_CLAMP;
        break;
    case eRepeat:
        wrap = GTEXTURE_REPEAT;
        break;
    }

    int filter = 0;
    switch (parameters.filter)
    {
    case eNearest:
        filter = GTEXTURE_NEAREST;
        break;
    case eLinear:
        filter = GTEXTURE_LINEAR;
        break;
    }

    int format = 0;
    int type = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB888:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_BYTE;
        break;
    case eRGB565:
        format = GTEXTURE_RGB;
        type = GTEXTURE_UNSIGNED_SHORT_5_6_5;
        break;
    case eRGBA4444:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_4_4_4_4;
        break;
    case eRGBA5551:
        format = GTEXTURE_RGBA;
        type = GTEXTURE_UNSIGNED_SHORT_5_5_5_1;
        break;
    }


    Dib dib2 = dib;

    if (parameters.grayscale)
        dib2.convertGrayscale();

#if PREMULTIPLIED_ALPHA
    dib2.premultiplyAlpha();
#endif


    g_id gid = 0;
    switch (parameters.format)
    {
    case eRGBA8888:
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, dib2.data(), NULL, 0);
        break;
    case eRGB888:
    {
        std::vector<unsigned char> data = dib2.to888();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGB565:
    {
        std::vector<unsigned short> data = dib2.to565();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGBA4444:
    {
        std::vector<unsigned short> data = dib2.to4444();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    case eRGBA5551:
    {
        std::vector<unsigned short> data = dib2.to5551();
        gid = gtexture_create(dib.width(), dib.height(), format, type, wrap, filter, &data[0], NULL, 0);
        break;
    }
    }

    TextureData* data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = dib.originalWidth();
    data->height = dib.originalHeight();
    data->exwidth = dib.width();
    data->exheight = dib.height();
    data->baseWidth = dib.baseOriginalWidth();
    data->baseHeight = dib.baseOriginalHeight();
    data->baseExWidth = dib.baseWidth();
    data->baseExHeight = dib.baseHeight();

    return data;
}

static unsigned int nextpow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

TextureData* TextureManager::createRenderTarget(int w, int h, const TextureParameters& parameters)
{
    int wrap = 0;
    switch (parameters.wrap)
    {
    case eClamp:
        wrap = GTEXTURE_CLAMP;
        break;
    case eRepeat:
        wrap = GTEXTURE_REPEAT;
        break;
    }

    int filter = 0;
    switch (parameters.filter)
    {
    case eNearest:
        filter = GTEXTURE_NEAREST;
        break;
    case eLinear:
        filter = GTEXTURE_LINEAR;
        break;
    }

    float scalex = application_->getLogicalScaleX();
    float scaley = application_->getLogicalScaleY();

    int baseWidth = w;
    int baseHeight = h;
    int baseExWidth = nextpow2(baseWidth);
    int baseExHeight = nextpow2(baseHeight);
    int width = (int)floor(baseWidth * scalex + 0.5);
    int height = (int)floor(baseHeight * scaley + 0.5);
    int exwidth = nextpow2(width);
    int exheight = nextpow2(height);

    g_id gid = gtexture_RenderTargetCreate(exwidth, exheight, wrap, filter);

    TextureData *data = new TextureData;

    data->gid = gid;
    data->parameters = parameters;
    data->width = width;
    data->height = height;
    data->exwidth = exwidth;
    data->exheight = exheight;
    data->baseWidth = baseWidth;
    data->baseHeight = baseHeight;
    data->baseExWidth = baseExWidth;
    data->baseExHeight = baseExHeight;

    return data;
}

void TextureManager::destroyTexture(TextureData* texture)
{
    TextureData* internal = (TextureData*)gtexture_getUserData(texture->gid);
    if (gtexture_delete(texture->gid))
        if (internal)
            delete internal;
    delete texture;
}

