#include <vector>
#include <string>
#include <stdlib.h>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QLocale>
#include <QDesktopServices>
#include <QDesktopWidget>
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

void g_setProperty(const char* what, const char* arg){


    // feel free to change this list
    QStringList acceptedWhat;
    acceptedWhat << "cursor(type)";
    acceptedWhat << "windowPosition(x,y)";
    acceptedWhat << "windowSize(w,h)";
    acceptedWhat << "minimumSize(w,h)";
    acceptedWhat << "maximumSize(w,h)";
    acceptedWhat << "windowColor(r,g,b)";
    acceptedWhat << "windowTitle(text)";
    acceptedWhat << "windowModel(type)";

    QString argGet = QString::fromUtf8(arg);
    QStringList arrayArg = argGet.split("|",QString::KeepEmptyParts);
    int arg1 = arrayArg.at(0).toInt();
    int arg2 = arrayArg.at(1).toInt();
    int arg3 = arrayArg.at(2).toInt();
    QString argString = arrayArg.at(3);



    if (strcmp(what, "cursor") == 0)
    {
        QStringList acceptedValue;
        acceptedValue << "arrow" << "upArrow" << "cross" << "wait" << "IBeam";
        acceptedValue << "sizeVer" << "sizeHor" << "sizeBDiag" << "sizeFDiag" << "sizeAll";
        acceptedValue << "blank" << "splitV" << "splitH" << "pointingHand" << "forbidden";
        acceptedValue << "whatsThis" << "busy" << "openHand" << "closedHand" << "dragCopy";
        acceptedValue << "dragMove" << "dragLink";
        // value of cursor also taken from index of the text, do not change the list

        if (acceptedValue.contains(argString)){
            arg1 = acceptedValue.indexOf(argString);
            MainWindow::getInstance()->setCursor((Qt::CursorShape) arg1);
        }else{
            MainWindow::getInstance()->printToOutput("Accepted value for cursor :");
            for( int i=0; i<acceptedValue.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
            }
        }


    }else if (strcmp(what, "windowPosition") == 0)
    {
        MainWindow::getInstance()->move(arg1,arg2);

    }else if (strcmp(what, "windowSize") == 0)
    {
        MainWindow::getInstance()->resizeWindow(arg1,arg2);


    }else if (strcmp(what, "minimumSize") == 0)
    {
        MainWindow::getInstance()->setMinimumSize(QSize(arg1,arg2));

    }else if (strcmp(what, "maximumSize") == 0)
    {
        MainWindow::getInstance()->setMaximumSize(QSize(arg1,arg2));

    }else if (strcmp(what, "windowColor") == 0)
    {
        if (arg1 > 255) {arg1 = 255;}else if(arg1 < 0) {arg1 = 255;}
        if (arg2 > 255) {arg2 = 255;}else if(arg2 < 0) {arg2 = 255;}
        if (arg3 > 255) {arg3 = 255;}else if(arg3 < 0) {arg3 = 255;}
        QPalette palette;
        QColor backgroundColor = QColor(arg1, arg2, arg3);
        palette.setColor(QPalette::Window, backgroundColor);
        MainWindow::getInstance()->setPalette(palette);
    }else if (strcmp(what, "windowTitle") == 0)
    {
        MainWindow::getInstance()->setWindowTitle(argString);

    }else if (strcmp(what, "windowModel") == 0)
    {
        QStringList acceptedValue;
        acceptedValue << "reset" << "stayOnTop" << "stayOnBottom" << "frameless" << "noTitleBar";
        acceptedValue << "noButton" << "onlyMinimize" << "onlyMaximize" << "onlyClose" << "noMinimize";
        acceptedValue << "noMaximize" << "noClose" << "helpButton";
        // value of windowModel also taken from index of the text, do not change the list

        if (acceptedValue.contains(argString)){
            Qt::WindowFlags flags = MainWindow::getInstance()->windowFlags();

            switch(arg1){
            case 0:{
                flags = Qt::Window;
                break;
            }
            case 1:{
                flags |= Qt::WindowStaysOnTopHint;
                break;
            }

            case 2:{
                flags |= Qt::WindowStaysOnBottomHint;
                break;
            }

            case 3:{
                flags |= Qt::FramelessWindowHint;
                break;
            }

            case 4:{
                flags = Qt::Window;
                flags |= Qt::CustomizeWindowHint;
                break;
            }

            case 5:{
                flags = Qt::Window;
                flags |= Qt::WindowTitleHint;
                break;
            }
            case 6:{
                flags = Qt::Window;
                flags |= Qt::WindowMinimizeButtonHint;
                break;
            }
            case 7:{
                flags = Qt::Window;
                flags |= Qt::WindowMaximizeButtonHint;
                break;
            }
            case 8:{
                flags = Qt::Window;
                flags |= Qt::WindowCloseButtonHint;
                break;
            }
            case 9:{
                flags = Qt::Window;
                flags |= Qt::WindowMaximizeButtonHint;
                flags |= Qt::WindowCloseButtonHint;
                break;
            }
            case 10:{
                flags = Qt::Window;
                flags |= Qt::WindowMinimizeButtonHint;
                flags |= Qt::WindowCloseButtonHint;
                break;
            }
            case 11:{
                flags = Qt::Window;
                flags |= Qt::WindowMinimizeButtonHint;
                flags |= Qt::WindowMaximizeButtonHint;
                break;
            }
            case 12:{
                flags = Qt::Window;
                flags |= Qt::WindowContextHelpButtonHint;
                flags |= Qt::WindowCloseButtonHint;
                break;
            }
            }

            MainWindow::getInstance()->setWindowFlags(flags);
            if (MainWindow::getInstance()->fullScreen()){
                MainWindow::getInstance()->showFullScreen();
            }else{
                MainWindow::getInstance()->showNormal();
            }

        }else{

            MainWindow::getInstance()->printToOutput("Accepted value for windowModel :");
            for( int i=0; i<acceptedValue.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
            }
        }

    }else{
        MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:set()");
        for( int i=0; i<acceptedWhat.size(); ++i ){
            MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
        }
    }
}


const char* g_getProperty(const char* what){
    QString returnedProperty = "";


    // feel free to change this list
    QStringList acceptedWhat;
    acceptedWhat << "[x,y] windowPosition";
    acceptedWhat << "[w,h] windowSize";
    acceptedWhat << "[w,h] screenSize";

    if (strcmp(what, "windowPosition") == 0)
    {
        returnedProperty.append( QString::number(MainWindow::getInstance()->pos().x()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(MainWindow::getInstance()->pos().y()) );

    }else if (strcmp(what, "windowSize") == 0)
    {
        returnedProperty.append( QString::number(MainWindow::getInstance()->windowSize().width()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(MainWindow::getInstance()->windowSize().height()) );

    }else if (strcmp(what, "screenSize") == 0)
    {
        returnedProperty.append( QString::number(QApplication::desktop()->availableGeometry().width()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(QApplication::desktop()->availableGeometry().height()) );
    }else{
        MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:get()");
        for( int i=0; i<acceptedWhat.size(); ++i ){
            MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
        }
    }

    return returnedProperty.toStdString().c_str();
}