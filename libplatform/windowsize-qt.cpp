#include "mainwindow.h"

void setWindowSize(LuaApplication* application, int width, int height){
    MainWindow* mainWindow = (MainWindow*)application->glcanvas()->parentWidget()->parentWidget();
    mainWindow->resizeWindow(width, height);
}
