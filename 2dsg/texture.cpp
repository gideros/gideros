#include "texture.h"
#include "application.h"
#include "dib.h"

Texture::Texture(Application* application) : TextureBase(application)
{
}

Texture::Texture(Application* application,
                 const char* filename, TextureParameters parameters, bool pow2) :
    TextureBase(application, filename, parameters, pow2)
{
}

Texture::Texture(Application* application,
				 const unsigned char* pixels, unsigned int width, unsigned int height, TextureParameters parameters, bool pow2, float scale) :
    TextureBase(application, pixels, width, height, parameters, pow2, scale)
{
}

Texture::~Texture()
{

}

void Texture::loadAsync(Application* application,
                const char* filename, TextureParameters parameters, bool pow2, std::function<void(Texture *,std::exception_ptr)> callback)
{
    auto future = application->getTextureManager()->createTextureFromFile(filename, parameters,pow2,
                  [=](TextureData *data, std::exception_ptr e){
        if (e) {
            callback(NULL,e);
        }
        else {
            Texture *tex=new Texture(application);
            tex->data=data;
            tex->uvscalex = (float)data->width / (float)data->baseWidth;
            tex->uvscaley = (float)data->height / (float)data->baseHeight;
            callback(tex,nullptr);
        }
       });
}

