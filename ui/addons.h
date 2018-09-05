#ifndef ADDONS_H
#define ADDONS_H

#include <string>
#include <set>
#include <vector>
#include <map>
#include "lua.hpp"

class Addon {
private:
	std::string path;
public:
	std::string name;
	std::string title;
	std::string gapp;
	std::vector<std::string> exts;
	Addon(std::string path) { this->path=path; };
	std::string getGApp() { return path+"/"+gapp; };
};

class AddonsManager
{

public:
    static std::vector<Addon> loadAddons(bool refresh);
    static void launch(std::string name, std::string env);
    static std::string addonForExtension(std::string ext);
    static lua_State *getLua();
private:
    static std::vector<Addon> addons;
    static lua_State *L;
};

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include <QHostAddress>
class AddonsServer : public QObject
{
    Q_OBJECT
public:
	AddonsServer(QObject* parent = 0);
	virtual ~AddonsServer();
	quint16 port();
    void notify(QString clientId,const char *data);


private slots:

	void onConnection();
	void onClosed();
	void onCanRead();

private:
	QTcpServer *server;
	QList<QTcpSocket *> clients;
	QMap<QString,QByteArray *> buffers;
};


#endif
