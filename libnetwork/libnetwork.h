#ifndef LIBNETWORK_H
#define LIBNETWORK_H

#include <deque>
#include <string>
#include <vector>
#include <time.h>

#define GIDEROS_DEFAULT_PORT	15000

enum GiderosPacketType {
	gptMakeDir = 0,
	gptWriteFile = 1,
	gptPlay = 2,
	gptStop = 3,
	gptPrint = 4, //Player->Studio
	//gptDelete=5, //UNUSED
	gptFileList=6, //Player->Studio
	gptGetFileList=7,
	gptSetProjectName=8,
	gptDeleteFile=9,
	gptRunning=10, //Player->Studio
	gptSetProperties=11,

	//Debugging
	gptLookupSymbol=20,
	gptSetBreakpoints=21,
	gptResume=22,
	gptBreaked=23, //Player->Studio
	gptSymbolValue=24, //Player->Studio

	gptLast
};

#ifdef WINSTORE

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#elif WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
typedef int SOCKET;
#endif

enum EventCode
{
	eNone,

	eFirstError,
	eCreateSocketError,
	eSetReuseAddrError,
	eBindError,
	eListenError,
	eAcceptError,
	eOtherSideClosedConnection,
	eConnectError,
	eLastError,

	eOtherSideConnected,
	eDataReceived,
	eDataSent,
};

const char* eventCodeString(EventCode eventCode);

struct NetworkEvent
{
	EventCode eventCode;
	std::vector<char> data;
	unsigned int id;
};

class QueueElement;

class NetworkBase
{
public:
	int sendData(const void* data, unsigned int size, bool noCheck=false);
	void cancelSend();

	SOCKET clientSock() const
	{
		return clientSock_;
	}

	bool isConnected() const;

	int dataSent() const
	{
		return dataSent_;
	}
	
	int dataReceived() const
	{
		return dataReceived_;
	}
	
/*
  public:
	virtual void dataReceived(int id, const char* data, unsigned int size) {}
	//virtual void dataSent(int id) {}
	virtual void connected() {}
	virtual void disconnected() {}
*/

	
protected:
	NetworkBase();
	~NetworkBase();

	void tickSend(NetworkEvent* event);
	void tickRecv(NetworkEvent* event);

	void sendAck(unsigned int id);

	SOCKET clientSock_;
	unsigned short port_;

	std::deque<QueueElement*> sendQueue_;

	int dataSent_;
	int dataReceived_;

	void cleanup(void);

	char* receiveData_;
	unsigned int receiveId_;
	unsigned int receiveSize_;
	unsigned int receiveCurrent_;
	unsigned int receiveType_;
};

class Server : public NetworkBase
{
public:
	Server(unsigned short port,const char *name=NULL);
	~Server();

	void tick(NetworkEvent* event);

	SOCKET serverSock() const
	{
		return serverSock_;
	}
    
private:
	SOCKET serverSock_;
    SOCKET broadcastSock_;
    time_t lastBcastTime_;
    char deviceName_[32];

	void cleanup(void);
	void advertise();
};

class Client : public NetworkBase
{
public:
	Client(const char* ip, unsigned short port);
	~Client();

	void tick(NetworkEvent* event);

private:
	std::string ip_;
	bool connecting_;

	void cleanup(void);
};

#endif
