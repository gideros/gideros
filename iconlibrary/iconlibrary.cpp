#include "iconlibrary.h"
#include <QDebug>

#define iconref(x,y) (((x)<<8)|y)
#define icontag(x,y,dx,dy) (((x)<<8)|(y)|(((dx)+128)<<24)|(((dy)+128)<<16))
IconLibrary::IconLibrary()
{
	image_=QPixmap("Resources/images.png");
    imagex2_=QPixmap("Resources/images@2x.png");

    iconRef_["start"] = iconref(13, 34);
    iconRef_["start all"] = iconref(15, 34);
    iconRef_["debug"] = iconref(0, 35);
    iconRef_["step into"] = iconref(3, 35);
    iconRef_["step over"] = iconref(2,35);
    iconRef_["step return"] = iconref(4, 35);
    iconRef_["resume"] = iconref(1, 35);
    iconRef_["stop"] = iconref(14, 34);
    iconRef_["profiler"] = iconref(10, 35);
    iconRef_["gamepad"] = iconref(12, 34);

    iconRef_["picture"] = iconref(0, 21);
    iconRef_["lua"] = iconref(14, 18);
    iconRef_["file"] = iconref(14, 32);
    iconRef_["folder"] = iconref(11, 12);
    iconRef_["folder plugins"] = iconref(13, 12);
    iconRef_["folder files"] = iconref(6, 13);
    iconRef_["plugin"] = iconref(2, 6);
    iconRef_["sound"] = iconref(14, 16);

    iconRef_["new"] = iconref(3, 272 / 16);
    iconRef_["open"] = iconref(6, 208 / 16);
    iconRef_["save"] = iconref(0, 34);
    iconRef_["save all"] = iconref(1, 34);

    iconRef_["undo"] = iconref(3, 34);
    iconRef_["redo"] = iconref(4, 34);

    iconRef_["cut"] = iconref(5, 34);
    iconRef_["copy"] = iconref(6, 34);
    iconRef_["paste"] = iconref(7, 34);

    iconRef_["toggle bookmark"] = iconref(8, 34);
    iconRef_["next bookmark"] = iconref(9, 34);
    iconRef_["previous bookmark"] = iconref(10, 34);
    iconRef_["clear bookmarks"] = iconref(11, 34);

    iconRef_["dot list"] = iconref(12, 27);
    iconRef_["num list"] = iconref(13, 27);

    iconRef_["blue dot"] = iconref(3, 0);
    iconRef_["green dot"] = iconref(11, 0);
    iconRef_["red dot"] = iconref(7, 1);
    iconRef_["purple dot"] = iconref(5, 1);
    iconRef_["yellow dot"] = iconref(15, 1);
    iconRef_["orange dot"] = iconref(1, 1);

    iconRef_["project"] = iconref(2, 17);

    iconRef_["export"] = iconref(2, 34);
    iconRef_["sort cat"] = iconref(6, 35);
    iconRef_["sort alpha"] = iconref(5, 35);

    //Autocompletion list
    iconRef_["method"] = iconref(9,0);
    iconRef_["constant"] = iconref(4,0);
    iconRef_["event"] = iconref(7,0);
    iconRef_["class"] = iconref(11,32);

    iconMap_["picture with magnifier"] = icon(0, 21, 0, 1, 3, 3);
    iconMap_["lua with stop"] = icon(14, 18, 4, 0, 4, 4);

    tagMap_["stop"]=icontag(4,0,4,4);
    tagMap_["link"]=icontag(15,0,2,-3);
    tagMap_["magnifier"]=icontag(0,1,3,3);

/*
	danish_["advanced"] = QIcon("Resources/danish/advanced.png");
	danish_["iPhone"] = QIcon("Resources/danish/iPhone.png");
	danish_["start"] = QIcon("Resources/danish/play.png");
    danish_["stop"] = QIcon("Resources/danish/stop_alt.png");*/
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

IconLibrary::~IconLibrary()
{

}

IconLibrary& IconLibrary::instance()
{
	static IconLibrary iconLibrary;
	return iconLibrary;
}

const QIcon& IconLibrary::icon(int category, const QString& name)
{
    Q_UNUSED(category);
	static QIcon emptyIcon;

    std::map<QString, QIcon>::const_iterator iter = iconMap_.find(name);
    if (iter != iconMap_.end())
        return iter->second;

    std::map<QString, int>::const_iterator iter2 = iconRef_.find(name);
    if (iter2 != iconRef_.end())
    {
        int ref=iter2->second;
        QIcon i=icon((ref>>8)&0xFF,ref&0xFF);
        iconMap_[name]=i;
        return iconMap_[name];
    }

/*	if (category == 0)
    {
    }
	else if (category == 1)
	{
		std::map<QString, QIcon>::const_iterator iter = danish_.find(name);
		if (iter != danish_.end())
			return iter->second;
    }*/

	return emptyIcon;
}

const QIcon& IconLibrary::icon(const QString& name,const QStringList tags)
{
    QString cname=name;
    foreach(QString t,tags)
        cname=cname+":"+t;

    static QIcon emptyIcon;

    std::map<QString, QIcon>::const_iterator iter = iconMap_.find(cname);
    if (iter != iconMap_.end())
        return iter->second;

    std::map<QString, int>::const_iterator iter2 = iconRef_.find(name);
    if (iter2 != iconRef_.end())
    {
        // Build layered icon
        int ref=iter2->second;
        int width = 16;
        int height = 16;

        QPixmap image = image_.copy(((ref>>8)&0xFF) * width, (ref&0xFF) * height, width, height);
        QPixmap imagex2 = imagex2_.copy(((ref>>8)&0xFF) * width*2, (ref&0xFF) * height*2, width*2, height*2);
        foreach(QString t,tags)
        {
            int tr=tagMap_[t];
            int i1=(tr>>8)&0xFF;
            int j1=(tr&0xFF);
            int dx=(int8_t)(((tr>>24)&0xFF)-128);
            int dy=(int8_t)(((tr>>16)&0xFF)-128);
            QPixmap image1 = image_.copy(i1 * width, j1 * height, width, height);
            image=QPixmap::fromImage(blend(image.toImage(), image1.toImage(), dx, dy));
            image1 = imagex2_.copy(i1 * width*2, j1 * height*2, width*2, height*2);
            imagex2=QPixmap::fromImage(blend(imagex2.toImage(), image1.toImage(), dx*2, dy*2));
        }

        QIcon icon(image);
        icon.addPixmap(imagex2);

        iconMap_[cname]=icon;
        return iconMap_[cname];
    }


    return emptyIcon;
}

QIcon IconLibrary::icon(int i, int j) const
{
    int width = 16;
    int height = 16;

	QIcon icon(image_.copy(i * width, j * height, width, height));
    width *= 2; height *= 2;
    icon.addPixmap(imagex2_.copy(i * width, j * height, width, height));
	return icon;
}

QIcon IconLibrary::icon(int i0, int j0, int i1, int j1, int dx, int dy) const
{
    int width = 16;
    int height = 16;

	QPixmap image0 = image_.copy(i0 * width, j0 * height, width, height);
	QPixmap image1 = image_.copy(i1 * width, j1 * height, width, height);
    QIcon icon(QPixmap::fromImage(blend(image0.toImage(), image1.toImage(), dx, dy)));

    width *= 2; height *= 2;
    image0 = imagex2_.copy(i0 * width, j0 * height, width, height);
    image1 = imagex2_.copy(i1 * width, j1 * height, width, height);
    icon.addPixmap(QPixmap::fromImage(blend(image0.toImage(), image1.toImage(), dx*2, dy*2)));

	return icon;
}
