#ifndef DIB_H
#define DIB_H

#include <vector>
#include <algorithm>
#include "refptr.h"

class Application;


class Dib
{
public:
	Dib(Application* application,
		int width, int height,
		bool pow2 = false);

	// can throw GiderosException
	Dib(Application* application,
		const char* filename,
        bool withsuffix,
        bool pow2,
        bool maketransparent,
        unsigned int transparentcolor);

	void fill(unsigned char rgba[4])
	{
		int n = width_ * height_;
		for (int i = 0; i < n; ++i)
		{
			int index = i * 4;

			data_[index + 0] = rgba[0];
			data_[index + 1] = rgba[1];
			data_[index + 2] = rgba[2];
			data_[index + 3] = rgba[3];
		}

	}

	int width() const
	{
		return width_;
	}

	int height() const
	{
		return height_;
	}

	unsigned char *dataArray()
	{
		return &data_[0];
	}

	int originalWidth() const
	{
		return originalWidth_;
	}

	int originalHeight() const
	{
		return originalHeight_;
	}

	int baseOriginalWidth() const
	{
		return baseOriginalWidth_;
	}

	int baseOriginalHeight() const
	{
		return baseOriginalHeight_;
	}

	const unsigned char* data() const
	{
		return &data_[0];
	}

	unsigned char* data()
	{
		return &data_[0];
	}

	void getPixel(int x, int y, unsigned char rgba[4])
	{
		int index = (x + y * width_) * 4;

		rgba[0] = data_[index + 0];
		rgba[1] = data_[index + 1];
		rgba[2] = data_[index + 2];
		rgba[3] = data_[index + 3];
	}

	void setPixel(int x, int y, const unsigned char rgba[4])
	{
		int index = (x + y * width_) * 4;

		data_[index + 0] = rgba[0];
		data_[index + 1] = rgba[1];
		data_[index + 2] = rgba[2];
		data_[index + 3] = rgba[3];
	}

	unsigned char getAlpha(int x, int y)
	{
		int index = (x + y * width_) * 4;
		return data_[index + 3];
	}

	void setAlpha(int x, int y, unsigned char a)
	{
		int index = (x + y * width_) * 4;
		data_[index + 3] = a;
	}

	void satAlpha(int x, int y, unsigned char a)
	{
		int index = (x + y * width_) * 4;
		data_[index + 3] = (std::min)(255, data_[index + 3] + a);
	}

	void intelligentFill();

    void premultiplyAlpha();

    void convertGrayscale();

    std::vector<unsigned char> to888() const;
    std::vector<unsigned short> to565() const;
    std::vector<unsigned short> to4444() const;
    std::vector<unsigned short> to5551() const;

private:
	std::vector<unsigned char> data_;

	int width_;
	int height_;
	int originalWidth_;
	int originalHeight_;

	int baseOriginalWidth_;
	int baseOriginalHeight_;
};

#endif
