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

    ui.glCanvas->setDrawInfos(false);
    ui.glCanvas->exportedApp_ = true;

    move(QPoint(0, 0));

    QDir directory = QDir("Assets");
    ui.glCanvas->projectDir_ = directory.absolutePath();
}

MainWindow::~MainWindow(){}

void MainWindow::resizeWindow(int width, int height){
    resize(width, height);
}

void MainWindow::fullScreenWindow(bool fullScreen){
    if(fullScreen)
        this->showFullScreen();
    else
        this->showNormal();
}

void MainWindow::projectNameChanged(const QString& projectName){
    show();
    raise();
    activateWindow();

    if(projectName.isEmpty() == true)
        setWindowTitle(Constants::WINDESK_WINDOW_TITLE);
    else
        setWindowTitle(projectName);
}
