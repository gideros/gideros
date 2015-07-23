#include "texturebase.h"
#include "application.h"

TextureBase::TextureBase(Application* application) : application_(application)
{
	data = NULL;
	sizescalex = 1;
	sizescaley = 1;
	uvscalex = 1;
	uvscaley = 1;
}

TextureBase::TextureBase(	Application* application,
                            const char* filename, Filter filter, Wrap wrap, Format format,
							bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/) :
	application_(application)
{

	TextureParameters parameters;
	parameters.filter = filter;
	parameters.wrap = wrap;
    parameters.format = format;
	parameters.maketransparent = maketransparent;
	parameters.transparentcolor = transparentcolor;

	data = application_->getTextureManager()->createTextureFromFile(filename, parameters);
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
