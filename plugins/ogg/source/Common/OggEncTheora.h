/*
 * OggDecTheora.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGENCTHEORA_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGENCTHEORA_H_
#include <theora/theora.h>
#include <theora/theoraenc.h>

#include "OggEnc.h"
class OggEncTheora : public OggEnc {
    //THEORA
	th_info ti;
	th_comment tc;
	th_enc_ctx *te;
	bool eof;
public:
	OggEncTheora();
	~OggEncTheora();
    virtual bool PacketOut(ogg_packet *op);
	virtual bool GenHeaderPage(ogg_stream_state *os);
    virtual bool InitVideo(float rate,unsigned int width, unsigned int height, int format, float quality);
    virtual void EncodeVideoFrame(ogg_int64_t gpos,VideoFrame *frame,bool last);
};

extern const OggEncType theora_einfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDECTHEORA_H_ */
