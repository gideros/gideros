#include "mainwindow.h"

void setWindowSize(int width, int height){
    MainWindow::getInstance()->resizeWindow(width, height);
}

void setFullScreen(bool fullScreen){
    MainWindow::getInstance()->fullScreenWindow(fullScreen);
}
