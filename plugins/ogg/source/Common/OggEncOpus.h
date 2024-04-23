/*
 * OggDecVorbis.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGENCOPUS_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGENCOPUS_H_
#include <opus.h>

#include "OggEnc.h"
class OggEncOpus : public OggEnc {
	//BUFFERS
	char *buf0;
	char *buf1;
	size_t buf0sz;
	size_t buf1sz;
	size_t buf0done;
	bool eof;
	int pnum;
    int nChannels;
	//OPUS
	OpusEncoder *enc;
	int fsk;
	int fsz;
	opus_int16 *smpbuf;
    size_t smpbufsz;
    size_t smpbufc;
	unsigned char pkt[1276];
	ogg_int64_t gpos;

public:
	OggEncOpus();
	~OggEncOpus();
	virtual bool PacketOut(ogg_packet *op);
    virtual bool InitAudio(unsigned int channels, unsigned int rate, float quality);
    virtual bool GenHeaderPage(ogg_stream_state *os);
    virtual void WriteAudio(void *buffer,size_t size);
};

extern const OggEncType opus_einfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGENCOPUS_H_ */
