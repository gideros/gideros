#ifndef TEXTUREBASE_H
#define TEXTUREBASE_H

#include "texturemanager.h"
#include "refptr.h"

class TextureBase : public GReferenced
{
public:
	float sizescalex;
	float sizescaley;
	float uvscalex;
	float uvscaley;

	TextureData* data;

protected:
	TextureBase(Application* application);
	TextureBase(Application* application,
                const char* filename, Filter filter, Wrap wrap, Format format,
				bool maketransparent = false, unsigned int transparentcolor = 0x00000000);
	TextureBase(Application* application,
                const unsigned char* pixels, unsigned int width, unsigned int height, Filter filter, Wrap wrap, Format format,
				bool maketransparent = false, unsigned int transparentcolor = 0x00000000);

	virtual ~TextureBase();

private:
	Application* application_;
};



#endif
