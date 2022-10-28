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
#include <QClipboard>
#include <QFileDialog>
#include <QCoreApplication>
#include <QClipboard>
#include <QMimeData>
#include <ginput.h>
#include <QStandardPaths>
#include <QScreen>
#include "platform.h"

#if defined(Q_OS_WIN)
    #include <windows.h>
#elif defined(Q_OS_MAC)
    #import <IOKit/pwr_mgt/IOPMLib.h>

    CFStringRef reasonForActivity= CFSTR("keep awake");

    IOPMAssertionID assertionID;
    IOReturn success = kIOReturnError;
#endif

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
    if (awake){
        #if defined(Q_OS_WIN)
            SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
            
        #elif defined(Q_OS_MAC)
            if(success == kIOReturnSuccess) {
            	
            }else{
                success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, 
                                                    kIOPMAssertionLevelOn, reasonForActivity, &assertionID);
            }
        #endif
    }else{
        #if defined(Q_OS_WIN)
            SetThreadExecutionState(ES_CONTINUOUS);
            
        #elif defined(Q_OS_MAC)
            if(success == kIOReturnSuccess) {
                IOPMAssertionRelease(assertionID);
                success = kIOReturnError;
                
            }else{
            	
            }
        #endif
    }
}

bool setKeyboardVisibility(bool visible){
	return false;
}

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	if (mimeType.size()==0)
		QApplication::clipboard()->clear();
	else {
		QMimeData *mime=new QMimeData;;
		mime->setData(QString(mimeType.c_str()),QByteArray(data.c_str(),data.size()));
		QApplication::clipboard()->setMimeData(mime);
	}
	return 1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData==NULL) return -1;
    if (mimeType.size()>0)
    {
    	if (!mimeData->hasFormat(QString(mimeType.c_str())))
    		return false;
    	QByteArray d=mimeData->data(QString(mimeType.c_str()));
    	data=std::string(d.data(),d.size());
    	return 1;
    }
    mimeType=mimeData->formats().join(" ").toStdString();

	return 1;
}

int getKeyboardModifiers() {
   Qt::KeyboardModifiers qmod=QGuiApplication::queryKeyboardModifiers();
   int m=0;
   if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
   if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
   if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
   if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;

   return m;
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

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
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
    QCoreApplication::quit();
}

#ifdef Q_OS_WIN
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>
#endif


