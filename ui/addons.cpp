#include "addons.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QStandardPaths>
#include <QProcess>
#include "lua.hpp"
std::vector<Addon> AddonsManager::addons;
lua_State *AddonsManager::L=NULL;

extern "C" {
 LUALIB_API int luaopen_cjson(lua_State *L);
}

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../Addons"
#else
#define ALL_PLUGINS_PATH "Addons"
#endif

lua_State *AddonsManager::getLua() {
	if (L) return L;
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_pushcnfunction(L, luaopen_cjson, "cjson_loader");
    lua_call(L, 0, 1);
    lua_pop(L,1);
    if (luaL_loadfilenamed(L,"Tools/StudioServer.lua","Tools/StudioServer.lua")==0) { //No Error while loading
		 if (lua_pcall(L, 0, 0, 0)==0) { //No error while running
		 }
	}
	//lua_close(L); TODO ??
	return L;
}

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

	lua_State *L=getLua();

	std::map<std::string,bool> seen;
	for (int i = 0; i < plugins.count(); i++) {
        if (luaL_loadfilenamed(L,plugins[i].toUtf8().constData(),plugins[i].toUtf8().constData())==0) { //No Error while loading
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
    	    QStringList args;
#if defined(Q_OS_MAC)
    	    process->setProgram("open");
    	    args << "../../Gideros Player.app" << "--args";
#elif defined(Q_OS_WIN)
    	    process->setProgram("GiderosPlayer.exe");
#else
    	    process->setProgram("GiderosPlayer");
#endif
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


AddonsServer::AddonsServer(QObject* parent) :
	QObject(parent)
{
	server=new QTcpServer(this);
	connect(server, SIGNAL(newConnection()), this, SLOT(onConnection()));
	server->listen();
}

AddonsServer::~AddonsServer()
{
	delete server;
}

quint16 AddonsServer::port() {
	return server->serverPort();
}

void AddonsServer::onConnection()
{
    QTcpSocket *pSocket = server->nextPendingConnection();

    connect(pSocket, SIGNAL(readyRead()), this, SLOT(onCanRead()));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(onClosed()));

    clients << pSocket;
    buffers[pSocket->peerAddress().toString()]=new QByteArray();
}

void AddonsServer::onClosed()
{
    QTcpSocket *pClient = qobject_cast<QTcpSocket *>(sender());
    if (pClient) {
        clients.removeAll(pClient);
        QByteArray *buffer=buffers.take(pClient->peerAddress().toString());
        delete buffer;
        pClient->deleteLater();
    }
}

void AddonsServer::onCanRead() {
    QTcpSocket *pClient = qobject_cast<QTcpSocket *>(sender());
    QByteArray *buffer=buffers[pClient->peerAddress().toString()];
    QByteArray pid=pClient->peerAddress().toString().toUtf8();
    while (buffer)
	{

		while (pClient->bytesAvailable() > 0)
			buffer->append(pClient->readAll());

		const unsigned int headerSize = sizeof(unsigned int) * 3;

		if (buffer->size() < headerSize)
			return;

		const unsigned int* header = (const unsigned int*)buffer->constData();

		unsigned int size = header[0];
		unsigned int id = header[1];
		unsigned int type = header[2];

		if (buffer->size() < size)
			return;

		QByteArray part0(buffer->constData() + headerSize, size - headerSize);
		const char* data = part0.constData();

		unsigned int rheader[3];
		rheader[1] = id;
		rheader[2] = -1;
		rheader[0] = headerSize;

		if (type == 0)
		{
			lua_State *L=AddonsManager::getLua();
			lua_getglobal(L,"RunStudioQuery");
            lua_pushlstring(L,pid.constData(),pid.size());
            lua_pushlstring(L,data,size-headerSize);
            if (lua_pcall(L, 2, 1, 0)==0)
				rheader[2]=0;
			const char *str=luaL_checkstring(L,-1);
			rheader[0]+=strlen(str);
            pClient->write((const char*)rheader, headerSize);
 			pClient->write(str, strlen(str));
		}
		else
        {
            //printf("unknown packet id");
             pClient->write((const char*)rheader, headerSize);
        }
		buffer->remove(0,size+headerSize);
	}
}

void AddonsServer::notify(QString clientId,const char *data) {
    foreach (QTcpSocket *pClient , clients) {
        if (clientId.isEmpty()||(pClient->peerAddress().toString()==clientId)) {
            const unsigned int headerSize = sizeof(unsigned int) * 3;
            unsigned int rheader[3];
            rheader[1] = 0;
            rheader[2] = 1;
            rheader[0] = headerSize+strlen(data);
            pClient->write((const char*)rheader, headerSize);
            pClient->write(data, strlen(data));
        }
    }
}
