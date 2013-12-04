#include "texturepack.h"
#include "dib.h"
#include "texturepacker.h"
#include "bitmapdata.h"
#include "ogl.h"
#include <gfile.h>
#include <gstdio.h>
#include <pystring.h>
#include "application.h"
#include "giderosexception.h"
#include <string.h>

TexturePack::TexturePack(Application* application,
                         const char* texturelistfile, const char* imagefile, Filter filter, Wrap wrap, Format format,
						 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/) :
    TextureBase(application, imagefile, filter, wrap, format, maketransparent, transparentcolor)
{
    float scale;
    const char *suffix = application->getImageSuffix(imagefile, &scale);

    const char *ext = strrchr(texturelistfile, '.');
    if (ext == NULL)
        ext = texturelistfile + strlen(texturelistfile);

    std::string texturelistfilex = std::string(texturelistfile, ext - texturelistfile) + (suffix ? suffix : "") + ext;

    G_FILE *f = g_fopen(texturelistfilex.c_str(), "r");
    if (f)
        g_fclose(f);

    if (f)
    {
        readTextureList(texturelistfilex.c_str(), textures_, filenameMap_);
        sizescalex = 1 / scale;
        sizescaley = 1 / scale;
        uvscalex = 1;
        uvscaley = 1;
    }
    else
    {
        readTextureList(texturelistfile, textures_, filenameMap_);
        // sizescalex, sizescaley, uvscalex, uvscaley are set at TextureBase's constructor
    }
}

TexturePack::TexturePack(Application* application,
                         const char** filenames, int padding, Filter filter, Wrap wrap, Format format,
						 bool maketransparent/* = false*/, unsigned int transparentcolor/* = 0x00000000*/) :
	TextureBase(application)
{
	std::vector<Dib> dibs;

	int count = 0;
	while (*filenames)
	{
		dibs.push_back(Dib(application, *filenames, true, false, maketransparent, transparentcolor));
		filenameMap_[*filenames] = count++;
		filenames++;
	}

	if (dibs.empty() == false)
	{
		TexturePacker* tp = createTexturePacker();

		tp->setTextureCount((int)dibs.size());
		for (std::size_t i = 0; i < dibs.size(); ++i)
			tp->addTexture(dibs[i].width(), dibs[i].height());

		tp->packTextures(&this->data->width, &this->data->height, padding);
		this->data->exwidth = this->data->width;
		this->data->exheight = this->data->height;

		Dib texture(application, data->width, data->height);
		//unsigned char* texture = new unsigned char[width * height * 4];
		//int texwidth = width;
		//int texheight = height;

		for (int i = 0; i < (int)dibs.size(); ++i)
		{
			int xo, yo;
			int width, height;
			tp->getTextureLocation(i, &xo, &yo, &width, &height);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x)
				{
					unsigned char rgba[4];
					dibs[i].getPixel(x, y, rgba);
					texture.setPixel(xo + x, yo + y, rgba);
				}

			textures_.push_back(Rect(xo, yo, width, height));
		}

		TextureParameters parameters;
		parameters.filter = filter;
		parameters.wrap = wrap;
        parameters.format = format;

		data = application->getTextureManager()->createTextureFromDib(texture, parameters);

		releaseTexturePacker(tp);
	}
}

TexturePack::~TexturePack()
{
}

bool TexturePack::location(int index, int* x, int* y, int* width, int* height, int* dx1, int* dy1, int* dx2, int* dy2) const
{
	if (index < 0 || index >= (int)textures_.size())
		return false;

	if (x)
		*x = textures_[index].x;
	if (y)
		*y = textures_[index].y;
	if (width)
		*width = textures_[index].width;
	if (height)
		*height = textures_[index].height;
	if (dx1)
		*dx1 = textures_[index].dx1;
	if (dy1)
		*dy1 = textures_[index].dy1;
	if (dx2)
		*dx2 = textures_[index].dx2;
	if (dy2)
		*dy2 = textures_[index].dy2;

	return true;
}

bool TexturePack::location(const char* filename, int* x, int* y, int* width, int* height, int* dx1, int* dy1, int* dx2, int* dy2) const
{
	std::map<std::string, int>::const_iterator iter = filenameMap_.find(filename);

	if (iter == filenameMap_.end())
		return false;

	location(iter->second, x, y, width, height, dx1, dy1, dx2, dy2);

	return true;
}


void TexturePack::readTextureList(const char* texturelistfile,
								  std::vector<Rect>& textures_,
								  std::map<std::string, int>& filenameMap_,
								  int* pwidth, int* pheight)
{
	G_FILE* fis = g_fopen(texturelistfile, "rt");

	if (fis == NULL)
	{
		throw GiderosException(GStatus(6000, texturelistfile));		// Error #6000: %s: No such file or directory.
		return;
	}

	textures_.clear();
	filenameMap_.clear();

	int width = 0;
	int height = 0;

	char line[1024];

	while (true)
	{
		line[0] = line[1023] = 0;
		if (g_fgets(line, 1024, fis) == NULL)
			break;

		char* c;
		if ((c = strrchr(line, 0xa)))
			*c = '\0';
		if ((c = strrchr(line, 0xd)))
			*c = '\0';

		if (line[0] == '\0')
			break;

		std::vector<std::string> result;
		pystring::split(line, result, ",");

		for (std::size_t i = 0; i < result.size(); ++i)
		{
			if (result[i].empty() == false)
				result[i] = pystring::strip(result[i]);
		}

		if (result.size() >= 9)
		{
			Rect rect;

			rect.x = atoi(result[1].c_str());
			rect.y = atoi(result[2].c_str());
			rect.width = atoi(result[3].c_str());
			rect.height = atoi(result[4].c_str());
			rect.dx1 = atoi(result[5].c_str());
			rect.dy1 = atoi(result[6].c_str());
			rect.dx2 = atoi(result[7].c_str());
			rect.dy2 = atoi(result[8].c_str());

			filenameMap_[result[0]] = textures_.size();
			textures_.push_back(rect);

			width += rect.width + rect.dx1 + rect.dx2;
			height += rect.height + rect.dy1 + rect.dy2;
		}
	}

	g_fclose(fis);

	if (pwidth)
		*pwidth = width;
	if (pheight)
		*pheight = height;

	if (textures_.empty() == true)
	{
		throw GiderosException(GStatus(6008, texturelistfile));		// Error #6008: %s: File does not contain texture region information.
		return;
	}
}
