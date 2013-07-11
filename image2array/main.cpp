#include <QtCore/QCoreApplication>
#include <QImage>
#include <QDebug>
#include <zlib.h>

uLong compressBound(uLong sourceLen)
{
	return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QImage image("C:/Users/Atilim Cetin/Desktop/gideros_mobile_logo_small.png");

	int width = image.width();
	int height = image.height();

	std::vector<unsigned char> data(width * height * 4);

	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			int index = (x + y * width) * 4;

			QRgb rgba = image.pixel(x, y);

			data[index + 0] = qRed(rgba);
			data[index + 1] = qGreen(rgba);
			data[index + 2] = qBlue(rgba);
			data[index + 3] = qAlpha(rgba);
		}

	uLong destLen = compressBound(data.size());
	Bytef* dest = (Bytef*)malloc(destLen);
	int result = compress(dest, &destLen, &data[0], data.size());

	FILE* fos = fopen("logo.inc", "wt");
	fprintf(fos, "static unsigned char logo[] = {\n");
	int count = 0;
	for (int i = 0; i < destLen; ++i)
	{
		fprintf(fos, "%d, ", dest[i]);
		count++;
		if (count == 16)
		{
			fprintf(fos, "\n");
			count = 0;
		}
	}
	fprintf(fos, "};\n");
	fclose(fos);

	free(dest);

	printf("finished.\n");

	return a.exec();
}
