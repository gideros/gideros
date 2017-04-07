#ifndef NETPLAYER_H_
#define NETPLAYER_H_

#include <emscripten.h>
#include <libnetwork.h>


extern "C" void serverStop();
extern "C" void serverStart();
extern "C" void serverSendData(const char *data,unsigned int size);
extern "C" int serverDataSent();
extern "C" int serverDataReceived();
extern "C" void serverTick(NetworkEvent *event);

#endif
