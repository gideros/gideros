/*
 * OggDecVorbis.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGDECVORBIS_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGDECVORBIS_H_
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>

#include "OggDec.h"
class OggDecVorbis : public OggDec {
	//VORBIS
	vorbis_info vi;
	vorbis_dsp_state vd;
	vorbis_block vb;
	vorbis_comment vc;
	int hdr_count;
public:
	OggDecVorbis();
	~OggDecVorbis();
	virtual double GranuleTime(ogg_int64_t gpos);
	virtual bool GotHeaders();
	virtual bool PacketIn(ogg_packet *op);
        virtual void GetAudioInfo(unsigned int &rate, unsigned int &channels);
	virtual void Restart();
	virtual int GetAudio(ogg_int16_t *buffer,int max,ogg_int64_t &gpos);
};

extern const OggDecType vorbis_cinfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDECVORBIS_H_ */
