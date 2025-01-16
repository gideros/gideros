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

    void update(const unsigned char* pixels,unsigned int width,unsigned int height, bool raw=false);

protected:
	TextureBase(Application* application);
	TextureBase(Application* application,
                const char* filename, TextureParameters parameters);
	TextureBase(Application* application,
                const unsigned char* pixels, unsigned int width, unsigned int height, TextureParameters parameters, float scale=1, bool raw=false);

	virtual ~TextureBase();

    Application* application_;
};



#endif
