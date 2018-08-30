#include "addons.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QStandardPaths>
#include <QProcess>
#include "lua.hpp"
std::vector<Addon> AddonsManager::addons;

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../Addons"
#else
#define ALL_PLUGINS_PATH "Addons"
#endif

std::vector<Addon> AddonsManager::loadAddons(bool refresh) {
	if (!refresh&&(addons.size()))
		return addons;
	addons.clear();

	QStringList dirs;
	QStringList plugins;

	QDir shared(
			QStandardPaths::writableLocation(
					QStandardPaths::GenericDataLocation));
	shared.mkpath("Gideros/UserAddons");
	bool sharedOk = shared.cd("Gideros");
	sharedOk=sharedOk&&shared.cd("UserAddons");
	if (sharedOk) {
		dirs = shared.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
		for (int i = 0; i < dirs.count(); i++) {
			QDir sourceDir2 = shared;
			if (sourceDir2.cd(dirs[i])) {
				QStringList filters;
				filters << "manifest.lua";
				sourceDir2.setNameFilters(filters);
				QStringList files = sourceDir2.entryList(
						QDir::Files | QDir::Hidden);
				for (int i = 0; i < files.count(); i++)
					plugins << sourceDir2.absoluteFilePath(files[i]);
			}
		}
	}

	QDir sourceDir(ALL_PLUGINS_PATH);
	dirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirs.count(); i++) {
		QDir sourceDir2 = sourceDir;
		if (sourceDir2.cd(dirs[i])) {
			QStringList filters;
			filters << "manifest.lua";
			sourceDir2.setNameFilters(filters);
			QStringList files = sourceDir2.entryList(
					QDir::Files | QDir::Hidden);
			for (int i = 0; i < files.count(); i++)
				plugins << sourceDir2.absoluteFilePath(files[i]);
		}
	}

	 lua_State *L = luaL_newstate();
	 luaL_openlibs(L);
	 //lua_pushcfunction(ctx->L, bindAll);
	 //lua_call(ctx->L, 0, 0);

	std::map<std::string,bool> seen;
	for (int i = 0; i < plugins.count(); i++) {
		if (luaL_loadfile(L,plugins[i].toUtf8().constData())==0) { //No Error while loading
			 if (lua_pcall(L, 0, 1, 0)==0) { //No error while running
				 Addon a(QFileInfo(plugins[i]).absolutePath().toStdString());
				 lua_getfield(L,-1,"name"); const char *name=luaL_optstring(L,-1,NULL); lua_pop(L,1);
				 lua_getfield(L,-1,"title"); const char *title=luaL_optstring(L,-1,NULL); lua_pop(L,1);
				 lua_getfield(L,-1,"gapp"); const char *gapp=luaL_optstring(L,-1,NULL); lua_pop(L,1);
				 lua_getfield(L,-1,"extensions"); if (!lua_isnil(L,-1)) {
					 int elen=lua_objlen(L,-1);
					 for (int k=1;k<=elen;k++)
					 {
						lua_rawgeti(L,-1,k);
						const char *ext=luaL_optstring(L,-1,NULL);
						lua_pop(L,1);
						if (ext)
							a.exts.push_back(ext);
					 }
				 } lua_pop(L,1);
				 if (name&&title&&!seen[name])
				 {
					 a.name=name;
					 a.title=title;
					 if (gapp) a.gapp=gapp;
					 addons.push_back(a);
					 seen[name]=true;
				 }
			 }
			 else {
				 //TODO display error ?
			 }
			 lua_pop(L,1);
		}
		else
		 {
						 //TODO display error ?
		 }
	}
	lua_close(L);
    return addons;
}

std::string AddonsManager::addonForExtension(std::string ext) {
    for (std::vector<Addon>::iterator it=addons.begin();it!=addons.end();it++) {
        for (std::vector<std::string>::iterator it2=it->exts.begin();it2!=it->exts.end();it2++) {
        	if (*it2==ext)
        		return it->name;
        }
    }
    return std::string();
}

void AddonsManager::launch(std::string name,std::string envs) {
    for (std::vector<Addon>::iterator it=addons.begin();it!=addons.end();it++) {
    	if (it->name==name){
    	    QProcess *process = new QProcess();
#if defined(Q_OS_MAC)
    	    process->setProgram("open \"../../Gideros Player.app\"");
#elif defined(Q_OS_WIN)
    	    process->setProgram("GiderosPlayer.exe");
#else
    	    process->setProgram("GiderosPlayer");
#endif
    	    QStringList args;
    	    if (!it->gapp.empty())
    	    	args << QString::fromStdString(it->getGApp());
    	    process->setArguments(args);
    	    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    	    env.insert("GIDEROS_STUDIO_DATA", QString::fromStdString(envs));
    	    process->setProcessEnvironment(env);
            process->startDetached(nullptr);
    	    delete process;
    	}
    }
}

