#include "mainwindow.h"
#include <QApplication>
#include <ghttp.h>

int main(int argc, char *argv[])
{
    gevent_Init();
    ghttp_Init();

    int result = 0;
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
    
        result = a.exec();
    }

    ghttp_Cleanup();
    gevent_Cleanup();

    return result;
}
