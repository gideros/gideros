#include "giderosnetworkclient.h"
#include <QFile>
#include <bytebuffer.h>

GiderosNetworkClient::GiderosNetworkClient(const QString& hostName, quint16 const port, QObject* const parent) :
	QObject(parent),
	hostName_(hostName),
	port_(port)
{
	connected_ = false;
	nextid_ = 1;

	client_ = new QTcpSocket(this);

	connect(client_, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(client_, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(client_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	connect(client_, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
	connect(client_, SIGNAL(readyRead()), this, SLOT(readyRead()));

	client_->connectToHost(hostName_, port_);
}

GiderosNetworkClient::~GiderosNetworkClient()
{
	delete client_;
}

void GiderosNetworkClient::onConnected()
{
	printf("connected\n");
	connected_ = true;
	emit connected();
}

void GiderosNetworkClient::onDisconnected()
{
	printf("disconnected\n");
	connected_ = false;
	emit disconnected();

	readArray_.clear();

	printf("connectToHost at onDisconnected\n");
	client_->connectToHost(hostName_, port_);
}

void GiderosNetworkClient::error(QAbstractSocket::SocketError socketError)
{	
//	printf("error: %d\n", socketError);

	if (connected_ == false)
	{
		client_->abort();
//		printf("connectToHost at error\n");
		client_->connectToHost(hostName_, port_);
	}
}

void GiderosNetworkClient::readyRead()
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
			printf("unknown packet id");
	}
}

unsigned int GiderosNetworkClient::sendData(const void* data, unsigned int size, unsigned int type)
{
	if (connected_ == false)
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

void GiderosNetworkClient::sendAck(unsigned int id)
{
	if (connected_ == false)
		return;

	sendData(&id, sizeof(unsigned int), 1);
}

void GiderosNetworkClient::bytesWritten(qint64 bytes)
{
	//	printf("bytesWritten: %d\n", bytes);
}

unsigned int GiderosNetworkClient::sendFile(const QString& remoteName, const QString& localFileName)
{
	QFile file(localFileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		printf("cannot open file\n");
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

unsigned int GiderosNetworkClient::sendPlay(const QStringList& luafiles)
{
	if (connected_ == false)
		return 0;

	ByteBuffer buffer;

	buffer << (char)2;
	for (int i = 0; i < luafiles.size(); ++i)
		buffer << luafiles[i].toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}

unsigned int GiderosNetworkClient::sendProjectName(const QString& projectName)
{
	if (connected_ == false)
		return 0;

	ByteBuffer buffer;

	buffer << (char)8;
	buffer << projectName.toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}

unsigned int GiderosNetworkClient::sendStop()
{
	if (connected_ == false)
		return 0;

	char stop = 3;
	return sendData(&stop, sizeof(char), 0);
}

unsigned int GiderosNetworkClient::sendDeleteFiles()
{
	if (connected_ == false)
		return 0;

	char deleteFiles = 5;
	return sendData(&deleteFiles, sizeof(char), 0);
}

unsigned int GiderosNetworkClient::sendGetFileList()
{
	if (connected_ == false)
		return 0;

	char getFileList = 7;
	return sendData(&getFileList, sizeof(char), 0);
}

unsigned int GiderosNetworkClient::sendCreateFolder(const QString& folderName)
{
	if (connected_ == false)
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

unsigned int GiderosNetworkClient::sendDeleteFile(const QString& fileName)
{
	if (connected_ == false)
		return 0;

	ByteBuffer buffer;

	buffer << (char)9;
	buffer << fileName.toStdString();

	return sendData(buffer.data(), buffer.size(), 0);
}


void GiderosNetworkClient::connectToHost(const QString& hostName, quint16 const port)
{
	hostName_ = hostName;
	port_ = port;

	if (connected_ == false)
	{
		client_->abort();
		printf("connectToHost at connectToHost\n");
		client_->connectToHost(hostName_, port_);
	}
	else
	{
		printf("connected.. just aborting\n");
		client_->abort();
	}
}

qint64 GiderosNetworkClient::bytesToWrite() const
{
	if (client_)
		return client_->bytesToWrite();

	return 0;
}
