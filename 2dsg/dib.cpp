#include "platform.h"
#include "dib.h"
#include "refptr.h"
#include <gfile.h>
#include "application.h"
#include <pystring.h>
#include "giderosexception.h"
#include <gimage.h>
#include <gstdio.h>

static unsigned int nextpow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

Dib::Dib(Application* application,
		 int width, int height,
		 bool pow2)
{
	originalWidth_ = width;
	originalHeight_ = height;

	if (pow2)
	{
		width_ = nextpow2(originalWidth_);
		height_ = nextpow2(originalHeight_);
	}
	else
	{
		width_ = originalWidth_;
		height_ = originalHeight_;
	}

	baseOriginalWidth_ = originalWidth_;
	baseOriginalHeight_ = originalHeight_;

	data_.resize(width_ * height_ * 4, 0);
}

static void check(int result, const char* file)
{
    switch (result)
    {
    case GIMAGE_NO_ERROR:
        break;
    case GIMAGE_CANNOT_OPEN_FILE:
        throw GiderosException(GStatus(6000, file));
        break;
    case GIMAGE_UNRECOGNIZED_FORMAT:
        throw GiderosException(GStatus(6005, file));
        break;
    case GIMAGE_ERROR_WHILE_READING:
        throw GiderosException(GStatus(6013, file));
        break;
    case GIMAGE_UNSUPPORTED_COLOR_SPACE:
        throw GiderosException(GStatus(6014, file));
        break;
    }
}

Dib::Dib(Application* application,
         const char* file,
         bool withsuffix,
         bool pow2,
         bool maketransparent,
         unsigned int transparentcolor)
{
    int width1, height1;
    int width2, height2;
    int comp;

    std::string filename;

    if (withsuffix)
    {
        const char *ext = strrchr(file, '.');
        if (ext == NULL)
            ext = file + strlen(file);

        float scale;
        const char *suffix = application->getImageSuffix(file, &scale);

        filename = std::string(file, ext - file) + (suffix ? suffix : "") + ext;

        check(gimage_parseImage(filename.c_str(), &width2, &height2, &comp), filename.c_str());

        G_FILE *fis = g_fopen(file, "rb");
        if (fis)
        {
            g_fclose(fis);
            check(gimage_parseImage(file, &width1, &height1, NULL), file);
        }
        else
        {
            width1 = width2 / scale;
            height1 = height2 / scale;
        }
    }
    else
    {
        check(gimage_parseImage(file, &width1, &height1, &comp), file);
        filename = file;
        width2 = width1;
        height2 = height1;
    }

    originalWidth_ = width2;
    originalHeight_ = height2;
    baseOriginalWidth_ = width1;
    baseOriginalHeight_ = height1;

    if (pow2)
    {
        width_ = nextpow2(originalWidth_);
        height_ = nextpow2(originalHeight_);
    }
    else
    {
        width_ = originalWidth_;
        height_ = originalHeight_;
    }

    std::vector<unsigned char> buf(originalWidth_ * originalHeight_ * comp);

    check(gimage_loadImage(filename.c_str(), &buf[0]), filename.c_str());

    data_.resize(width_ * height_ * 4);

    if (comp == 4)
    {// optimization for 4 components case.
        for (int y = 0; y < originalHeight_; ++y)
        {
            int srcindex = (y * originalWidth_) * 4;
            int dstindex = (y * width_) * 4;
            std::copy((const unsigned int*)&buf[srcindex], (const unsigned int*)(buf.data() + srcindex + originalWidth_ * 4),
                    (unsigned int*)&data_[dstindex]);
        }
    }
    else
    {
        for (int y = 0; y < originalHeight_; ++y)
            for (int x = 0; x < originalWidth_; ++x)
            {
                unsigned char rgba[4] = {255, 255, 255, 255};

                int srcindex = (x + y * originalWidth_) * comp;

                switch (comp)
                {
                case 1:
                    rgba[0] = rgba[1] = rgba[2] = buf[srcindex + 0];
                    break;
                case 2:
                    rgba[0] = rgba[1] = rgba[2] = buf[srcindex + 0];
                    rgba[3] =                     buf[srcindex + 1];
                    break;
                case 3:
                    rgba[0] = buf[srcindex + 0];
                    rgba[1] = buf[srcindex + 1];
                    rgba[2] = buf[srcindex + 2];
                    break;
                case 4:
                    rgba[0] = buf[srcindex + 0];
                    rgba[1] = buf[srcindex + 1];
                    rgba[2] = buf[srcindex + 2];
                    rgba[3] = buf[srcindex + 3];
                    break;
                }

                int dstindex = (x + y * width_) * 4;

                data_[dstindex + 0] = rgba[0];
                data_[dstindex + 1] = rgba[1];
                data_[dstindex + 2] = rgba[2];
                data_[dstindex + 3] = rgba[3];
            }
    }

	if (maketransparent)
	{
		unsigned int r = (transparentcolor >> 16) & 0xff;
		unsigned int g = (transparentcolor >> 8) & 0xff;
		unsigned int b = transparentcolor & 0xff;

		for (int y = 0; y < originalHeight_; ++y)
			for (int x = 0; x < originalWidth_; ++x)
			{
				int index = (x + y * width_) * 4;

				if (data_[index + 0] == r &&
					data_[index + 1] == g &&
					data_[index + 2] == b)
				{
					data_[index + 3] = 0;
				}
			}
	}

	intelligentFill();
}

