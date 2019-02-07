#include "texture.h"

Texture::Texture(Application* application,
                 const char* filename, Filter filter, Wrap wrap, Format format,
				 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/, bool pow2) :
    TextureBase(application, filename, filter, wrap, format, maketransparent, transparentcolor, pow2)
{
}

Texture::Texture(Application* application,
				 const unsigned char* pixels, unsigned int width, unsigned int height, Filter filter, Wrap wrap, Format format,
				 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/, bool pow2, float scale) :
    TextureBase(application, pixels, width, height, filter, wrap, format, maketransparent, transparentcolor, pow2, scale)
{
}

Texture::~Texture()
{

}

