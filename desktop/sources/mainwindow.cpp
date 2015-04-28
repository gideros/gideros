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

    ui.setupUi(this);

    #if defined(Q_OS_MAC)
//        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowTitleHint);
    #else
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    #endif

    move(0, 0);

    fullscreen_ = false;
    width_ = 320;
    height_ = 480;

    ui.glCanvas->setExportedApp(true);
    ui.glCanvas->projectDir_ = QDir("assets").absolutePath();
}

MainWindow::~MainWindow(){}

void MainWindow::resizeWindow(int width, int height){
    resize(width, height);
}

void MainWindow::fullScreenWindow(bool fullScreen){
    fullscreen_ = fullScreen;

    int width, height;

    if(fullscreen_){
        ui.centralWidget->setMinimumSize(1, 1);

        showFullScreen();

        width = ui.centralWidget->width();
        height = ui.centralWidget->height();

    }else{
        ui.centralWidget->setMinimumSize(0, 0);

        showNormal();

        width = width_;
        height = height_;
    }

    ::setResolution(width, height);
}

void MainWindow::setResolution(int width, int height){
    if(!fullscreen_){
        width_ = width;
        height_ = height;
    }

    ui.glCanvas->setFixedSize(width, height);
    ui.glCanvas->setResolution(width, height);

    if(!fullscreen_){
        ::setWindowSize(width, height);
    }
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

void MainWindow::resizeEvent(QResizeEvent*){
    if(fullscreen_){
        ::setFullScreen(true);
    }
}