void Dib::intelligentFill()
{
	int midx = (originalWidth_ + width_) / 2;
	int midy = (originalHeight_ + height_) / 2;

    midx = midy = 0x7fffffff;   // currently clamp mode, if this is comment out then repeat mode

	for (int y = 0; y < originalHeight_; ++y)
	{
		for (int x = originalWidth_; x < width_; ++x)
		{
			int indexsrc = (x < midx) ? ((originalWidth_ - 1) + y * width_) * 4 : (y * width_) * 4;
			int indexdst = (x + y * width_) * 4;

			data_[indexdst + 0] = data_[indexsrc + 0];
			data_[indexdst + 1] = data_[indexsrc + 1];
			data_[indexdst + 2] = data_[indexsrc + 2];
			data_[indexdst + 3] = data_[indexsrc + 3]; //0;
		}
	}

	for (int x = 0; x < originalWidth_; ++x)
	{
		for (int y = originalHeight_; y < height_; ++y)
		{
			int indexsrc = (y < midy) ? (x + (originalHeight_ - 1) * width_) * 4 : (x) * 4;
			int indexdst = (x + y * width_) * 4;

			data_[indexdst + 0] = data_[indexsrc + 0];
			data_[indexdst + 1] = data_[indexsrc + 1];
			data_[indexdst + 2] = data_[indexsrc + 2];
			data_[indexdst + 3] = data_[indexsrc + 3]; //0;
		}
	}

	for (int y = originalHeight_; y < height_; ++y)
		for (int x = originalWidth_; x < width_; ++x)
		{
			int indexsrc = 0;
			indexsrc += (x < midx) ? (originalWidth_ - 1) * 4 : 0;
			indexsrc += (y < midy) ? (originalHeight_ - 1) * width_ * 4 : 0;
			int indexdst = (x + y * width_) * 4;

			data_[indexdst + 0] = data_[indexsrc + 0];
			data_[indexdst + 1] = data_[indexsrc + 1];
			data_[indexdst + 2] = data_[indexsrc + 2];
			data_[indexdst + 3] = data_[indexsrc + 3]; //0;
		}
}

void Dib::premultiplyAlpha()
{
    gimage_premultiplyAlpha(width_, height_, &data_[0]);
}

void Dib::convertGrayscale()
{
    unsigned int* iptr = (unsigned int*)&data_[0];

    for (int i = 0; i < width_ * height_; ++i)
        iptr[i] = ((iptr[i] ^ 0x00ff0000) << 8) | 0x00ffffff;
}

std::vector<unsigned short> Dib::to565() const
{
    std::vector<unsigned short> result(width_ * height_);

    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
        {
            int index = x + y * width_;
            unsigned short r = data_[index * 4 + 0];
            unsigned short g = data_[index * 4 + 1];
            unsigned short b = data_[index * 4 + 2];
            result[index] = ((r >> 3) << 11) |
                            ((g >> 2) << 5)  |
                            ((b >> 3) << 0);
        }

    return result;
}

std::vector<unsigned char> Dib::to888() const
{
    std::vector<unsigned char> result(width_ * height_ * 3);

    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
        {
            int index = x + y * width_;
            result[index * 3 + 0] = data_[index * 4 + 0];
            result[index * 3 + 1] = data_[index * 4 + 1];
            result[index * 3 + 2] = data_[index * 4 + 2];
        }

    return result;
}

std::vector<unsigned short> Dib::to4444() const
{
    std::vector<unsigned short> result(width_ * height_);

    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
        {
            int index = x + y * width_;
            unsigned short r = data_[index * 4 + 0];
            unsigned short g = data_[index * 4 + 1];
            unsigned short b = data_[index * 4 + 2];
            unsigned short a = data_[index * 4 + 3];
            result[index] = ((r >> 4) << 12) |
                            ((g >> 4) << 8)  |
                            ((b >> 4) << 4)  |
                            ((a >> 4) << 0);
        }

    return result;
}


std::vector<unsigned short> Dib::to5551() const
{
    std::vector<unsigned short> result(width_ * height_);

    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
        {
            int index = x + y * width_;
            unsigned short r = data_[index * 4 + 0];
            unsigned short g = data_[index * 4 + 1];
            unsigned short b = data_[index * 4 + 2];
            unsigned short a = data_[index * 4 + 3];
            result[index] = ((r >> 3) << 11) |
                            ((g >> 3) << 6)  |
                            ((b >> 3) << 1)  |
                            ((a >> 7) << 0);
        }

    return result;
}
