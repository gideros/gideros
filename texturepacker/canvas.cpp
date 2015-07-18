#include "canvas.h"
#include <QPainter>
#include "texturepacker.h"
#include <QColor>
#include <limits.h>

Canvas::Canvas(QWidget *parent)
	: QWidget(parent)
{
	QImage brushImage(64, 64, QImage::Format_RGB32);

	for (int y = 0; y < 64; ++y)
		for (int x = 0; x < 64; ++x)
		{
			int u = x / 32;
			int v = y / 32;
			int b = (u ^ v) & 1;

			brushImage.setPixel(QPoint(x, y), b ? 0xb0b0b0 : 0xe0e0e0);
		}

	brush_ = QBrush(brushImage);
}

Canvas::~Canvas()
{

}

void Canvas::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setBackground(brush_);
	painter.eraseRect(0, 0, width(), height());

	int x = (width() - texture_.width()) / 2;
	int y = (height() - texture_.height()) / 2;

	x = std::max(x, 0);
	y = std::max(y, 0);

	painter.drawImage(QPoint(x, y), texture_);
}

static QImage removeBorder(const QImage& image, float alpha, int* minx, int* miny, int* maxx, int* maxy)
{
	*minx = INT_MAX;
	*miny = INT_MAX;
	*maxx = INT_MIN;
	*maxy = INT_MIN;

	alpha = 254 * alpha + 0.5f; // [0.5, 254.5]

	for (int y = 0; y < image.height(); ++y)
		for (int x = 0; x < image.width(); ++x)
		{
			unsigned char a = image.pixel(x, y) >> 24;

			if (a >= alpha)	// don't discard
			{
				*minx = std::min(*minx, x);
				*miny = std::min(*miny, y);
				*maxx = std::max(*maxx, x);
				*maxy = std::max(*maxy, y);
			}
		}

	if (*minx <= *maxx && *miny <= *maxy)
		return image.copy(*minx, *miny, *maxx - *minx + 1, *maxy - *miny + 1);

	return QImage();
}

inline void copyPixel(QImage& image, int sx, int sy, int dx, int dy, bool transparent)
{
	QRgb rgba = image.pixel(sx, sy);
    if (transparent)
        rgba &= 0x00ffffff;
	image.setPixel(dx, dy, rgba);
}

QString Canvas::packTextures(const QStringList& textureFileNames, const QStringList& names, int padding, int extrude, bool removeAlphaBorder, float alphaThreshold, bool forceSquare, bool showUnusedAreas)
{
	QList<QImage> images;
	QList<QPoint> deltamin;
	QList<QPoint> deltamax;
	for (int i = 0; i < textureFileNames.size(); ++i)
	{
		QImage image;
		if (image.load(textureFileNames[i]))
		{
			int owidth = image.width();
			int oheight = image.height();

			int minx, miny, maxx, maxy;
			image = removeBorder(image, removeAlphaBorder ? alphaThreshold : -1, &minx, &miny, &maxx, &maxy);

			if (minx <= maxx && miny <= maxy)
			{
				images.push_back(image);
				deltamin.push_back(QPoint(minx, miny));
				deltamax.push_back(QPoint(owidth - 1 - maxx, oheight - 1 - maxy));
			}
			else
			{
				// TODO: give a warning here (bence burada transparan texture'u direk atlayabiliriz. 1x1'e gerek yok)
			}
		}
		else
		{
			// TODO: give a warning here (bence burada yuklenmeyen texture'u direk atlayabiliriz. 1x1'e gerek yok)
		}
	}

	if (images.empty())
	{
		texture_ = QImage();
		update();
        return QString();
	}

	{
		TexturePacker* tp = createTexturePacker();

		tp->setTextureCount(images.size());
		for (int i = 0; i < images.size(); ++i)
			tp->addTexture(images[i].width(), images[i].height());

        int width = 0, height = 0;
        tp->packTextures(&width, &height, padding + extrude, forceSquare);

        if (width == 0 || height == 0)
        {
            texture_ = QImage();
            releaseTexturePacker(tp);
            update();
            return "Resulting texture is bigger than 4096x4096.";
        }

		texture_ = QImage(width, height, QImage::Format_ARGB32);

		if (showUnusedAreas == true)
			texture_.fill(0x80ff0000);
		else
			texture_.fill(0x00000000);

		textureLocationList_.clear();

		for (int i = 0; i < images.size(); ++i)
		{
			int xo, yo;
			int width, height;
			tp->getTextureLocation(i, &xo, &yo, &width, &height);

			textureLocationList_ += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n").
										arg(names[i]).
										arg(xo).
										arg(yo).
										arg(width).
										arg(height).
										arg(deltamin[i].x()).
										arg(deltamin[i].y()).
										arg(deltamax[i].x()).
										arg(deltamax[i].y());

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x)
				{
					QRgb rgba = images[i].pixel(x, y);
					texture_.setPixel(xo + x, yo + y, rgba);
				}

            for (int i = 0; i < padding + extrude; ++i)
			{
                bool transparent = i >= extrude;

				for (int x = -i; x < width + i; ++x)
				{
                    copyPixel(texture_, x + xo, -i + yo,              x + xo, -i + yo - 1,      transparent);
                    copyPixel(texture_, x + xo,  i + yo + height - 1, x + xo,  i + yo + height, transparent);
				}

				for (int y = -i; y < height + i; ++y)
				{
                    copyPixel(texture_, -i + xo,             y + yo, -i + xo - 1,     y + yo, transparent);
                    copyPixel(texture_,  i + xo + width - 1, y + yo,  i + xo + width, y + yo, transparent);
				}

                copyPixel(texture_, -i + xo,             -i + yo,              -i + xo - 1,     -i + yo - 1,      transparent);
                copyPixel(texture_,  i + xo + width - 1, -i + yo,               i + xo + width, -i + yo - 1,      transparent);
                copyPixel(texture_, -i + xo,              i + yo + height - 1, -i + xo - 1,      i + yo + height, transparent);
                copyPixel(texture_,  i + xo + width - 1,  i + yo + height - 1,  i + xo + width,  i + yo + height, transparent);
			}
		}

		releaseTexturePacker(tp);

        update();

        return QString("Size of the texture is %1x%2.").arg(width).arg(height);
    }
}




void Canvas::clear()
{
	texture_ = QImage();
	update();
}

