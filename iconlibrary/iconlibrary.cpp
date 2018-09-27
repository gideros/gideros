#include "iconlibrary.h"
#include <QDebug>

IconLibrary::IconLibrary()
{
	image_=QPixmap("Resources/images.png");
	//imagex2_.load("Resources/images@x2.png");

	iconMap_["start"] = icon(13, 34);
	iconMap_["start all"] = icon(15, 34);
	iconMap_["debug"] = icon(0, 35);
    iconMap_["step into"] = icon(3, 35);
    iconMap_["step over"] = icon(2,35);
    iconMap_["step return"] = icon(4, 35);
    iconMap_["resume"] = icon(1, 35);
    iconMap_["stop"] = icon(14, 34);
    iconMap_["gamepad"] = icon(12, 34);

	iconMap_["picture"] = icon(0, 21);
    iconMap_["picture with magnifier"] = icon(0, 21, 0, 1, 3, 3);
    iconMap_["lua"] = icon(14, 18);
    iconMap_["lua with stop"] = icon(14, 18, 4, 0, 4, 4);
    iconMap_["file"] = icon(14, 32);
	iconMap_["folder"] = icon(11, 12);
	iconMap_["folder plugins"] = icon(13, 12);
	iconMap_["folder files"] = icon(6, 13);
	iconMap_["plugin"] = icon(2, 6);
	iconMap_["sound"] = icon(14, 16);

	iconMap_["new"] = icon(3, 272 / 16);
	iconMap_["open"] = icon(6, 208 / 16);
	iconMap_["save"] = icon(0, 34);
	iconMap_["save all"] = icon(1, 34);

	iconMap_["undo"] = icon(3, 34);
	iconMap_["redo"] = icon(4, 34);

	iconMap_["cut"] = icon(5, 34);
	iconMap_["copy"] = icon(6, 34);
	iconMap_["paste"] = icon(7, 34);

	iconMap_["toggle bookmark"] = icon(8, 34);
	iconMap_["next bookmark"] = icon(9, 34);
	iconMap_["previous bookmark"] = icon(10, 34);
	iconMap_["clear bookmarks"] = icon(11, 34);

    iconMap_["dot list"] = icon(12, 27);
    iconMap_["num list"] = icon(13, 27);

    iconMap_["blue dot"] = icon(3, 0);
    iconMap_["green dot"] = icon(11, 0);
    iconMap_["red dot"] = icon(7, 1);
    iconMap_["purple dot"] = icon(5, 1);
    iconMap_["yellow dot"] = icon(15, 1);
    iconMap_["orange dot"] = icon(1, 1);

	iconMap_["project"] = icon(2, 17);

    iconMap_["export"] = icon(2, 34);
    iconMap_["sort cat"] = icon(6, 35);
    iconMap_["sort alpha"] = icon(5, 35);

    //Autocompletion list
    iconMap_["method"] = icon(9,0);
    iconMap_["constant"] = icon(4,0);
    iconMap_["event"] = icon(7,0);
    iconMap_["class"] = icon(11,32);

	danish_["advanced"] = QIcon("Resources/danish/advanced.png");
	danish_["iPhone"] = QIcon("Resources/danish/iPhone.png");
	danish_["start"] = QIcon("Resources/danish/play.png");
	danish_["stop"] = QIcon("Resources/danish/stop_alt.png");
}

IconLibrary::~IconLibrary()
{

}

IconLibrary& IconLibrary::instance()
{
	static IconLibrary iconLibrary;
	return iconLibrary;
}

const QIcon& IconLibrary::icon(int category, const QString& name) const
{
	static QIcon emptyIcon;

	if (category == 0)
	{
		std::map<QString, QIcon>::const_iterator iter = iconMap_.find(name);
		if (iter != iconMap_.end())
			return iter->second;
	}
	else if (category == 1)
	{
		std::map<QString, QIcon>::const_iterator iter = danish_.find(name);
		if (iter != danish_.end())
			return iter->second;
	}

	return emptyIcon;
}

QIcon IconLibrary::icon(int i, int j) const
{
	const int width = 16;
	const int height = 16;

	QIcon icon(image_.copy(i * width, j * height, width, height));
	return icon;
}

static QImage blend(const QImage& i0, const QImage& i1, int dx, int dy)
{
	QImage result = i0;

	for (int y = 0; y < i0.height(); ++y)
		for (int x = 0; x < i0.width(); ++x)
		{
			if (i1.valid(x - dx, y - dy) == false)
				continue;

			QRgb p0 = i0.pixel(x, y);
			QRgb p1 = i1.pixel(x - dx, y - dy);

			int r0 = qRed(p0);			// destination
			int g0 = qGreen(p0);
			int b0 = qBlue(p0);
			int a0 = qAlpha(p0);

			int r1 = qRed(p1);			// source
			int g1 = qGreen(p1);
			int b1 = qBlue(p1);
			int a1 = qAlpha(p1);

			int c1 = a1;			// sfactor = SRC_ALPHA
			int c0 = 255 - a1;		// dfactor = GL_ONE_MINUS_SRC_ALPHA

			int r = (r0 * c0 + r1 * c1) / 255;
			int g = (g0 * c0 + g1 * c1) / 255;
			int b = (b0 * c0 + b1 * c1) / 255;
			int a = (a0 * c0 + a1 * c1) / 255;

			result.setPixel(x, y, qRgba(r, g, b, a));
		}

	return result;
}

QIcon IconLibrary::icon(int i0, int j0, int i1, int j1, int dx, int dy) const
{
	const int width = 16;
	const int height = 16;

	QPixmap image0 = image_.copy(i0 * width, j0 * height, width, height);
	QPixmap image1 = image_.copy(i1 * width, j1 * height, width, height);
	QIcon icon(QPixmap::fromImage(blend(image0.toImage(), image1.toImage(), dx, dy)));

	return icon;
}
