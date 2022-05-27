#ifndef TEXTUREPACK_H
#define TEXTUREPACK_H

#include "texturebase.h"
#include "fontbase.h"
#include <map>
#include <string>
#include <vector>
#include <wchar32.h>
#include <gstatus.h>

class GraphicsBase;
struct TextureData;

class BitmapData;

class TexturePack : public TextureBase
{
public:
    TexturePack(Application* application);
    // can throw GiderosException
    TexturePack(Application* application, const char** filenames, int padding, TextureParameters parameters);
    TexturePack(Application* application, const char* texturelistfile, const char* imagefile, TextureParameters parameters);
	virtual ~TexturePack();
    static void loadAsync(Application* application, const char** filenames, int padding, TextureParameters parameters,std::function<void(TexturePack *,std::exception_ptr)> callback);
    static void loadAsync(Application* application, const char* texturelistfile, const char* imagefile, TextureParameters parameters,std::function<void(TexturePack *,std::exception_ptr)> callback);

    std::vector<std::string> getRegionsNames();
	bool location(int index, int* x, int* y, int* width, int* height, int* dx1, int* dy1, int* dx2, int* dy2) const;
	bool location(const char* filename, int* x, int* y, int* width, int* height, int* dx1, int* dy1, int* dx2, int* dy2) const;

private:
	struct Rect
	{
		Rect()
		{

		}

		Rect(int x, int y, int width, int height, int dx1 = 0, int dy1 = 0, int dx2 = 0, int dy2 = 0) :
			x(x),
			y(y),
			width(width),
			height(height),
			dx1(dx1), dy1(dy1),
			dx2(dx2), dy2(dy2)
		{

		}

		int x, y;
		int width, height;
		int dx1, dy1;
		int dx2, dy2;
	};

protected:
	std::vector<Rect> textures_;
	std::map<std::string, int> filenameMap_;
	static void readTextureList(const char* texturelistfile,
								std::vector<Rect>& textures_,
								std::map<std::string, int>& filenameMap_,
                                int* pwidth = NULL, int* pheight = NULL);
};

class TexturePackFont : public BMFontBase
{
public:
    TexturePackFont(Application *application, TexturePack *pack, std::map<wchar32_t,std::string> mappings, double scale, double anchory);
    virtual ~TexturePackFont();

    virtual Type getType() const
    {
        return eFont;
    }

    virtual void drawText(std::vector<GraphicsBase> * graphicsBase, const char *text, float r, float g, float b, float a, TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l);

    virtual void getBounds(const char *text, float letterSpacing, float *minx, float *miny, float *maxx, float *maxy, std::string name="");
    virtual float getAdvanceX(const char *text, float letterSpacing, int size = -1, std::string name="");
    virtual float getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size = -1, std::string name="");
    virtual float getAscender();
    virtual float getDescender();
    virtual float getLineHeight();

private:
    struct FontInfo
    {
        int height;
        int ascender;
        int descender;
        bool isSetTextColorAvailable;
    };
    TexturePack *pack_;
    std::map<wchar32_t,std::string> mappings_;
    double anchory_;
    float sizescalex_;
    float sizescaley_;

    FontInfo fontInfo_;
};

#endif
