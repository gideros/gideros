/*
 * OggDecVorbis.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGENCVORBIS_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGENCVORBIS_H_
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>

#include "OggEnc.h"
class OggEncVorbis : public OggEnc {
	//VORBIS
	vorbis_info vi;
	vorbis_dsp_state vd;
	vorbis_block vb;
	vorbis_comment vc;
	bool encodeInited;
	bool stereo;
public:
	OggEncVorbis();
	~OggEncVorbis();
	virtual bool PacketOut(ogg_packet *op);
    virtual bool InitAudio(unsigned int channels, unsigned int rate, float quality);
    virtual bool GenHeaderPage( ogg_stream_state *os);
    virtual void WriteAudio(void *buffer,size_t size);
};

extern const OggEncType vorbis_einfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDECVORBIS_H_ */
