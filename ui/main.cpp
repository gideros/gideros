#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <licensemanager.h>
#include "countly.h"

Countly *g_countly = NULL;

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("GiderosMobile");
	QCoreApplication::setOrganizationDomain("giderosmobile.com");
	QCoreApplication::setApplicationName("GiderosStudio");

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication a(argc, argv);

#if 0
	LicenseManager licenseManager;
	licenseManager.checkExpired();
	licenseManager.updateLicense();
#endif

	QDir dir = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MAC)
	dir.cdUp();
#endif
	QDir::setCurrent(dir.absolutePath());

    LicenseManager licenseManager;
    g_countly = new Countly("2013.09.1", licenseManager.getLicenseType());
    g_countly->beginSession();

	MainWindow w;
	w.show();
    int result = a.exec();

    g_countly->endSession();
    g_countly->waitForFinished(3000);
    delete g_countly;

    return result;
}
