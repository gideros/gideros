#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QSettings>
#include <QLocalSocket>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QStringList arguments = a.arguments();

	bool sendrun = false;
	bool sendstop = false;

	for (int i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i] == "--sendrun")
			sendrun = true;
		else if (arguments[i] == "--sendstop")
			sendstop = true;
	}

	if (sendrun || sendstop)
	{
		QLocalSocket socket;
		socket.connectToServer("GiderosProjectManager");
		bool connected = socket.waitForConnected(1000);

		char c;
		if (sendrun)
			c = 0;
		else if (sendstop)
			c = 1;

		socket.write(&c, 1);

		socket.disconnectFromServer();
		socket.waitForDisconnected(1000);

		return 0;
	}

	QCoreApplication::setOrganizationName("Betikus");
	QCoreApplication::setOrganizationDomain("www.betikus.com");
	QCoreApplication::setApplicationName("Gideros");

	QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindow w;
    w.show();

    return a.exec();
}