std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
    std::vector<gapplication_Variant> rets;
    gapplication_Variant r;
    if (!set) {
        /*------------------------------------------------------------------*/
        if (strcmp(what, "windowPosition") == 0)
        {
            r.type=gapplication_Variant::DOUBLE;
            r.d=MainWindow::getInstance()->pos().x();
            rets.push_back(r);
            r.d=MainWindow::getInstance()->pos().y();
            rets.push_back(r);
            /*------------------------------------------------------------------*/

        }else if (strcmp(what, "windowSize") == 0)
        {
            r.type=gapplication_Variant::DOUBLE;
            r.d=MainWindow::getInstance()->windowSize().width();
            rets.push_back(r);
            r.d=MainWindow::getInstance()->windowSize().height();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "screenSize") == 0)
        {
            r.type=gapplication_Variant::DOUBLE;
            r.d=QApplication::primaryScreen()->geometry().width();
            rets.push_back(r);
            r.d=QApplication::primaryScreen()->geometry().height();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "cursorPosition") == 0)
        {
            r.type=gapplication_Variant::DOUBLE;
            r.d=QCursor::pos().x();
            rets.push_back(r);
            r.d=QCursor::pos().y();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
            QClipboard *clipboard = QApplication::clipboard();
            r.type=gapplication_Variant::STRING;
            r.s=clipboard->text().toStdString();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=MainWindow::getInstance()->windowTitle().toStdString();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "directory") == 0)
        {

            QStringList acceptedValue;
            acceptedValue << "executable" << "document"  << "desktop" << "temporary" << "data" ;
            acceptedValue << "music" << "movies"  << "pictures" << "cache" << "download" ;
            acceptedValue << "home";

            QString argString=QString::fromUtf8(args[0].s.c_str());
            if ((args.size()>0)&&acceptedValue.contains(argString)){

                QString pathGet = "";
                if (argString == "executable"){
                    pathGet = QDir::currentPath();
                }else if (argString == "document"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
                }else if (argString == "desktop"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                }else if (argString == "temporary"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                }else if (argString == "data"){
    #ifdef RASPBERRY_PI
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    #else
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    #endif
                }else if (argString == "music"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
                }else if (argString == "movies"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
                }else if (argString == "pictures"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
                }else if (argString == "cache"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
                }else if (argString == "download"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
                }else if (argString == "home"){
                    pathGet = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

                }else{

                }
                r.type=gapplication_Variant::STRING;
                r.s=pathGet.toStdString();
                rets.push_back(r);
            }else{
                QString info = "Accepted value for ";
                info.append(what);
                info.append(" :");
                MainWindow::getInstance()->printToOutput(info.toStdString().c_str());
                for( int i=0; i<acceptedValue.size(); ++i ){
                    MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
                }
            }

            /*------------------------------------------------------------------*/
        }else if ((strcmp(what, "openDirectoryDialog") == 0)
                || (strcmp(what, "openFileDialog") == 0)
                || (strcmp(what, "saveFileDialog") == 0))
            {
            if(args.size() == 1){

                MainWindow::getInstance()->printToOutput("[[Usage Example]]");
                if (strcmp(what, "openDirectoryDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"openDirectoryDialog\",\"Open Directory\",\"C:/)\")");
                }else if (strcmp(what, "openFileDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"openFileDialog\",\"Open File\",\"C:/\",\"Text File (*.txt);;Image File (*.jpg *.png)\")");
                }else if (strcmp(what, "saveFileDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"saveFileDialog\",\"Save File\",\"C:/\",\"Text File (*.txt);;Image File (*.jpg *.png)\")");
                }
            }else{

                QString title = QString::fromUtf8(args[0].s.c_str());
                QString place = "";
                QString extension = "";
                if (args.size() >= 2){place = QString::fromUtf8(args[1].s.c_str());}
                if (args.size() >= 3){extension = QString::fromUtf8(args[2].s.c_str());}

                QString fileName = "";
                if (strcmp(what, "openDirectoryDialog") == 0){
                    fileName = QFileDialog::getExistingDirectory(MainWindow::getInstance(),title ,place, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                }else if (strcmp(what, "openFileDialog") == 0){
                    fileName = QFileDialog::getOpenFileName(MainWindow::getInstance(), title,place,extension);

                }else if (strcmp(what, "saveFileDialog") == 0){
                    fileName = QFileDialog::getSaveFileName(MainWindow::getInstance(), title,place,extension);

                }
                r.type=gapplication_Variant::STRING;
                r.s=fileName.toStdString();
                rets.push_back(r);
            }
            /*------------------------------------------------------------------*/

        }else if (strcmp(what, "temporaryDirectory") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=getTemporaryDirectory();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "documentDirectory") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=getDocumentsDirectory();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else{
            // feel free to change this list
            QStringList acceptedWhat;
            acceptedWhat << "[x,y] windowPosition";
            acceptedWhat << "[w,h] windowSize";
            acceptedWhat << "[w,h] screenSize";
            acceptedWhat << "[x,y] cursorPosition";
            acceptedWhat << "[text] clipboard";
            acceptedWhat << "[text] windowTitle";
            acceptedWhat << "[path] directory(where//help)";
            acceptedWhat << "[path] openDirectoryDialog(title|path//help)";
            acceptedWhat << "[path] openFileDialog(title|path|extensions//help)";
            acceptedWhat << "[path] saveFileDialog(title|path|extensions//help)";
            acceptedWhat << "[path] documentDirectory";
            acceptedWhat << "[path] temporaryDirectory";

            MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:get()");
            for( int i=0; i<acceptedWhat.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
            }
        }
    }
    else {
        /*------------------------------------------------------------------*/
        if (strcmp(what, "cursor") == 0)
        {
            QStringList acceptedValue;
            acceptedValue << "arrow" << "upArrow" << "cross" << "wait" << "IBeam";
            acceptedValue << "sizeVer" << "sizeHor" << "sizeBDiag" << "sizeFDiag" << "sizeAll";
            acceptedValue << "blank" << "splitV" << "splitH" << "pointingHand" << "forbidden";
            acceptedValue << "whatsThis" << "busy" << "openHand" << "closedHand" << "dragCopy";
            acceptedValue << "dragMove" << "dragLink";
            // value of cursor also taken from index of the text, do not change the list

            QString argString=QString::fromUtf8(args[0].s.c_str());
            if ((args.size()>0)&&acceptedValue.contains(argString)){
                MainWindow::getInstance()->setCursor((Qt::CursorShape) acceptedValue.indexOf(argString));
            }else{
                QString info = "Accepted value for ";
                info.append(what);
                info.append(" :");
                MainWindow::getInstance()->printToOutput(info.toStdString().c_str());
                for( int i=0; i<acceptedValue.size(); ++i ){
                    MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
                }
            }


            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowPosition") == 0)
        {
            if (args.size()>=2)
                MainWindow::getInstance()->move(args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowSize") == 0)
        {
            if (args.size()>=2)
                MainWindow::getInstance()->resizeWindow(args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "minimumSize") == 0)
        {
            if (args.size()>=2)
                MainWindow::getInstance()->setMinimumSize(QSize(args[0].d,args[1].d));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "maximumSize") == 0)
        {
            if (args.size()>=2)
                MainWindow::getInstance()->setMaximumSize(QSize(args[0].d,args[1].d));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowColor") == 0)
        {
            if (args.size()>=3) {
                if (args[0].d > 255) {args[0].d = 255;}else if(args[0].d < 0) {args[0].d = 255;}
                if (args[1].d > 255) {args[1].d = 255;}else if(args[1].d < 0) {args[1].d = 255;}
                if (args[2].d > 255) {args[2].d = 255;}else if(args[2].d < 0) {args[2].d = 255;}
                QPalette palette;
                QColor backgroundColor = QColor(args[0].d, args[1].d, args[2].d);
                palette.setColor(QPalette::Window, backgroundColor);
                MainWindow::getInstance()->setPalette(palette);
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
            if (args.size()>=1)
                MainWindow::getInstance()->setWindowTitle(QString::fromUtf8(args[0].s));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowModel") == 0)
        {
            QStringList acceptedValue;
            acceptedValue << "reset" << "stayOnTop" << "stayOnBottom" << "frameless" << "noTitleBar";
            acceptedValue << "noButton" << "onlyMinimize" << "onlyMaximize" << "onlyClose" << "noMinimize";
            acceptedValue << "noMaximize" << "noClose" << "helpButton";

            QString argString=QString::fromUtf8(args[0].s.c_str());
            if ((args.size()>0)&&acceptedValue.contains(argString)) {
                Qt::WindowFlags flags = MainWindow::getInstance()->windowFlags();

                if (argString == "reset"){
                    flags = Qt::Window;
                }else if (argString == "stayOnTop"){
                    flags |= Qt::WindowStaysOnTopHint;
                }else if (argString == "stayOnBottom"){
                    flags |= Qt::WindowStaysOnBottomHint;
                }else if (argString == "frameless"){
                    flags |= Qt::FramelessWindowHint;
                }else if (argString == "noTitleBar"){
                    flags = Qt::Window;
                    flags |= Qt::CustomizeWindowHint;
                }else if (argString == "noButton"){
                    flags = Qt::Window;
                    flags |= Qt::WindowTitleHint;
                }else if (argString == "noClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                }else if (argString == "onlyMaximize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMaximizeButtonHint;
                }else if (argString == "onlyClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noMinimize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMaximizeButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noMaximize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                    flags |= Qt::WindowMaximizeButtonHint;
                }else if (argString == "helpButton"){
                    flags = Qt::Window;
                    flags |= Qt::WindowContextHelpButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }

                MainWindow::getInstance()->setWindowFlags(flags);
                if (MainWindow::getInstance()->fullScreen()){
                    MainWindow::getInstance()->showFullScreen();
                }else{
                    MainWindow::getInstance()->showNormal();
                }

            }else{

                QString info = "Accepted value for ";
                info.append(what);
                info.append(" :");
                MainWindow::getInstance()->printToOutput(info.toStdString().c_str());
                for( int i=0; i<acceptedValue.size(); ++i ){
                    MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
                }
            }

            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "wintabMode") == 0)
        {
    #ifdef Q_OS_WIN
            auto nativeWindowsApp = dynamic_cast<QNativeInterface::Private::QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration());
            if (args.size()>=1)
                nativeWindowsApp->setWinTabEnabled(args[0].d);
    #endif
        }else if (strcmp(what, "cursorPosition") == 0)
        {
            if (args.size()>=2)
                QCursor::setPos(args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
            QClipboard *clipboard = QApplication::clipboard();
            if (args.size()>=1)
                clipboard->setText(QString::fromUtf8(args[0].s));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "mkDir") == 0)
        {
            if(args.size() <= 1){
                MainWindow::getInstance()->printToOutput("[[Usage Example]]");
                MainWindow::getInstance()->printToOutput("application:set(\"mkDir\",application:get(\"directory\",\"executable\")..\"|dirName\")");
            }else{
                QDir dirPath = QDir::temp();
                dirPath.setPath(QString::fromUtf8(args[0].s));
                dirPath.mkdir(QString::fromUtf8(args[1].s));
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "documentDirectory") == 0)
        {
            if(args.size() >= 1)
                setDocumentsDirectory(args[0].s.c_str());
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "temporaryDirectory") == 0)
        {
            if(args.size() >= 1)
                setTemporaryDirectory(args[0].s.c_str());
        }else{

            // feel free to change this list
            QStringList acceptedWhat;
            acceptedWhat << "windowPosition(x,y)";
            acceptedWhat << "windowSize(w,h)";
            acceptedWhat << "minimumSize(w,h)";
            acceptedWhat << "maximumSize(w,h)";
            acceptedWhat << "windowColor(r,g,b)";
            acceptedWhat << "windowTitle(text)";
            acceptedWhat << "wintabMode(enable)";
            acceptedWhat << "windowModel(type//help)";
            acceptedWhat << "cursor(type//help)";
            acceptedWhat << "cursorPosition(x,y)";
            acceptedWhat << "clipboard(text)";
            acceptedWhat << "mkdir(path|dirName//help)";
            acceptedWhat << "documentDirectory(path)";
            acceptedWhat << "temporaryDirectory(path)";

            MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:set()");
            for( int i=0; i<acceptedWhat.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
            }
        }

    }
    return rets;
}
