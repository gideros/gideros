#ifndef TEXTURE_H
#define TEXTURE_H

#include "texturebase.h"
#include <functional>

class Texture : public TextureBase
{
public:
    Texture(Application* application);
	// can throw GiderosException
	Texture(Application* application,
            const char* filename, TextureParameters parameters, bool pow2=true);
	Texture(Application* application,
			const unsigned char* pixels, unsigned int width, unsigned int height, TextureParameters parameters, bool pow2=true, float scale=1);
	virtual ~Texture();
    static void loadAsync(Application* application,
                    const char* filename, TextureParameters parameters, bool pow2, std::function<void(Texture *,std::exception_ptr)> callback);

};



#endif
