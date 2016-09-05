#define NOMINMAX

#include "libnetwork.h"
#include <stdlib.h>
#include <memory.h>
#include <algorithm>
#include <time.h>
#include <stdint.h>

#define ADVERTISE_PERIOD	5	//Advertise every 5 seconds
//#define IPV6 //enable this when IPv6 support is working
/*
BSD behavior
 
-1 EINPROGRESS
-1 EALREADY *
0
-1 EISCONN *
 
       If the connection cannot be established immediately and  O_NONBLOCK  is  set
       for  the  file descriptor for the socket, connect() shall fail and set errno
       to [EINPROGRESS], but the connection request shall not be aborted,  and  the
       connection  shall  be  established  asynchronously. Subsequent calls to con?
       nect() for the same socket, before the connection is established, shall fail
       and set errno to [EALREADY].
 
       EINPROGRESS
              O_NONBLOCK is set for the file descriptor for the socket and the con?
              nection  cannot  be  immediately established; the connection shall be
              established asynchronously.
       EALREADY
              A connection request is already in progress for the specified socket.
       EISCONN
              The specified socket is connection-mode and is already connected.
 
Winsock behavior
 
-1 WSAEWOULDBLOCK                     | WSAEINVAL (?)
-1 WSAEALREADY *                             | WSAEINVAL (?)
-1 WSAEISCONN
-1 WSAEISCONN *
*/  


#if defined(WIN32) || defined(_WIN32)

typedef int socklen_t;

// windows
#define BACKLOG SOMAXCONN

#define EWOULDBLOCK2 WSAEWOULDBLOCK
#define EALREADY2 WSAEALREADY
#define EINPROGRESS2 WSAEINPROGRESS
#define EISCONN2 WSAEISCONN
#define EINVAL2 WSAEINVAL

#else


// unix

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define BACKLOG 1 //One connection allowed
#define INVALID_SOCKET (-1)
static int closesocket(SOCKET s)
{
	return close(s);
}

#define EWOULDBLOCK2 EWOULDBLOCK
#define EALREADY2 EALREADY
#define EINPROGRESS2 EINPROGRESS
#define EISCONN2 EISCONN
#define EINVAL2 EINVAL
#endif

#if __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef WIN32

static int getError()
{
	return WSAGetLastError();
}

static int initialize(void)
{
	WORD versionRequested = MAKEWORD (2, 2);
	WSADATA wsaData;

	if (WSAStartup (versionRequested, & wsaData))
		return -1;

	if (LOBYTE (wsaData.wVersion) != 2||
		HIBYTE (wsaData.wVersion) != 2)
	{
		WSACleanup ();

		return -1;
	}

	return 0;
}

static void deinitialize(void)
{
	WSACleanup ();
}

static void setNonBlocking(SOCKET socket, bool block)
{
	u_long iMode = block ? 1 : 0;
	ioctlsocket(socket, FIONBIO, &iMode);
}


#else

//unix

static int getError(void)
{
	return errno;
}

static int initialize(void)
{
	return 0;
}

static void deinitialize(void)
{

}

static void setNonBlocking(SOCKET socket, bool block)
{
	fcntl(socket, F_SETFL, block ? O_NONBLOCK : 0);
}

#endif

static unsigned int nextid = 0;

class QueueElement
{
public:
	QueueElement(const void* data, unsigned int size, unsigned int type)
	{
		const unsigned int headerSize = sizeof(unsigned int) * 3;

		size_ = headerSize + size;
		id_ = nextid++;
		data_ = (char*)malloc(size_);

		unsigned int* header = (unsigned int*)data_;
		header[0] = size_;
		header[1] = id_;
		header[2] = type;		// 0=>we are sending data, 1=>ack
		memcpy(data_ + headerSize, data, size);

		currentPosition_ = 0;
	}
	
	~QueueElement()
	{
		free(data_);
	}

	unsigned int id() const
	{
		return id_;
	}

	bool finished() const
	{
		return currentPosition_ == size_;
	}

