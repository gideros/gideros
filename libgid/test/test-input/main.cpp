#include <QtGui/QApplication>
#include "mainwindow.h"
#include <ginput.h>
#include <gevent.h>

int main(int argc, char *argv[])
{
    gevent_Init();
    ginput_init();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    int result = a.exec();

    ginput_cleanup();
    gevent_Cleanup();

    return result;
}
