#include "texturebase.h"
#include "application.h"
#include "dib.h"

TextureBase::TextureBase(Application* application) : application_(application)
{
	data = NULL;
	sizescalex = 1;
	sizescaley = 1;
	uvscalex = 1;
	uvscaley = 1;
}

TextureBase::TextureBase(	Application* application,
                            const char* filename, TextureParameters parameters) :
	application_(application)
{
    data = application_->getTextureManager()->createTextureFromFile(filename, parameters).get();
	sizescalex = 1;
	sizescaley = 1;
    uvscalex = (float)data->width / (float)data->baseWidth;
    uvscaley = (float)data->height / (float)data->baseHeight;
}

TextureBase::TextureBase(	Application* application,
                            const unsigned char* pixels,unsigned int width,unsigned int height, TextureParameters parameters, float scale) :
	application_(application)
{

    Dib dib(application, width, height, parameters.pow2, scale);
	if (pixels)
	for (unsigned int y=0;y<height;y++)
		memcpy(dib.dataArray()+y*dib.width()*4,pixels+y*width*4,width*4);

	data = application_->getTextureManager()->createTextureFromDib(dib, parameters);
	sizescalex = 1;
	sizescaley = 1;
    uvscalex = (float)data->width / (float)data->baseWidth;
    uvscaley = (float)data->height / (float)data->baseHeight;
}

TextureBase::~TextureBase()
{
	if (data)
		application_->getTextureManager()->destroyTexture(data);
}
