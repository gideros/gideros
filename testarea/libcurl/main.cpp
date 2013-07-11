#include <QtGui/QApplication>
#include "mainwindow.h"
#include "requester.h"
#include <curl/curl.h>
#include <QDebug>

int main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_WIN32);
	req_init();

	QApplication a(argc, argv);
    MainWindow w;
    w.show();

	qDebug() << "1";

	int result = a.exec();

	qDebug() << "2";

	req_clean();

	qDebug() << "3";

	curl_global_cleanup();

	qDebug() << "4";

	return result;
}
