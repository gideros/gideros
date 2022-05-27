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
#include "graphicsbase.h"
#include <string.h>
#include <utf8.h>
#include <algorithm>

TexturePack::TexturePack(Application* application) : TextureBase(application)
{
}

TexturePack::TexturePack(Application* application,
                         const char* texturelistfile, const char* imagefile, TextureParameters parameters) :
    TextureBase(application, imagefile, parameters)
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
                         const char** filenames, int padding, TextureParameters parameters) :
	TextureBase(application)
{
	std::vector<Dib> dibs;

	int count = 0;
	while (*filenames)
	{
		dibs.push_back(Dib(application, *filenames, true, false, parameters.maketransparent, parameters.transparentcolor));
		filenameMap_[*filenames] = count++;
		filenames++;
	}

	if (dibs.empty() == false)
	{
		TexturePacker* tp = createTexturePacker();

		tp->setTextureCount((int)dibs.size());
		for (std::size_t i = 0; i < dibs.size(); ++i)
			tp->addTexture(dibs[i].width(), dibs[i].height());

        int pwidth, pheight;
        tp->packTextures(&pwidth, &pheight, padding);

        Dib texture(application, pwidth, pheight);
        static unsigned char transparent[4]={0,0,0,0};
        texture.fill(transparent);
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
		data = application->getTextureManager()->createTextureFromDib(texture, parameters);

		releaseTexturePacker(tp);
	}
}

TexturePack::~TexturePack()
{
}

void TexturePack::loadAsync(Application* application, const char** filenames, int padding, TextureParameters parameters,
                            std::function<void(TexturePack *,std::exception_ptr)> callback)
{
    TexturePack *pack=new TexturePack(application,filenames,padding,parameters);
    callback(pack,NULL); //Still synchronous, is it worth making an async version of this one ?
}

void TexturePack::loadAsync(Application* application, const char* texturelistfile, const char* imagefile, TextureParameters parameters,
                      std::function<void(TexturePack *,std::exception_ptr)> callback)
{
    auto future = application->getTextureManager()->createTextureFromFile(imagefile, parameters,
                  [=](TextureData *data, std::exception_ptr e){
        if (e) {
            callback(NULL,e);
        }
        else {
            TexturePack *tex=new TexturePack(application);
            tex->data=data;
            tex->uvscalex = (float)data->width / (float)data->baseWidth;
            tex->uvscaley = (float)data->height / (float)data->baseHeight;

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
                readTextureList(texturelistfilex.c_str(), tex->textures_, tex->filenameMap_);
                tex->sizescalex = 1 / scale;
                tex->sizescaley = 1 / scale;
                tex->uvscalex = 1;
                tex->uvscaley = 1;
            }
            else
            {
                tex->readTextureList(texturelistfile, tex->textures_, tex->filenameMap_);
                // sizescalex, sizescaley, uvscalex, uvscaley are set at TextureBase's constructor
            }
            callback(tex,nullptr);
        }
       });
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

