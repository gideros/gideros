#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <gfile.h>
#include <unzip.h>
#include <pluginmanager.h>
#include <QDebug>
#include <QLibrary>
#include <lua.hpp>
#include <ghttp.h>
#include <gpath.h>
#include <gstdio.h>
#include <gvfs-native.h>
#include <gui.h>
#include <ginput.h>
#include <gevent.h>
#include <ggeolocation.h>
#include <gtexture.h>
#include <gapplication.h>
#include <gaudio.h>
#include <memory.h>
#include <glog.h>
#include "constants.cpp"
//#include <curl/curl.h>



static void loadPlugins(){
	QDir dir = QDir::currentPath();

    #if defined(Q_OS_MAC)
        dir.cd("../../Plugins");
        QStringList files = dir.entryList(QStringList() << "*.dylib");
    #elif defined(Q_OS_LINUX)
        dir.cd("Plugins");
        QStringList files = dir.entryList(QStringList() << "*.so");
    #else
        dir.cd("Plugins");
        QStringList files = dir.entryList(QStringList() << "*.dll");
    #endif

    for(int i = 0; i < files.size(); ++i){
		QString filename = dir.absoluteFilePath(files[i]);
		QLibrary library(filename);
        void* plugin = (void*)library.resolve("g_pluginMain");
		if (plugin)
			PluginManager::instance().registerPlugin((void*(*)(lua_State*, int))plugin);
	}
}



int main(int argc, char *argv[]){
//	curl_global_init(CURL_GLOBAL_WIN32);

    QCoreApplication::setOrganizationName(Constants::ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(Constants::ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(Constants::PLAYER_APPLICATION_NAME);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication a(argc, argv);

	QDir dir = QCoreApplication::applicationDirPath();

    #if defined(Q_OS_MAC)
        dir.cdUp();
    #endif

	QDir::setCurrent(dir.absolutePath());

    gpath_init();

    gpath_addDrivePrefix(0, "|R|");
    gpath_addDrivePrefix(0, "|r|");
    gpath_addDrivePrefix(1, "|D|");
    gpath_addDrivePrefix(1, "|d|");
    gpath_addDrivePrefix(2, "|T|");
    gpath_addDrivePrefix(2, "|t|");

    gpath_setDriveFlags(0, GPATH_RO | GPATH_REAL);
    gpath_setDriveFlags(1, GPATH_RW | GPATH_REAL);
    gpath_setDriveFlags(2, GPATH_RW | GPATH_REAL);

    gpath_setAbsolutePathFlags(GPATH_RW | GPATH_REAL);

    gpath_setDefaultDrive(0);

    gvfs_init();

    gevent_Init();

    gapplication_init();

    ginput_init();

    ggeolocation_init();

    ghttp_Init();

    gui_init();

    gtexture_init();

    gaudio_Init();

    loadPlugins();

/*	if (argc > 1)
        setZipFile(argv[1]); */

    if (argc>1)
    	GLCanvas::appPackage=argv[1];

	int result = 0;
	{
		MainWindow w;
		w.show();
		result = a.exec();
	}

    gaudio_Cleanup();

    gtexture_cleanup();

    gui_cleanup();

    ghttp_Cleanup();

    ggeolocation_init();

    ginput_cleanup();

    gapplication_cleanup();

    gevent_Cleanup();

    gvfs_cleanup();

    gpath_cleanup();

//	curl_global_cleanup();

	return result;
}



/*
this method should be added to unzip.c:

extern uLong unzGetOffset2(unzFile file)
{
    unz64_s* s;
    s=(unz64_s*)file;

    return s->pfile_in_zip_read->pos_in_zipfile + s->pfile_in_zip_read->byte_before_the_zipfile;
}

*/

/*
static void setZipFile(const char* filename)
{
    unzFile file = unzOpen(filename);
    if (file == NULL)
        return;

    g_clearfilepos();

    if (unzGoToFirstFile(file) != UNZ_OK)
        return;

    while (true)
    {
        unz_file_info info;
        char filename[256];
        unzGetCurrentFileInfo(file, &info, filename, 255, NULL, 0, NULL, 0);

        bool uisdir = (info.external_fa & 0x40000000) != 0;
        bool wisdir = (info.external_fa & 0x00000010) != 0;
        bool isdir = uisdir | wisdir;

        if (!isdir && info.compression_method == 0)
        {
            if (unzOpenCurrentFile(file) == UNZ_OK)
            {
                uLong offset = unzGetOffset2(file);
                g_setfilepos(filename, offset, info.uncompressed_size);
                unzCloseCurrentFile(file);
            }
        }

        if (unzGoToNextFile(file) != UNZ_OK)
            break;
    }

    g_setbigfile(filename);
}
*/
