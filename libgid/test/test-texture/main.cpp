#include <QtGui/QApplication>
#include "mainwindow.h"
#include <gstdio.h>
#include <gtexture.h>
#include <gvfs-native.h>
#include <gpath.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    gpath_init();

    gpath_setDriveFlags(0, GPATH_RW | GPATH_REAL);
    gpath_setDrivePath(0, "");
    gpath_setDefaultDrive(0);

    gvfs_init();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    int result = a.exec();

    gvfs_cleanup();

    gpath_cleanup();

    return result;
}
