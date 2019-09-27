#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include <gtexture.h>
#include <map>

class Application;
class Dib;

enum Filter
{
    eNearest,
    eLinear,
};

enum Wrap
{
    eClamp,
    eRepeat,
};

enum Format
{
    eRGBA8888,
    eRGB888,
    eRGB565,
    eRGBA4444,
    eRGBA5551,
	eY8,
	eA8,
	eYA8
};

struct TextureParameters
{
    TextureParameters()
    {
        filter = eNearest;
        wrap = eClamp;
        format = eRGBA8888;
        maketransparent = false;
        transparentcolor = 0x00000000;
        grayscale = false;
    }

    Filter filter;
    Wrap wrap;
    Format format;
    bool maketransparent;
    unsigned int transparentcolor;
    bool grayscale;
};

struct TextureData
{
    ShaderTexture *id();
    g_id gid;
    TextureParameters parameters;
    int width, height;
    int exwidth, exheight;
    int baseWidth, baseHeight;
    float scale;
};


class TextureManager
{
public:
    TextureManager(Application* application);
    ~TextureManager();

    virtual TextureData* createTextureFromFile(const char* filename, const TextureParameters& parameters, bool pow2=true);
    virtual TextureData* createTextureFromDib(const Dib& dib, const TextureParameters& parameters);
    virtual TextureData* createRenderTarget(int width, int height, const TextureParameters& parameters, bool selectScale=false,bool depth=false);
    virtual void updateTextureFromDib(TextureData* data, const Dib& dib);
    virtual void destroyTexture(TextureData* texture);

private:
    Application *application_;
};


#endif
