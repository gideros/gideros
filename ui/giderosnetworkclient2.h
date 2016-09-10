#ifndef GIDEROSNETWORKCLIENT2_H
#define GIDEROSNETWORKCLIENT2_H

#define GiderosNetworkClient2 GiderosNetworkClient

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QUdpSocket>

struct ProjectProperties;

class GiderosNetworkClient2 : public QObject
{
    Q_OBJECT
public:
	GiderosNetworkClient2(const QString& hostName, quint16 port, QObject* parent = 0);
	virtual ~GiderosNetworkClient2();

	void connectToHost(const QString& hostName, quint16 port);

	bool isConnected() const
	{
		return status_ == eConnected;
	}

	qint64 bytesToWrite() const;

    unsigned int sendFile(const QString& remoteName, const QString& localFileName);
	unsigned int sendCreateFolder(const QString& folderName);
	unsigned int sendPlay(const QStringList& luafiles);
	unsigned int sendProjectName(const QString& projectName);
	unsigned int sendProjectProperties(const ProjectProperties& properties);
	unsigned int sendStop();
	unsigned int sendDeleteFiles();
	unsigned int sendGetFileList();
	unsigned int sendDeleteFile(const QString& fileName);

private:
	virtual void timerEvent(QTimerEvent *);

signals:
	void connected();
	void disconnected();
	void dataReceived(const QByteArray& data);
	void ackReceived(unsigned int id);
    void advertisement(const QString&,unsigned short,unsigned short,const QString&);
    QByteArray getExpandedMacro(const QString&);

public slots:

private slots:
	void onConnected();
	void onDisconnected();
	void error(QAbstractSocket::SocketError socketError);
	void bytesWritten(qint64 bytes);
	void readyRead();

private:
	unsigned int sendData(const void* data, unsigned int size, unsigned int type);
	void sendAck(unsigned int id);

private:
	enum Status
	{
		eDisconnected,
		eConnected,
		eTrying,
	};

	QString hostName_;
	quint16 port_;
	QTcpSocket* client_;
	QUdpSocket* advertisements_;
	Status status_;
	int nextid_;
	QByteArray readArray_;
};

#endif // GIDEROSNETWORKCLIENT2_H
