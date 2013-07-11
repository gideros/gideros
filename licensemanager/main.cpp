#include <QApplication>
#include "mainwindow.h"
#include <QSettings>
#include "licensemanager.h"
#include <QNetworkProxyFactory>

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("GiderosMobile");
	QCoreApplication::setOrganizationDomain("giderosmobile.com");
	QCoreApplication::setApplicationName("GiderosLicenseManager");

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication a(argc, argv);

    //g_querySystemProxy();
    QNetworkProxyFactory::setUseSystemConfiguration(true);

	MainWindow w;
    w.show();

    return a.exec();
}
