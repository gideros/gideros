#include "texture.h"
#include "application.h"
#include "dib.h"

Texture::Texture(Application* application) : TextureBase(application)
{
}

Texture::Texture(Application* application,
                 const char* filename, TextureParameters parameters) :
    TextureBase(application, filename, parameters)
{
}

Texture::Texture(Application* application,
                 const unsigned char* pixels, unsigned int width, unsigned int height, TextureParameters parameters, float scale) :
    TextureBase(application, pixels, width, height, parameters, scale)
{
}

Texture::~Texture()
{

}

void Texture::loadAsync(Application* application,
                const char* filename, TextureParameters parameters, std::function<void(Texture *,std::exception_ptr)> callback)
{
    auto future = application->getTextureManager()->createTextureFromFile(filename, parameters,
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

