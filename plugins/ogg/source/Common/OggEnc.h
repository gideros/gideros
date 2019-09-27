/*
 * OggDec.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGENC_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGENC_H_
#include <ogg/ogg.h>
#include <stdlib.h>

class OggEnc {
public:
        virtual ~OggEnc() {};
        virtual bool PacketOut(ogg_packet *op)=0;
	virtual bool InitAudio(unsigned int channels, unsigned int rate, float quality, ogg_stream_state *os) { return false; };
	virtual void WriteAudio(void *buffer,size_t size) { };
};

struct OggEncType {
	OggEnc *(*build)();
};


extern "C" {
void register_oggenc(const char *name,OggEncType codec);
void unregister_oggenc(const char *name);
OggEnc *build_oggenc(const char *name);
}
#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDEC_H_ */
