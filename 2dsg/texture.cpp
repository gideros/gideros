#include "texture.h"

Texture::Texture(Application* application,
                 const char* filename, Filter filter, Wrap wrap, Format format,
				 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/) :
    TextureBase(application, filename, filter, wrap, format, maketransparent, transparentcolor)
{
}

Texture::Texture(Application* application,
				 const unsigned char* pixels, unsigned int width, unsigned int height, Filter filter, Wrap wrap, Format format,
				 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/) :
    TextureBase(application, pixels, width, height, filter, wrap, format, maketransparent, transparentcolor)
{
}

Texture::~Texture()
{

}

