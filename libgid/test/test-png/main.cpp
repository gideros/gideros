#include <QtGui/QApplication>
#include "mainwindow.h"
#include <gstdio.h>

extern "C" {
GIDLIB_API gfile_Vfs native_vfs;
}

int main(int argc, char *argv[])
{
    gfile_setVfs(native_vfs);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