	int send(SOCKET socket)
	{
		int bytesToSend = std::min(size_ - currentPosition_, 1024U * 1024U);

		if (bytesToSend == 0)
			return 0;

		int bytes = ::send(socket, data_ + currentPosition_, bytesToSend, 0);

		if (bytes == -1)
		{
			if (getError() == EWOULDBLOCK2)
			{
				// only send queue is full, nothing to worry about
				return 0;
			}
			else
			{
				// most probably other side closed the connection and errno is EPIPE
				return -1;
			}
		}
		else
		{
			currentPosition_ += bytes;
		}

		return bytes;
	}

private:
	char* data_;
	unsigned int size_;
	unsigned int id_;
	unsigned int currentPosition_;
};

static bool isInitialized = false;

NetworkBase::NetworkBase()
{
	if (isInitialized == false)
	{
		initialize();
		atexit(deinitialize);
		isInitialized = true;
	}

	clientSock_ = INVALID_SOCKET;

	receiveData_ = 0;
	receiveCurrent_ = 0;

	dataSent_ = 0;
	dataReceived_ = 0;
}

NetworkBase::~NetworkBase()
{
	cleanup();
}

bool NetworkBase::isConnected() const
{
	return clientSock_ != INVALID_SOCKET;
}

void NetworkBase::cleanup(void)
{
	if (clientSock_ != INVALID_SOCKET)
	{
		setNonBlocking(clientSock_, false);
		closesocket(clientSock_);
		clientSock_ = INVALID_SOCKET;
	}

	while (sendQueue_.empty() == false)
	{
		QueueElement* queueElement = sendQueue_.front();
		delete queueElement;
		sendQueue_.pop_front();
	}

	dataSent_ = 0;
	dataReceived_ = 0;

	free(receiveData_);
	receiveData_ = 0;
	receiveCurrent_ = 0;
}


void NetworkBase::tickSend(NetworkEvent* event)
{
	// send
	if (sendQueue_.empty() == false)
	{
		QueueElement* queueElement = sendQueue_.front();

		int sent = queueElement->send(clientSock_);
		
		if (sent == -1)
		{
			cleanup();
			event->eventCode = eOtherSideClosedConnection;
			return;
		}

		dataSent_ += sent;
		
		if (queueElement->finished() == true)
		{
			delete queueElement;
			sendQueue_.pop_front();
		}
	}
}

void NetworkBase::tickRecv(NetworkEvent* event)
{
	const unsigned int headerSize = sizeof(unsigned int) * 3;
	if (receiveData_ == 0)
	{
		// first allocate data for header
		receiveData_ = (char*)malloc(headerSize);
		receiveCurrent_ = 0;
	}

	if (receiveCurrent_ < headerSize)
	{
		// header has not arrived yet, try to get the header
		unsigned int bytesToReceive = headerSize - receiveCurrent_;
		int bytes = recv(clientSock_, receiveData_ + receiveCurrent_, bytesToReceive, 0);

		if (bytes == 0)
		{
			// other side closed the connection gracefully
			cleanup();
			event->eventCode = eOtherSideClosedConnection;
			return;
		}
		else if (bytes == -1)
		{
			if (getError() == EWOULDBLOCK2)
			{
				// nothing in receive queue, nothing to worry about, just return
			}
			else
			{
				// there is another error
				cleanup();
				event->eventCode = eOtherSideClosedConnection; // most probably other side closed the connection ungracefully.
				return;
			}
		}
		else if (bytes > 0)
		{
			dataReceived_ += bytes;

			receiveCurrent_ += bytes;

			// we grab the header, reallocate to full size
			if (receiveCurrent_ == headerSize)
			{
				unsigned int* header = (unsigned int*)receiveData_;
				receiveSize_ = header[0];
				receiveId_ = header[1];
				receiveType_ = header[2];

				receiveData_ = (char*)realloc(receiveData_, receiveSize_);
			}
		}
	}
	else
	{
		// continue to recv data
		unsigned int bytesToReceive = receiveSize_ - receiveCurrent_;
		int bytes = recv(clientSock_, receiveData_ + receiveCurrent_, bytesToReceive, 0);

		if (bytes == 0)
		{
			// other side closed the connection gracefully
			cleanup();
			event->eventCode = eOtherSideClosedConnection;
			return;
		}
		else if (bytes == -1)
		{
			if (getError() == EWOULDBLOCK2)
			{
				// nothing in receive queue, nothing to worry about, just return
			}
			else
			{
				// there is another error
				cleanup();
				event->eventCode = eOtherSideClosedConnection; // most probably other side closed the connection ungracefully
				return;
			}
		}
		else if (bytes > 0)
		{
			dataReceived_ += bytes;

			receiveCurrent_ += bytes;
			
			if (receiveCurrent_ == receiveSize_)
			{
				// we finished receiving the data
				if (receiveType_ == 0)
				{
					event->eventCode = eDataReceived;

					event->data.resize(receiveSize_ - headerSize);
					memcpy(&event->data[0], receiveData_ + headerSize, receiveSize_ - headerSize);

					sendAck(receiveId_);
				}
				else if (receiveType_ == 1)
				{
					event->eventCode = eDataSent;
					event->id = *(unsigned int*)(receiveData_ + headerSize);
				}
				

				free(receiveData_);
				receiveData_ = 0;
			}
		}
	}
}