std::vector<std::string> TexturePack::getRegionsNames() {
	std::vector<std::string> names;
	for (auto it = filenameMap_.begin(); it != filenameMap_.end(); ++it)
		names.push_back(it->first);
	return names;
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


TexturePackFont::TexturePackFont(Application *application, TexturePack *pack, std::map<wchar32_t,std::string> mappings, double scale, double anchory) :
        BMFontBase(application) {
    mappings_=mappings;
    pack_=pack;
    pack->ref();
    anchory_=anchory;
    sizescalex_ = scale;
    sizescaley_ = scale;

    fontInfo_.isSetTextColorAvailable = true;

    int ascender = 0;
    int descender = 0;
    std::map<wchar32_t, std::string>::iterator iter, e =
            mappings_.end();
    for (iter = mappings_.begin(); iter != e; ++iter) {
        const std::string &name = iter->second;

        int left,top,width,height;
        int dx1,dy1,dx2,dy2;
        pack_->location(name.c_str(),&left,&top,&width,&height,&dx1,&dy1,&dx2,&dy2);
        int asc=(int)(height*anchory_);
        int desc=height-asc;
        ascender = std::max(ascender, asc);
        descender = std::max(descender, desc);
    }

    fontInfo_.height = (ascender + descender);
    fontInfo_.ascender = ascender;
    fontInfo_.descender = descender;

}

void TexturePackFont::drawText(std::vector<GraphicsBase> * vGraphicsBase, const char* text,
        float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample,
        float minx, float miny, TextLayout &l) {
    G_UNUSED(hasSample);
    size_t size = utf8_to_wchar(text, strlen(text), NULL, 0, 0);

    if (!(l.styleFlags&TEXTSTYLEFLAG_SKIPLAYOUT))
        layoutText(text, layout, l);

    if (size == 0) {
        return;
    }

    int gfx = vGraphicsBase->size();
    vGraphicsBase->resize(gfx+1);
    GraphicsBase *graphicsBase = &((*vGraphicsBase)[gfx]);

    graphicsBase->data = pack_->data;
    if (fontInfo_.isSetTextColorAvailable)
    {
        if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
        {
            graphicsBase->colors.resize(size * 16);
            graphicsBase->colors.Update();
        }
        else
            graphicsBase->setColor(r, g, b, a);
    }
    else
    {
        graphicsBase->setColor(1, 1, 1, 1);
        l.styleFlags&=(~TEXTSTYLEFLAG_COLOR);
    }
    graphicsBase->vertices.resize(size * 4);
    graphicsBase->texcoords.resize(size * 4);
    graphicsBase->indices.resize(size * 6);
    graphicsBase->vertices.Update();
    graphicsBase->texcoords.Update();
    graphicsBase->indices.Update();

    size_t gi=0;
    for (size_t pn = 0; pn < l.parts.size(); pn++) {
        ChunkLayout c = l.parts[pn];
        unsigned char rgba[4]={1,1,1,1};
        if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
        {
            float ca=(c.style.styleFlags&TEXTSTYLEFLAG_COLOR)?(1.0/255)*((c.style.color>>24)&0xFF):a;
            rgba[0]=(unsigned char)(ca*((c.style.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.style.color>>16)&0xFF:r*255));
            rgba[1]=(unsigned char)(ca*((c.style.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.style.color>>8)&0xFF:g*255));
            rgba[2]=(unsigned char)(ca*((c.style.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.style.color>>0)&0xFF:b*255));
            rgba[3]=(unsigned char)(ca*255);
        }
        std::basic_string<wchar32_t> wtext;
        size_t wsize = utf8_to_wchar(c.text.c_str(), c.text.size(), NULL, 0, 0);
        wtext.resize(wsize);
        utf8_to_wchar(c.text.c_str(), c.text.size(), &wtext[0], wsize, 0);

        float x = (c.dx-minx)/sizescalex_, y = (c.dy-miny)/sizescaley_;

        for (size_t i = 0; i < wsize; ++i) {
            std::map<wchar32_t, std::string>::const_iterator iter =
                    mappings_.find(wtext[i]);

            if (iter == mappings_.end())
                continue;

            const std::string &name = iter->second;

            int left,top,width,height;
            int dx1,dy1,dx2,dy2;
            pack_->location(name.c_str(),&left,&top,&width,&height,&dx1,&dy1,&dx2,&dy2);

            float x0 = x;
            float y0 = y - height*anchory_;
            float x1 = x + width;
            float y1 = y0 + height;

            graphicsBase->vertices[gi * 4 + 0] = Point2f(sizescalex_ * x0,
                    sizescaley_ * y0);
            graphicsBase->vertices[gi * 4 + 1] = Point2f(sizescalex_ * x1,
                    sizescaley_ * y0);
            graphicsBase->vertices[gi * 4 + 2] = Point2f(sizescalex_ * x1,
                    sizescaley_ * y1);
            graphicsBase->vertices[gi * 4 + 3] = Point2f(sizescalex_ * x0,
                    sizescaley_ * y1);

            float u0 = (float) left / (float) pack_->data->exwidth;
            float v0 = (float) top / (float) pack_->data->exheight;
            float u1 = (float) (left + width)
                    / (float) pack_->data->exwidth;
            float v1 = (float) (top + height)
                    / (float) pack_->data->exheight;

            u0 *= pack_->uvscalex;
            v0 *= pack_->uvscaley;
            u1 *= pack_->uvscalex;
            v1 *= pack_->uvscaley;

            graphicsBase->texcoords[gi * 4 + 0] = Point2f(u0, v0);
            graphicsBase->texcoords[gi * 4 + 1] = Point2f(u1, v0);
            graphicsBase->texcoords[gi * 4 + 2] = Point2f(u1, v1);
            graphicsBase->texcoords[gi * 4 + 3] = Point2f(u0, v1);

            if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
            {
                for (int v=0;v<16;v+=4) {
                    graphicsBase->colors[gi * 16 + 0 + v] = rgba[0];
                    graphicsBase->colors[gi * 16 + 1 + v] = rgba[1];
                    graphicsBase->colors[gi * 16 + 2 + v] = rgba[2];
                    graphicsBase->colors[gi * 16 + 3 + v] = rgba[3];
                }
            }

            graphicsBase->indices[gi * 6 + 0] = gi * 4 + 0;
            graphicsBase->indices[gi * 6 + 1] = gi * 4 + 1;
            graphicsBase->indices[gi * 6 + 2] = gi * 4 + 2;
            graphicsBase->indices[gi * 6 + 3] = gi * 4 + 0;
            graphicsBase->indices[gi * 6 + 4] = gi * 4 + 2;
            graphicsBase->indices[gi * 6 + 5] = gi * 4 + 3;

            x += width;

            x += layout->letterSpacing / sizescalex_;
            gi++;
        }
    }
    graphicsBase->vertices.resize(gi * 4);
    graphicsBase->texcoords.resize(gi * 4);
    graphicsBase->indices.resize(gi * 6);
}

TexturePackFont::~TexturePackFont() {
    pack_->unref();
}

void TexturePackFont::getBounds(const char *text, float letterSpacing, float *pminx,
        float *pminy, float *pmaxx, float *pmaxy, std::string name) {
    G_UNUSED(name);
    float minx = 1e30;
    float miny = 1e30;
    float maxx = -1e30;
    float maxy = -1e30;

    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0) {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    float x = 0, y = 0;
    for (std::size_t i = 0; i < wtext.size(); ++i) {
        std::map<wchar32_t, std::string>::const_iterator iter =
                mappings_.find(wtext[i]);

        if (iter == mappings_.end())
            continue;

        const std::string &name = iter->second;

        int left,top,width,height;
        int dx1,dy1,dx2,dy2;
        pack_->location(name.c_str(),&left,&top,&width,&height,&dx1,&dy1,&dx2,&dy2);

        float x0 = x;
        float y0 = y - height*anchory_;
        float x1 = x + width;
        float y1 = y0 + height;

        minx = std::min(minx, sizescalex_ * x0);
        minx = std::min(minx, sizescalex_ * x1);
        miny = std::min(miny, sizescaley_ * y0);
        miny = std::min(miny, sizescaley_ * y1);
        maxx = std::max(maxx, sizescalex_ * x0);
        maxx = std::max(maxx, sizescalex_ * x1);
        maxy = std::max(maxy, sizescaley_ * y0);
        maxy = std::max(maxy, sizescaley_ * y1);

        x += width;

        x += letterSpacing / sizescalex_;
    }

    if (!x) minx=miny=maxx=maxy=0;

    if (pminx)
        *pminx = minx;
    if (pminy)
        *pminy = miny;
    if (pmaxx)
        *pmaxx = maxx;
    if (pmaxy)
        *pmaxy = maxy;
}

float TexturePackFont::getAdvanceX(const char *text, float letterSpacing, int size, std::string name) {
    G_UNUSED(name);
    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0) {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    if (size < 0 || size > (int)wtext.size())
        size = wtext.size();

    wtext.push_back(0);

    float x = 0;
    for (int i = 0; i < size; ++i) {
        std::map<wchar32_t, std::string>::const_iterator iter =
                mappings_.find(wtext[i]);

        if (iter == mappings_.end())
            continue;

        const std::string &name = iter->second;

        int left,top,width,height;
        int dx1,dy1,dx2,dy2;
        pack_->location(name.c_str(),&left,&top,&width,&height,&dx1,&dy1,&dx2,&dy2);

        x += width;

        x += letterSpacing / sizescalex_;
    }

    return x * sizescalex_;
}

float TexturePackFont::getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size, std::string name)
{
    G_UNUSED(name);
    std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
    if (len != 0) {
        wtext.resize(len);
        utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
    }

    if (size < 0 || size > (int)wtext.size())
        size = wtext.size();

    wtext.push_back(0);
    offset*=sizescalex_;

    float x = 0;
    float px = 0;
    for (int i = 0; i < size; ++i) {
        std::map<wchar32_t, std::string>::const_iterator iter =
                mappings_.find(wtext[i]);

        if (iter == mappings_.end())
            continue;

        const std::string &name = iter->second;

        int left,top,width,height;
        int dx1,dy1,dx2,dy2;
        pack_->location(name.c_str(),&left,&top,&width,&height,&dx1,&dy1,&dx2,&dy2);

        x += width;

        x += letterSpacing / sizescalex_;
        if ((x>px)&&(offset>=px)&&(offset<x))
        {
            const char *tp=text;
            while (i--)
            {
                char fc=*(tp++);
                if ((fc&0xE0)==0xC0) tp++;
                if ((fc&0xF0)==0xE0) tp+=2;
                if ((fc&0xF8)==0xF0) tp+=3;
            }
            float rv=tp-text;
            return rv+(offset-px)/(x-px);
        }
    }

    return strlen(text);
}

float TexturePackFont::getAscender() {
    return fontInfo_.ascender * sizescaley_;
}

float TexturePackFont::getDescender() {
    return fontInfo_.descender * sizescaley_;
}

float TexturePackFont::getLineHeight() {
    return fontInfo_.height * sizescaley_;
}

