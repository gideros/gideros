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
#include <QPalette>
#include "libnetwork.h"
#include "applicationwrapper.h"
#include "glcanvas.h"
#include "platform.h"
#include "settingsdialog.h"
#include "constants.cpp"

MainWindow* MainWindow::instance;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    MainWindow::instance = this;
    setScale(100);
    ui.setupUi(this);

    /*#if defined(Q_OS_MAC)
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    #else
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    #endif

    move(0, 0);*/

    ui.glCanvas->setExportedApp(true);
    ui.glCanvas->projectDir_ = QDir("assets").absolutePath();
}

MainWindow::~MainWindow(){}

void MainWindow::resizeEvent(QResizeEvent*){
    updateResolution();
}

void MainWindow::closeEvent(QCloseEvent*){

}

void MainWindow::resizeWindow(int width, int height){
    if(ui.glCanvas->getHardwareOrientation() == eLandscapeLeft || ui.glCanvas->getHardwareOrientation() == eLandscapeRight){
        int temp = width;
        width = height;
        height = temp;
    }
    resize(width, height);
    updateResolution();
}

void MainWindow::fullScreenWindow(bool fullScreen){
    if(fullScreen)
        this->showFullScreen();
    else
        this->showNormal();
    updateResolution();
}

void MainWindow::updateResolution(){
    int width = ui.centralWidget->width();
    int height = ui.centralWidget->height();
    if(ui.glCanvas->getHardwareOrientation() == eLandscapeLeft || ui.glCanvas->getHardwareOrientation() == eLandscapeRight){
        width = ui.centralWidget->height();
        height = ui.centralWidget->width();
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

    ui.glCanvas->setResolution(width, height);
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
