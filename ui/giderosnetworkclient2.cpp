/*
	ui -> player
	play = 2
	stop = 3
	deleteFiles = 5 (unused)
	getFileList = 7
	deleteFile = 9
	sendProjectProperties = 11

-------------------------------

	player -> ui
	sendFileList = 6
	sendRun = 10 (unused)

*/

#include "giderosnetworkclient2.h"
#include <QDebug>
#include <QFile>
#include <bytebuffer.h>
#include <QStringList>
#include <QFileInfo>
#include "projectproperties.h"

GiderosNetworkClient2::GiderosNetworkClient2(const QString& hostName, quint16 port, QObject* parent) :
	QObject(parent),
	hostName_(hostName),
	port_(port)
{
	advertisements_ = new QUdpSocket(this);
	advertisements_->bind(15000,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);

	client_ = new QTcpSocket(this);

	connect(client_, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(client_, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(client_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	connect(client_, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
	connect(client_, SIGNAL(readyRead()), this, SLOT(readyRead()));

	status_ = eDisconnected;

	nextid_ = 1;

	startTimer(500);
}

GiderosNetworkClient2::~GiderosNetworkClient2()
{
	delete client_;
}

void GiderosNetworkClient2::onConnected()
{
    //qDebug() << "connected";
	status_ = eConnected;
	emit connected();
}

void GiderosNetworkClient2::onDisconnected()
{
    //qDebug() << "disconnected";
	status_ = eDisconnected;
	readArray_.clear();
	emit disconnected();
}

void GiderosNetworkClient2::error(QAbstractSocket::SocketError socketError)
{
    //qDebug() << "error" << socketError;
	if (status_ == eTrying)
	{
		status_ = eDisconnected;
	}
}

void GiderosNetworkClient2::bytesWritten(qint64 bytes)
{

}

void GiderosNetworkClient2::readyRead()
{
	while (1)
	{
		while (client_->bytesAvailable() > 0)
			readArray_.append(client_->readAll());

		const unsigned int headerSize = sizeof(unsigned int) * 3;

		if (readArray_.size() < headerSize)
			return;

		const unsigned int* header = (const unsigned int*)readArray_.constData();

		unsigned int size = header[0];
		unsigned int id = header[1];
		unsigned int type = header[2];

		if (readArray_.size() < size)
			return;

		QByteArray part0(readArray_.constData() + headerSize, size - headerSize);
		QByteArray part1(readArray_.constData() + size, readArray_.size() - size);
		readArray_ = part1;

		const char* data = part0.constData();

		if (type == 0)
		{
			emit dataReceived(part0);
			sendAck(id);
		}
		else if (type == 1)
		{
			emit ackReceived(*(unsigned int*)data);
		}
		else
        {
            //printf("unknown packet id");
        }
	}
}


void GiderosNetworkClient2::timerEvent(QTimerEvent *)
{
	while (advertisements_->hasPendingDatagrams())
	{
		QHostAddress host;
		unsigned char dgram[49];
		int dgramlen;
		if ((dgramlen=advertisements_->readDatagram((char *) dgram,48,&host))>=16)
		{
			dgram[dgramlen]=0;
			if (!memcmp(dgram,"Gideros0",8))
			{
				QString haddr;
				if (!((dgram[8]==0)&&(dgram[9]==0)&&(dgram[10]==0)&&(dgram[11]==0)))
					host.setAddress((dgram[8]<<24)|(dgram[9]<<16)|(dgram[10]<<8)|(dgram[11]<<0));
				unsigned short port=(dgram[12]<<8)|(dgram[13]);
				unsigned short flags=(dgram[14]<<8)|(dgram[15]);

				QString name="";
				if ((dgramlen>16)&&dgram[16])
					name=QString((char *)(dgram+16));

				emit advertisement(host.toString(),port,flags,name);
			}
		}
	}
	if (status_ == eDisconnected)
	{
		status_ = eTrying;
		client_->connectToHost(hostName_, port_);
	}
}

void GiderosNetworkClient2::connectToHost(const QString& hostName, quint16 port)
{
	hostName_ = hostName;
	port_ = port;

	switch (status_)
	{
	case eDisconnected:
		break;
	case eConnected:
		client_->disconnectFromHost();
		break;
	case eTrying:
		client_->abort();
		status_ = eDisconnected;
		break;
	}
}


unsigned int GiderosNetworkClient2::sendData(const void* data, unsigned int size, unsigned int type)
{
	if (status_ != eConnected)
		return 0;

	const unsigned int headerSize = sizeof(unsigned int) * 3;

	unsigned int header[3];
	header[0] = headerSize + size;
	header[1] = nextid_++;
	header[2] = type;		// 0=>we are sending data, 1=>ack

	client_->write((const char*)header, headerSize);
	client_->write((const char*)data, size);

	return header[1];
}

void GiderosNetworkClient2::sendAck(unsigned int id)
{
	if (status_ != eConnected)
		return;

	sendData(&id, sizeof(unsigned int), 1);
}


unsigned int GiderosNetworkClient2::sendFile(const QString& remoteName, const QString& localFileName)
{
	if (status_ != eConnected)
		return 0;

	QFile file(localFileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
        //printf("cannot open file\n");
		return 0;
	}

	std::string n = remoteName.toStdString();
    QByteArray b = file.readAll();

	std::vector<char> buffer(1 + n.size() + 1 + b.size());

	int pos = 0;
	buffer[pos] = 1;								// 1 for file
	pos += 1;
	strcpy(&buffer[pos], n.c_str());				// name
	pos += n.size() + 1;
	if (b.isEmpty() == false)
		memcpy(&buffer[pos], b.constData(), b.size());	// data
	pos += b.size();

	Q_ASSERT(buffer.size() == pos);

	return sendData(&buffer[0], buffer.size(), 0);
}

unsigned int GiderosNetworkClient2::sendPlay(const QStringList& luafiles)
{
	if (status_ != eConnected)
		return 0;

	ByteBuffer buffer;

	buffer << (char)2;
	for (int i = 0; i < luafiles.size(); ++i)
		buffer << luafiles[i].toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}

unsigned int GiderosNetworkClient2::sendProjectName(const QString& projectName)
{
	if (status_ != eConnected)
		return 0;

	ByteBuffer buffer;

	buffer << (char)8;
	buffer << projectName.toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}

unsigned int GiderosNetworkClient2::sendStop()
{
	if (status_ != eConnected)
		return 0;

	char stop = 3;
	return sendData(&stop, sizeof(char), 0);
}

unsigned int GiderosNetworkClient2::sendDeleteFiles()
{
	if (status_ != eConnected)
		return 0;

	char deleteFiles = 5;
	return sendData(&deleteFiles, sizeof(char), 0);
}

unsigned int GiderosNetworkClient2::sendGetFileList()
{
	if (status_ != eConnected)
		return 0;

	char getFileList = 7;
	return sendData(&getFileList, sizeof(char), 0);
}

unsigned int GiderosNetworkClient2::sendCreateFolder(const QString& folderName)
{
	if (status_ != eConnected)
		return 0;

	std::string n = folderName.toStdString();

	std::vector<char> buffer(1 + n.size() + 1);
	int pos = 0;
	buffer[pos] = 0;						// 0 for folder
	pos += 1;
	strcpy(&buffer[pos], n.c_str());		// name
	pos += n.size() + 1;

	Q_ASSERT(buffer.size() == pos);

	return sendData(&buffer[0], buffer.size(), 0);
}

unsigned int GiderosNetworkClient2::sendDeleteFile(const QString& fileName)
{
	if (status_ != eConnected)
		return 0;

	ByteBuffer buffer;

	buffer << (char)9;
	buffer << fileName.toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}

qint64 GiderosNetworkClient2::bytesToWrite() const
{
	if (client_)
		return client_->bytesToWrite();

	return 0;
}

unsigned int GiderosNetworkClient2::sendProjectProperties(const ProjectProperties& properties)
{
	if (status_ != eConnected)
		return 0;

	ByteBuffer buffer;

	buffer << (char)11;
	buffer << properties.scaleMode;
	buffer << properties.logicalWidth;
	buffer << properties.logicalHeight;

	buffer << (int)properties.imageScales.size();
	for (size_t i = 0; i < properties.imageScales.size(); ++i)
	{
		buffer << properties.imageScales[i].first.toUtf8().constData();
		buffer << (float)properties.imageScales[i].second;
	}

	buffer << properties.orientation;
	buffer << properties.fps;

    buffer << properties.retinaDisplay;
	buffer << properties.autorotation;

    buffer << (properties.mouseToTouch ? 1 : 0);
    buffer << (properties.touchToMouse ? 1 : 0);
    buffer << properties.mouseTouchOrder;
    
    buffer << properties.windowWidth;
    buffer << properties.windowHeight;

	return sendData(buffer.data(), buffer.size(), 0);
}
