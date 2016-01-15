#include "mainwindow.h"
#include <QActionGroup>
#include <QSettings>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <algorithm>
#include <QDesktopWidget>
#include <QPalette>
#include <QWindowStateChangeEvent>
#include "libnetwork.h"
#include "applicationwrapper.h"
#include "glcanvas.h"
#include "platform.h"
#include "settingsdialog.h"
#include "constants.cpp"

#include <bytebuffer.h>

MainWindow* MainWindow::instance;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    fixedSize_ = true;
    MainWindow::instance = this;
    setScale(100);
    ui.setupUi(this);

    /*#if defined(Q_OS_MAC)
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    #else
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    #endif
    */
    move(0, 0);

    ui.glCanvas->setExportedApp(true);
    ui.glCanvas->projectDir_ = QDir("assets").absolutePath();

    
    width0_ = 320;
    height0_ = 480;
    scaleModeNum_ = 0;
    fullScreen_ = false;

    QDir dir = QCoreApplication::applicationDirPath();

    #if defined(Q_OS_MAC)
        dir.cdUp();
    #endif

    QFile file(dir.absolutePath()+"/assets/properties.bin");
    if(file.exists()){
        file.open(QIODevice::ReadOnly);
        QByteArray ba = file.readAll();
        file.close();
        std::vector<char> data(ba.data(), ba.data() + ba.size());
        ByteBuffer buffer(&data[0], data.size());

        int scaleMode, logicalWidth, logicalHeight, windowWidth, windowHeight;
        buffer >> scaleMode;

        setLogicalScaleMode((LogicalScaleMode) scaleMode);
        
        buffer >> logicalWidth;
        buffer >> logicalHeight;

        width0_ = logicalWidth;
        height0_ = logicalHeight;

    	int scaleCount;
    	buffer >> scaleCount;
    	std::vector<std::pair<std::string, float> > imageScales(scaleCount);
    	for (int i = 0; i < scaleCount; ++i) {
    		buffer >> imageScales[i].first;
    		buffer >> imageScales[i].second;
    	}
    	
        int orientation;
        buffer >> orientation;
        ui.glCanvas->setHardwareOrientation((Orientation) orientation);

        int fps;
        buffer >> fps;

        int retinaDisplay;
        buffer >> retinaDisplay;

        int autorotation;
        buffer >> autorotation;

        int mouseToTouch;
        buffer >> mouseToTouch;
        int touchToMouse;
        buffer >> touchToMouse;
        int mouseTouchOrder;
        buffer >> mouseTouchOrder;



        buffer >> windowWidth;
        buffer >> windowHeight;
        if (windowWidth == 0 && windowHeight == 0) {
            windowWidth = logicalWidth;
            windowHeight = logicalHeight;
            setFixedSize(false);
        }else{
            width0_ = windowWidth;
            height0_ = windowHeight;
        }
        setWindowSize(windowWidth, windowHeight);

        
    }

    resolution_ = (float)width0_ / height0_;
    
}

MainWindow::~MainWindow(){}

void MainWindow::resizeEvent(QResizeEvent*){
    updateResolution(true);
}

void MainWindow::closeEvent(QCloseEvent*){

}

void MainWindow::setFixedSize(bool fixedSize){
    fixedSize_ = fixedSize;
}

void MainWindow::resizeWindow(int width, int height){
    if(ui.glCanvas->getHardwareOrientation() == eLandscapeLeft || ui.glCanvas->getHardwareOrientation() == eLandscapeRight){
        int temp = width;
        width = height;
        height = temp;
    }
    width_ = width;
    height_ = height;
    if(fixedSize_){
        setMaximumSize(width, height);
        setMinimumSize(width, height);
    }
    resize(width, height);
    
    updateResolution(false);
}

