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
                const char* filename, TextureParameters parameters, bool pow2=true);
	TextureBase(Application* application,
                const unsigned char* pixels, unsigned int width, unsigned int height, TextureParameters parameters, bool pow2=true, float scale=1);

	virtual ~TextureBase();

    Application* application_;
};



#endif