int NetworkBase::sendData(const void* data, unsigned int size, bool noCheck)
{
    if (!noCheck) {
        if (isConnected() == false)
            return -1;
    } else
        if (sendQueue_.size() > 1024) //Avoid queue size beginning too big in forced mode (print mainly)
            return -1;

	QueueElement* queueElement = new QueueElement(data, size, 0);
	sendQueue_.push_back(queueElement);
	return queueElement->id();
}


void NetworkBase::cancelSend()
{
	if (isConnected() == false)
		return;

	// delete all except the front element (front element is currently sending. we cannot cancel it)
	while (sendQueue_.size() > 1)
	{
		QueueElement* queueElement = sendQueue_.back();
		sendQueue_.pop_back();
		delete queueElement;
	}
}



void NetworkBase::sendAck(unsigned int id)
{
	if (isConnected() == false)
		return;

	QueueElement* queueElement = new QueueElement(&id, sizeof(id), 1);
	sendQueue_.push_back(queueElement);
}


SOCKET makeBroadcastSocket()
{
#ifdef IPV6
    SOCKET sock= socket(PF_INET6, SOCK_DGRAM,0);
	setsockopt(sock, SOL_SOCKET, IPV6_V6ONLY, 0, sizeof(int)); //I change the socket option IPV6_V6ONLY to false, so it should be compatible with IPv4
#else
    SOCKET sock= socket(PF_INET, SOCK_DGRAM,0);
#endif	
    int bcast=1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *) (&bcast),  sizeof(bcast));
#ifdef TARGET_OS_IPHONE
    int set = 1;
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
 	return sock;
}

Server::Server(unsigned short port,const char *name)
{
	port_ = port;
	serverSock_ = INVALID_SOCKET;
    lastBcastTime_=0;
    broadcastSock_=makeBroadcastSocket();
    if (name)
    	strncpy(deviceName_,name,32);
    else
    	deviceName_[0]=0;
}

Server::~Server()
{
	cleanup();
}

