#ifndef TEXTUREPACK_H
#define TEXTUREPACK_H

#include "texturebase.h"
#include <map>
#include <string>

class BitmapData;

class TexturePack : public TextureBase
{
public:
	// can throw GiderosException
    TexturePack(Application* application, const char** filenames, int padding, Filter filter, Wrap wrap, Format format, bool maketransparent = false, unsigned int transparentcolor = 0x00000000);
    TexturePack(Application* application, const char* texturelistfile, const char* imagefile, Filter filter, Wrap wrap, Format format, bool maketransparent = false, unsigned int transparentcolor = 0x00000000);
	virtual ~TexturePack();

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

	std::vector<Rect> textures_;
	std::map<std::string, int> filenameMap_;
	static void readTextureList(const char* texturelistfile,
								std::vector<Rect>& textures_,
								std::map<std::string, int>& filenameMap_,
                                int* pwidth = NULL, int* pheight = NULL);
};

#endif
