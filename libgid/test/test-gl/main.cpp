#include <QtGui/QApplication>
#include "mainwindow.h"
#include <ggl.h>
#include <gpath.h>
#include <gvfs-native.h>

int main(int argc, char *argv[])
{
    gpath_init();

    gpath_setDriveFlags(0, GPATH_RW | GPATH_REAL);
    gpath_setDrivePath(0, "");
    gpath_setDefaultDrive(0);

    gvfs_init();

    ggl_Init();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    int result = a.exec();

    ggl_Cleanup();

    gvfs_cleanup();

    gpath_cleanup();

    return result;
}
