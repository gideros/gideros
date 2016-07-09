#ifndef TEXTURE_H
#define TEXTURE_H

#include "texturebase.h"

class Texture : public TextureBase
{
public:
	// can throw GiderosException
	Texture(Application* application,
            const char* filename, Filter filter, Wrap wrap, Format format,
			bool maketransparent = false, unsigned int transparentcolor = 0x00000000);
	Texture(Application* application,
			const unsigned char* pixels, unsigned int width, unsigned int height, Filter filter, Wrap wrap, Format format,
			bool maketransparent = false, unsigned int transparentcolor = 0x00000000);
	virtual ~Texture();
};



#endif