void MainWindow::fullScreenWindow(bool fullScreen){
    fullScreen_ = fullScreen;
    if(fullScreen){
        setMaximumSize(16777215, 16777215);
        this->showFullScreen();
    }
    else{
        this->showNormal();
        if(fixedSize_){
            setMaximumSize(width_, height_);
        }else{
            setMaximumSize(16777215, 16777215);
        }
    }
    updateResolution(true);
}

void MainWindow::updateResolution(bool event){
    int width = ui.centralWidget->width();
    int height = ui.centralWidget->height();
    if(ui.glCanvas->getHardwareOrientation() == eLandscapeLeft || ui.glCanvas->getHardwareOrientation() == eLandscapeRight){
        width = ui.centralWidget->height();
        height = ui.centralWidget->width();
    }

    const float resolution = (float)width / height;

    if (scaleModeNum_ == 1 ){
        if (resolution > resolution_){
           width = height * resolution_;
        }else{
           height = width / resolution_;
        }
    }else if (scaleModeNum_ == 2 ){
        width = height * resolution_;
    }else if (scaleModeNum_ == 3 ){
        height = width / resolution_;
    }
    
    float canvasScaleFactor = 1;
    float widgetScaleFactor = 1;
    if (deviceScale() != 0) {
        const float hundredPercentScale = 100;
        canvasScaleFactor = hundredPercentScale / deviceScale();
        widgetScaleFactor = hundredPercentScale / scale();
    }

    ui.glCanvas->setScale(canvasScaleFactor);

    switch (ui.glCanvas->getHardwareOrientation()){
        case ePortrait:
        case ePortraitUpsideDown:
            ui.glCanvas->setFixedSize(width / widgetScaleFactor, height / widgetScaleFactor);
            break;

        case eLandscapeLeft:
        case eLandscapeRight:
            ui.glCanvas->setFixedSize(height / widgetScaleFactor, width / widgetScaleFactor);
            break;
    }

    ui.glCanvas->setResolution(width, height, event);
}

void MainWindow::projectNameChanged(const QString& projectName){
    show();
    raise();
    activateWindow();

    if(projectName.isEmpty())
        setWindowTitle(Constants::DESK_WINDOW_TITLE);
    else
        setWindowTitle(projectName);
}

float MainWindow::deviceScale()
{
    return (float)((float)100 * (float)devicePixelRatio());
}

float MainWindow::scale(){
    return scale_;
}

void MainWindow::setScale(float scale){
    scale_ = scale;
}

void MainWindow::changeEvent(QEvent* e)
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QWindowStateChangeEvent* event = static_cast< QWindowStateChangeEvent* >( e );

        if( event->oldState() & Qt::WindowMinimized )
        {

        }
        else if( event->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized )
        {
            fullScreenWindow(true);
        }
    }
}



void MainWindow::setLogicalScaleMode(LogicalScaleMode scaleMode){
    // scaleModeNum_, 0 = no aspect ratio, 1 = aspect ratio, 2 = fit width, 3 = fit height
    if (scaleMode == eNoScale)
    {
        scaleModeNum_ = 0;
    }
    else if (scaleMode == eCenter)
    {
        scaleModeNum_ = 0;
    }
    else if (scaleMode == ePixelPerfect)
    {
        scaleModeNum_ = 0;
    }
    else if (scaleMode == eLetterBox)
    {
        scaleModeNum_ = 1;
    }
    else if (scaleMode == eCrop)
    {
        scaleModeNum_ = 1;
    }
    else if (scaleMode == eStretch)
    {
        scaleModeNum_ = 0;
    }
    else if (scaleMode == eFitWidth)
    {
        scaleModeNum_ = 2;
    }
    else if (scaleMode == eFitHeight)
    {
        scaleModeNum_ = 3;
    }
}

QSize MainWindow::windowSize(){
    int width,height;
    width = size().width();
    height = size().height();

    return QSize(width,height);
}

bool MainWindow::fullScreen(){
    return fullScreen_;
}

void MainWindow::printToOutput(const char* text){

}
