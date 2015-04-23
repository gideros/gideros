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
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    move(0, 0);

    ui.glCanvas->setExportedApp(true);
    ui.glCanvas->projectDir_ = QDir("assets").absolutePath();
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

    if(projectName.isEmpty())
        setWindowTitle(Constants::DESK_WINDOW_TITLE);
    else
        setWindowTitle(projectName);
}
