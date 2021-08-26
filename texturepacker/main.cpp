#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
//#include <zip.h>
#include <time.h>
#include <QDir>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
/*	int err;
	zipFile zf = zipOpen("my.zip", 0);

	zip_fileinfo zi;
	zi.tmz_date.tm_sec = 0;
	zi.tmz_date.tm_min = 0;
	zi.tmz_date.tm_hour = 0;
	zi.tmz_date.tm_mday = 1;
	zi.tmz_date.tm_mon = 1;
	zi.tmz_date.tm_year = 2011;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	err = zipOpenNewFileInZip(zf, "1.txt", &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

	char* buffer = "atilim cetin";
	zipWriteInFileInZip(zf, buffer, strlen(buffer));

	err = zipCloseFileInZip(zf);
	err = zipClose(zf, NULL);

	return 0; */


	QCoreApplication::setOrganizationName("GiderosMobile");
	QCoreApplication::setOrganizationDomain("giderosmobile.com");
	QCoreApplication::setApplicationName("GiderosTexturePacker");
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication a(argc, argv);

        QDir dir = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MAC)
        dir.cdUp();
#endif
        QDir::setCurrent(dir.absolutePath());

	MainWindow w;
	w.show();

    if (argc>1)
        w.openStartProject(argv[1]);

	return a.exec();
}