static const uint8_t llbcast[16] = { 0xFF,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
void Server::advertise()
{
    time_t ctime=time(NULL);
    if ((broadcastSock_ != INVALID_SOCKET)&&
    		((lastBcastTime_<=(ctime-ADVERTISE_PERIOD))||
    				(lastBcastTime_>ctime)))
    {
        lastBcastTime_=ctime;
        struct adv_ {
            uint8_t signature[8]; // 'Gideros0'
            uint32_t ip; //INADDR_ANY for same as UDP peer
            uint16_t port;
            uint16_t flags;
            char devName[32];
        } advPacket;
        memcpy(advPacket.signature,"Gideros0",8);
        advPacket.ip=htonl(INADDR_ANY);
        advPacket.port=htons(port_);
        advPacket.flags=0; // May be used to differentiate player platform/type
        memcpy(advPacket.devName,deviceName_,32);

        int error=1;

		#ifdef IPV6
        	sockaddr_in6 ai_addr;
        	memset(&ai_addr, 0, sizeof(ai_addr));
        	ai_addr.sin6_family = AF_INET6;
        	memcpy(&ai_addr.sin6_addr, llbcast, 16);
        	ai_addr.sin6_port = htons(GIDEROS_DEFAULT_PORT);
		#else
        	sockaddr_in ai_addr;
        	memset(&ai_addr, 0, sizeof(ai_addr));
        	ai_addr.sin_family = AF_INET;
        	ai_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
        	ai_addr.sin_port = htons(GIDEROS_DEFAULT_PORT);
        #endif
       	if (sendto(broadcastSock_,(char *) &advPacket,sizeof(advPacket),0,(sockaddr *)&ai_addr,sizeof(ai_addr))>0)
       		error=0;
        if (error)
        {
          //Recreate broadcast socket
            closesocket(broadcastSock_);
            broadcastSock_=makeBroadcastSocket();
        }
    }
}

void Server::tick(NetworkEvent* event)
{
	event->eventCode = eNone;

	// try to initialize	
	if (serverSock_ == INVALID_SOCKET && clientSock_ == INVALID_SOCKET)
	{
#ifdef IPV6	
        serverSock_ = socket(PF_INET6, SOCK_STREAM, 0);
    	setsockopt(serverSock_, SOL_SOCKET, IPV6_V6ONLY, 0, sizeof(int)); //I change the socket option IPV6_V6ONLY to false, so it should be compatible with IPv4
#else
        serverSock_ = socket(PF_INET, SOCK_STREAM, 0);
#endif    	
		if (serverSock_ == INVALID_SOCKET)
		{
			cleanup();
			event->eventCode = eCreateSocketError;
			return;
		}

		if (port_)
		{
			int yes = 1;
			if (setsockopt(serverSock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
			{
				cleanup();
				event->eventCode = eSetReuseAddrError;
				return;
			}
		}

#ifdef IPV6
		sockaddr_in6 ai_addr;
		memset(&ai_addr, 0, sizeof(ai_addr));
		ai_addr.sin6_family = AF_INET6;
		memcpy(&ai_addr.sin6_addr, &in6addr_any, 16);
		ai_addr.sin6_port = htons(port_?port_:GIDEROS_DEFAULT_PORT);
#else
		sockaddr_in ai_addr;
		memset(&ai_addr, 0, sizeof(ai_addr));
		ai_addr.sin_family = AF_INET;
		ai_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		ai_addr.sin_port = htons(port_?port_:GIDEROS_DEFAULT_PORT);
#endif		
		if (bind(serverSock_, (sockaddr *)&ai_addr, sizeof(ai_addr)) == -1)
		{
			if (port_)
			{
				cleanup();
				event->eventCode = eBindError;
				return;
			}
			else
			{
				//No specific port requested, so try again letting the system choose a port
				memset(&ai_addr, 0, sizeof(ai_addr));
#ifdef IPV6
				ai_addr.sin6_family = AF_INET6;
				memcpy(&ai_addr.sin6_addr, &in6addr_any, 16);
				ai_addr.sin6_port = 0;
#else
				ai_addr.sin_family = AF_INET;
				ai_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				ai_addr.sin_port = 0;
#endif				
				if (bind(serverSock_, (sockaddr *)&ai_addr, sizeof(ai_addr)) == -1)
				{
					cleanup();
					event->eventCode = eBindError;
				}
				//Retrieve the choosen port
				socklen_t ai_len=sizeof(ai_addr);
				if (getsockname(serverSock_, (sockaddr *)&ai_addr, &ai_len) == -1)
				{
					cleanup();
					event->eventCode = eBindError;
				}
#ifdef IPV6				
				port_=ntohs(ai_addr.sin6_port);
#else
				port_=ntohs(ai_addr.sin_port);
#endif				
			}
		}
		else
		{
			if (port_==0)
				port_=GIDEROS_DEFAULT_PORT;
		}

		if (listen(serverSock_, BACKLOG) == -1)
		{
			cleanup();
			event->eventCode = eListenError;
			return;
		}

		setNonBlocking(serverSock_, true);
	}

	// try to accept
	if (serverSock_ != INVALID_SOCKET && clientSock_ == INVALID_SOCKET)
	{
#ifdef IPV6
		sockaddr_in6 client_addr;
#else
		sockaddr_in client_addr;
#endif		
		socklen_t sizeof_client_addr = sizeof(client_addr);
		clientSock_ = accept(serverSock_, (sockaddr *)&client_addr, &sizeof_client_addr);
		if (clientSock_ == INVALID_SOCKET)
		{
			if (getError() == EWOULDBLOCK2)
			{
				// no connections are present to be accepted, everything is ok, return
				advertise();
				return;
			}
			else
			{
				// there is another error
				cleanup();
				event->eventCode = eAcceptError;
				return;
			}
		}

		setNonBlocking(clientSock_, true);

#if 0 //No let the socket open to prevent reuse of the port
		// we close the listening serverSock_ in order to prevent more incoming connections on the same port
		setNonBlocking(serverSock_, false);
		closesocket(serverSock_);
		serverSock_ = INVALID_SOCKET;
#endif
		event->eventCode = eOtherSideConnected;
		return;
	}

	// send and recv
	if (clientSock_ != INVALID_SOCKET)
	{
		advertise(); //Advertise even when connected, to let studios know we are alive
		tickRecv(event);
		if (eFirstError < event->eventCode && event->eventCode < eLastError)
		{
			cleanup();
			return;
		}

		if (event->eventCode != eNone)
			return;

		tickSend(event);
		if (eFirstError < event->eventCode && event->eventCode < eLastError)
		{
			cleanup();
			return;
		}
	}
}

void Server::cleanup()
{
	NetworkBase::cleanup();

	if (serverSock_ != INVALID_SOCKET)
	{
		setNonBlocking(serverSock_, false);
		closesocket(serverSock_);
		serverSock_ = INVALID_SOCKET;
	}
}

Client::Client(const char* ip, unsigned short port)
{
	port_ = port;
	ip_ = ip;
}

Client::~Client()
{
	cleanup();
}

void Client::tick(NetworkEvent* event)
{
	event->eventCode = eNone;

	if (clientSock_ == INVALID_SOCKET)
	{
		// create socket
		clientSock_ = socket(PF_INET, SOCK_STREAM, 0);
		if (clientSock_ == INVALID_SOCKET)
		{
			cleanup();
			event->eventCode = eCreateSocketError;
			return;
		}

		setNonBlocking(clientSock_, true);

		connecting_ = true;
	}
		

	if (connecting_ == true)
	{
		// connect
		sockaddr_in ai_addr;
		ai_addr.sin_family = AF_INET;
		ai_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
		ai_addr.sin_port = htons(port_);

		int c = connect(clientSock_, (struct sockaddr*)&ai_addr, sizeof(ai_addr));

		if (c == 0 || getError() == EISCONN2)
		{
			connecting_ = false;
			event->eventCode = eOtherSideConnected;
			return;
		}
		else
		{
			if (getError() == EWOULDBLOCK2 || getError() == EALREADY2 || getError() == EINPROGRESS2 || getError() == EINVAL2)
			{
				// still trying to connect
				return;
			}
			else
			{
				cleanup();
				event->eventCode = eConnectError;
				return;
			}
		}
	}

	// send and recv
	if (clientSock_ != INVALID_SOCKET)
	{
		tickRecv(event);
		if (eFirstError < event->eventCode && event->eventCode < eLastError)
		{
			cleanup();
			return;
		}

		if (event->eventCode != eNone)
			return;

		tickSend(event);
		if (eFirstError < event->eventCode && event->eventCode < eLastError)
		{
			cleanup();
			return;
		}
	}
}

void Client::cleanup(void)
{
	NetworkBase::cleanup();

	connecting_ = false;
}

const char* eventCodeString(EventCode eventCode)
{
	switch (eventCode)
	{
	case eNone:
		return "eNone";

	case eFirstError:
		return "eFirstError";

	case eCreateSocketError:
		return "eCreateSocketError";

	case eSetReuseAddrError:
		return "eSetReuseAddrError";

	case eBindError:
		return "eBindError";

	case eListenError:
		return "eListenError";

	case eAcceptError:
		return "eAcceptError";

	case eOtherSideClosedConnection:
		return "eOtherSideClosedConnection";

	case eConnectError:
		return "eConnectError";

	case eLastError:
		return "eLastError";

	case eOtherSideConnected:
		return "eOtherSideConnected";

	case eDataReceived:
		return "eDataReceived";

	case eDataSent:
		return "eDataSent";
	}

	return "unknown eventCode";
}
