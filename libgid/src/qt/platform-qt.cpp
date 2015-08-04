#include <vector>
#include <string>
#include <stdlib.h>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QLocale>
#include <QDesktopServices>
#include <QUrl>
#include <QHostInfo>
#include "mainwindow.h"

std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

#ifdef Q_OS_WIN
	result.push_back("Windows");
#endif
#ifdef Q_OS_MAC
	result.push_back("Mac OS");
#endif

	return result;
}

void setKeepAwake(bool awake)
{

}

void vibrate(int ms)
{
}

void setWindowSize(int width, int height){
    MainWindow::getInstance()->resizeWindow(width, height);
}

void setFullScreen(bool fullScreen){
    MainWindow::getInstance()->fullScreenWindow(fullScreen);
}

std::string getDeviceName(){
    return QHostInfo::localHostName().toStdString();
}

std::string getLocale()
{
	return QLocale().name().toStdString();
}

std::string getLanguage()
{
	QString locale = QLocale().name();

	QStringList list = locale.split("_");

	if (!list.empty())
		return list[0].toStdString();

	return "en";
}

void openUrl(const char* url)
{
    QDesktopServices::openUrl(QUrl::fromEncoded(url));
}

bool canOpenUrl(const char *url)
{
    return true;
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
    s_fps = fps;
}

}


void g_exit()
{
    MainWindow::getInstance()->saveSettings();
    QCoreApplication::quit();
}
