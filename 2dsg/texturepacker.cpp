#include "texturepacker.h"
#include "MaxRectsBinPack.h"
#include <stdlib.h>
#include <algorithm>

class MyTexturePacker : public TexturePacker
{
public:
	MyTexturePacker()
	{
		textureindex_ = 0;
	}

	virtual ~MyTexturePacker() {}
	virtual void setTextureCount(int tcount)
	{
		rects_.resize(tcount);
		textureindex_ = 0;
	}

	virtual void addTexture(int width, int height)
	{
		rects_[textureindex_].width = width;
		rects_[textureindex_++].height = height;
	}

	virtual void packTextures(int* pwidth, int* pheight, int border, bool forceSquare)
	{
		for (size_t i = 0; i < rects_.size(); ++i)
		{
			rects_[i].width += border * 2;
			rects_[i].height += border * 2;
		}

		int maxWidth = 0;
		int maxHeight = 0;
		int totalArea = 0;
		for (size_t i = 0; i < rects_.size(); ++i)
		{
			int width = rects_[i].width;
			int height = rects_[i].height;

			maxWidth = std::max(maxWidth, width);
			maxHeight = std::max(maxHeight, height);
			totalArea += width * height;
		}

		int max = 11;		// 2^11 = 2048
		for (int i = 0; i < max * 2 + 1; ++i)
		{
			int jmax = max - abs(max - i) + 1;
			int jstart = (i < max) ? 0 : (i - max);
			for (int j = 0; j < jmax; ++j)
			{
				int x = (jmax / 2) + offset(j) + jstart;		// j + jstart
				int y = i - x;

				if (forceSquare && (x != y))
					continue;

				int width = 1 << x;
				int height = 1 << y;
				int area = width * height;

				if (width < maxWidth || height < maxHeight || area < totalArea)
					continue;

				packHelper2(width, height);

				if (result_.size() == rects_.size())
				{
					*pwidth = width;
					*pheight = height;

					for (size_t i = 0; i < result_.size(); ++i)
					{
						result_[i].width -= 2 * border;
						result_[i].height -= 2 * border;
						result_[i].x += border;
						result_[i].y += border;
					}

					return;
				}
			}
		}
	}

	virtual void getTextureLocation(int index, int* x, int* y, int* width, int* height)
	{
		*x = result_[index].x;
		*y = result_[index].y;
		*width = result_[index].width;
		*height = result_[index].height;
	}

private:
	// 0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -6, 6 ...
	inline int offset(int i)
	{
		return ((i + 1) / 2) * ((i % 2) ? -1 : 1);
	}

	void packHelper1(int width, int height, MaxRectsBinPack::FreeRectChoiceHeuristic method)
	{
		result_.clear();
		binPack_.Init(width, height);
		for (size_t i = 0; i < rects_.size(); ++i)
		{
			Rect rect = binPack_.Insert(rects_[i].width, rects_[i].height, method);
			if (rect.height == 0)
				break;
			result_.push_back(rect);
		}
	}

	void packHelper2(int width, int height)
	{
		packHelper1(width, height, MaxRectsBinPack::RectBestShortSideFit);
		if (result_.size() == rects_.size())
			return;
		packHelper1(width, height, MaxRectsBinPack::RectBestLongSideFit);
		if (result_.size() == rects_.size())
			return;
		packHelper1(width, height, MaxRectsBinPack::RectBestAreaFit);
		if (result_.size() == rects_.size())
			return;
		packHelper1(width, height, MaxRectsBinPack::RectBottomLeftRule);
		if (result_.size() == rects_.size())
			return;
		packHelper1(width, height, MaxRectsBinPack::RectContactPointRule);
		if (result_.size() == rects_.size())
			return;
	}

	std::vector<RectSize> rects_;
	std::vector<Rect> result_;
	int textureindex_;
	MaxRectsBinPack binPack_;
};

TexturePacker* createTexturePacker(void)
{
	MyTexturePacker* m = new MyTexturePacker;
	return static_cast<TexturePacker*>(m);
}

void releaseTexturePacker(TexturePacker *tp)
{
	MyTexturePacker* m = static_cast<MyTexturePacker*>(tp);
	delete m;
}
