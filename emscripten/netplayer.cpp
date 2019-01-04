#include "netplayer.h"
#include <time.h>
#include <stdlib.h>

static int dataSent=0,dataReceived=0;
static bool srvOn=false;

static void wsSend(const char *data,int size)
{
	EM_ASM_({
		Module['gnetplayerSend'](Module.HEAPU8.subarray($0,$0+$1));
	},data,size);
}

extern "C" void serverStop()
{
	srvOn=false;
	EM_ASM({
		if (GiderosNetplayerWS)
			GiderosNetplayerWS.close();
	});
}

extern "C" void serverStart()
{
	dataSent=0;
	dataReceived=0;
	srvOn=true;
}

static int nextId=0;
extern "C" void serverSendData(const char *data,unsigned int size)
{
    char *p=(char *)malloc(sizeof(unsigned int)*2+size);
    ((unsigned int *)p)[0]=++nextId;
    ((unsigned int *)p)[1]=0;
    memcpy(p+sizeof(unsigned int)*2,data,size);
    wsSend(p,sizeof(unsigned int)*2+size);
    free(p);
}

extern "C" void serverSendAck(unsigned int id)
{

    char *p=(char *)malloc(sizeof(unsigned int)*3);
    ((unsigned int *)p)[0]=++nextId;
    ((unsigned int *)p)[1]=1;
    ((unsigned int *)p)[1]=id;
    wsSend(p,sizeof(unsigned int)*3);
    free(p);
}

extern "C" int serverDataSent()
{
	return dataSent;
}

extern "C" int serverDataReceived()
{
	return dataReceived;
}


static time_t lastBcastTime_;
#define ADVERTISE_PERIOD	1	//Advertise every 1 seconds
extern "C" void serverTick(NetworkEvent *event)
{
	if (srvOn)
	EM_ASM({
        try {
        	if (typeof GiderosNetplayerWSHost === 'undefined') {
        		GiderosNetplayerWSHost="127.0.0.1";
        	}
            if ((GiderosNetplayerWS==null)&&(GiderosNetplayerWSHost!=null))
            {
            if (typeof MozWebSocket == 'function')
                WebSocket = MozWebSocket;
            GiderosNetplayerWSQ=[];
            GiderosNetplayerWS = new WebSocket( "ws://"+GiderosNetplayerWSHost+":15001" );
            GiderosNetplayerWS.binaryType = 'arraybuffer';
            GiderosNetplayerWS.onmessage = function (evt) {
            	GiderosNetplayerWSQ.push(evt.data);
            };
            GiderosNetplayerWS.onclose = function (evt) {
            	GiderosNetplayerWS = null;
            };
            GiderosNetplayerWS.onerror = function (evt) {
            	GiderosNetplayerWS = null;
            };
            }
        } catch (exception) {
        	GiderosNetplayerWS = null;
        }
	});

	event->eventCode = eNone;
    time_t ctime=time(NULL);
    if ((lastBcastTime_<=(ctime-ADVERTISE_PERIOD))||
    				(lastBcastTime_>ctime))
    {
        lastBcastTime_=ctime;
        struct bcast {
        	int id;
        	int type;
        	unsigned char rsv;
        	unsigned char flagshi,flagslo;
        	char name[64];
        } __attribute__((packed)) p;
        p.id=0;
        p.type=2;
        p.rsv=0;
        p.flagshi=0;
        p.flagslo=0;
        strcpy(p.name,"Html5");
        wsSend((const char *)&p,sizeof(struct bcast));
    }
    //RX
    char *b=(char *) EM_ASM_INT_V(
	{
    	var buffer=0;
    	if (GiderosNetplayerWS!=null)
    	{
    		var data=GiderosNetplayerWSQ.shift();
    		if (data) {
    			var byteArray = new Uint8Array(data);
    			buffer = _malloc(byteArray.length);
    			HEAPU8.set(byteArray, buffer);
    			GiderosNetplayerWSR=byteArray.length;
    		}
    	}
    	return buffer;
    });
   if (b)
   {
	   int sz=EM_ASM_INT_V({ return GiderosNetplayerWSR;});
	   unsigned int id=((unsigned int *)b)[0];
	   unsigned int type=((unsigned int *)b)[1];

		if (type == 0)
		{
			event->eventCode = eDataReceived;

			event->data.resize(sz - sizeof(unsigned int)*2);
			memcpy(&event->data[0], b+sizeof(unsigned int)*2,sz-sizeof(unsigned int)*2);

			serverSendAck(id);
		}
		else if (type == 1)
		{
			event->eventCode = eDataSent;
			event->id = *(unsigned int*)(b+sizeof(unsigned int)*2);
		}

	   free(b);
   }
}
