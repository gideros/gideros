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
#include <QClipboard>
#include <QFileDialog>
#include <QCoreApplication>

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
    QCoreApplication::quit();
}


bool g_checkStringProperty(bool isSet, const char* what){
    if (isSet){
        if ( (strcmp(what, "cursor") == 0)
             || (strcmp(what, "windowTitle") == 0)
             || (strcmp(what, "windowModel") == 0)
             || (strcmp(what, "clipboard") == 0)
             || (strcmp(what, "mkDir") == 0)
             || (strcmp(what, "documentDirectory") == 0)
             || (strcmp(what, "temporaryDirectory") == 0)
           )
        {
            return true;
        }else{
            return false;
        }
    }else{
        if ( (strcmp(what, "openFileDialog") == 0)
             || (strcmp(what, "openDirectoryDialog") == 0)
             || (strcmp(what, "saveFileDialog") == 0)
             || (strcmp(what, "directory") == 0)
           )
        {
            return true;
        }else{
            return false;
        }

    }
}

void g_setProperty(const char* what, const char* arg){

    QString argGet = QString::fromUtf8(arg);
    int arg1 = 0;
    int arg2 = 0;
    int arg3 = 0;
    QString argString = "";

    if ( g_checkStringProperty(true,what)){
        argString = argGet;
        argString.replace("\\","\\\\");
    }else{
        QStringList arrayArg = argGet.split("|",QString::KeepEmptyParts);
        arg1 = arrayArg.at(0).toInt();
        arg2 = arrayArg.at(1).toInt();
        arg3 = arrayArg.at(2).toInt();
    }



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

        if (acceptedValue.contains(argString)){
            arg1 = acceptedValue.indexOf(argString);
            MainWindow::getInstance()->setCursor((Qt::CursorShape) arg1);
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
        MainWindow::getInstance()->move(arg1,arg2);

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "windowSize") == 0)
    {
        MainWindow::getInstance()->resizeWindow(arg1,arg2);


        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "minimumSize") == 0)
    {
        MainWindow::getInstance()->setMinimumSize(QSize(arg1,arg2));

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "maximumSize") == 0)
    {
        MainWindow::getInstance()->setMaximumSize(QSize(arg1,arg2));

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "windowColor") == 0)
    {
        if (arg1 > 255) {arg1 = 255;}else if(arg1 < 0) {arg1 = 255;}
        if (arg2 > 255) {arg2 = 255;}else if(arg2 < 0) {arg2 = 255;}
        if (arg3 > 255) {arg3 = 255;}else if(arg3 < 0) {arg3 = 255;}
        QPalette palette;
        QColor backgroundColor = QColor(arg1, arg2, arg3);
        palette.setColor(QPalette::Window, backgroundColor);
        MainWindow::getInstance()->setPalette(palette);

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "windowTitle") == 0)
    {
        MainWindow::getInstance()->setWindowTitle(argString);

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "windowModel") == 0)
    {
        QStringList acceptedValue;
        acceptedValue << "reset" << "stayOnTop" << "stayOnBottom" << "frameless" << "noTitleBar";
        acceptedValue << "noButton" << "onlyMinimize" << "onlyMaximize" << "onlyClose" << "noMinimize";
        acceptedValue << "noMaximize" << "noClose" << "helpButton";

        if (acceptedValue.contains(argString)){
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
    }else if (strcmp(what, "cursorPosition") == 0)
    {
        QCursor::setPos(arg1,arg2);


        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "clipboard") == 0)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(argString);

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "mkDir") == 0)
    {
        QStringList argSplit = argString.split("|",QString::KeepEmptyParts);
        if(argSplit.size() == 1){

            MainWindow::getInstance()->printToOutput("[[Usage Example]]");
            MainWindow::getInstance()->printToOutput("application:set(\"mkDir\",application:get(\"directory\",\"executable\")..\"|dirName\")");
        }else{
            QDir dirPath = QDir::temp();
            dirPath.setPath(argSplit.at(0));
            dirPath.mkdir(argSplit.at(1));
        }

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "documentDirectory") == 0)
    {
        setDocumentsDirectory(argString.toStdString().c_str());
        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "temporaryDirectory") == 0)
    {
        setTemporaryDirectory(argString.toStdString().c_str());

    }else{
    	
	    // feel free to change this list
	    QStringList acceptedWhat;
	    acceptedWhat << "windowPosition(x,y)";
	    acceptedWhat << "windowSize(w,h)";
	    acceptedWhat << "minimumSize(w,h)";
	    acceptedWhat << "maximumSize(w,h)";
	    acceptedWhat << "windowColor(r,g,b)";
	    acceptedWhat << "windowTitle(text)";
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

const char* g_getProperty(const char* what, const char* arg){
    QString returnedProperty = "";

    QString argGet = QString::fromUtf8(arg);
    int arg1 = 0;
    int arg2 = 0;
    int arg3 = 0;
    QString argString = "";

    if ( g_checkStringProperty(false,what)){
        argString = argGet;
        argString.replace("\\","\\\\");
    }else{
        QStringList arrayArg = argGet.split("|",QString::KeepEmptyParts);
        arg1 = arrayArg.at(0).toInt();
        arg2 = arrayArg.at(1).toInt();
        arg3 = arrayArg.at(2).toInt();
    }



    /*------------------------------------------------------------------*/
    if (strcmp(what, "windowPosition") == 0)
    {
        returnedProperty.append( QString::number(MainWindow::getInstance()->pos().x()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(MainWindow::getInstance()->pos().y()) );
        /*------------------------------------------------------------------*/

    }else if (strcmp(what, "windowSize") == 0)
    {
        returnedProperty.append( QString::number(MainWindow::getInstance()->windowSize().width()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(MainWindow::getInstance()->windowSize().height()) );

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "screenSize") == 0)
    {
        returnedProperty.append( QString::number(QApplication::desktop()->availableGeometry().width()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(QApplication::desktop()->availableGeometry().height()) );
        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "cursorPosition") == 0)
    {
        returnedProperty.append( QString::number(QCursor::pos().x()) );
        returnedProperty.append("|");
        returnedProperty.append( QString::number(QCursor::pos().y()) );

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "clipboard") == 0)
    {
        QClipboard *clipboard = QApplication::clipboard();
        returnedProperty.append("s");
        returnedProperty.append( clipboard->text() );

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "windowTitle") == 0)
    {
        returnedProperty.append("s");
        returnedProperty.append( MainWindow::getInstance()->windowTitle() );

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "directory") == 0)
    {

        QStringList acceptedValue;
        acceptedValue << "executable" << "document"  << "desktop" << "temporary" << "data" ;
        acceptedValue << "music" << "movies"  << "pictures" << "cache" << "download" ;
        acceptedValue << "home";

        if (acceptedValue.contains(argString)){
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
            returnedProperty.append("s");
            returnedProperty.append( pathGet );
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
        QStringList dialogArg = argString.split("|",QString::KeepEmptyParts);
        if(dialogArg.size() == 1){

            MainWindow::getInstance()->printToOutput("[[Usage Example]]");
            if (strcmp(what, "openDirectoryDialog") == 0){
                MainWindow::getInstance()->printToOutput("application:get(\"openDirectoryDialog\",\"Open Directory|C:/)\")");
            }else if (strcmp(what, "openFileDialog") == 0){
                MainWindow::getInstance()->printToOutput("application:get(\"openFileDialog\",\"Open File|C:/|Text File (*.txt);;Image File (*.jpg *.png)\")");
            }else if (strcmp(what, "saveFileDialog") == 0){
                MainWindow::getInstance()->printToOutput("application:get(\"saveFileDialog\",\"Save File|C:/|Text File (*.txt);;Image File (*.jpg *.png)\")");
            }
        }else{

            QString title = dialogArg.at(0);
            QString place = "";
            QString extension = "";
            if (dialogArg.size() >= 2){place = dialogArg.at(1);}
            if (dialogArg.size() >= 3){extension = dialogArg.at(2);}

            QString fileName = "";
            if (strcmp(what, "openDirectoryDialog") == 0){
                fileName = QFileDialog::getExistingDirectory(MainWindow::getInstance(),title ,place, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            }else if (strcmp(what, "openFileDialog") == 0){
                fileName = QFileDialog::getOpenFileName(MainWindow::getInstance(), title,place,extension);

            }else if (strcmp(what, "saveFileDialog") == 0){
                fileName = QFileDialog::getSaveFileName(MainWindow::getInstance(), title,place,extension);

            }
            returnedProperty.append("s");
            returnedProperty.append( fileName );
        }

        /*------------------------------------------------------------------*/

    }else if (strcmp(what, "temporaryDirectory") == 0)
    {
        returnedProperty.append("s");
        returnedProperty.append(  QString::fromUtf8( getTemporaryDirectory() )  );

        /*------------------------------------------------------------------*/
    }else if (strcmp(what, "documentDirectory") == 0)
    {
        returnedProperty.append("s");
        returnedProperty.append(  QString::fromUtf8( getDocumentsDirectory() )  );

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


    return returnedProperty.toStdString().c_str();
}
