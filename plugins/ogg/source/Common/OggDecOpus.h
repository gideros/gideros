/*
 * OggDecOpus.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGDECOPUS_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGDECOPUS_H_
#include <opus.h>

#include "OggDec.h"
#define OPUS_MAX_SAMPLES	(48*120*2) //48kHZ,120ms,stereo
class OggDecOpus : public OggDec {
	int hdr_count;
	OpusDecoder *dec;
	int opusFs,opusChannels;
	opus_int16 pcm[OPUS_MAX_SAMPLES];
	int pcm_count,pcm_offset;
	ogg_int64_t granulepos;
public:
	OggDecOpus();
	~OggDecOpus();
	virtual double GranuleTime(ogg_int64_t gpos);
	virtual bool GotHeaders();
	virtual bool PacketIn(ogg_packet *op);
        virtual void GetAudioInfo(unsigned int &rate, unsigned int &channels);
	virtual void Restart();
	virtual int GetAudio(ogg_int16_t *buffer,int max,ogg_int64_t &gpos);
};
extern const OggDecType opus_cinfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDECVORBIS_H_ */
