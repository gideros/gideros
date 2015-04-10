#include "mainwindow.h"

void setFullScreen(LuaApplication* application, bool fullScreen){
    MainWindow* mainWindow = (MainWindow*)application->glcanvas()->parentWidget()->parentWidget();
    mainWindow->fullScreenWindow(fullScreen);
}
