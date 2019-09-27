/*
 * OggDecTheora.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGDECTHEORA_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGDECTHEORA_H_
#include <theora/theora.h>
#include <theora/theoradec.h>

#include "OggDec.h"
class OggDecTheora : public OggDec {
    //THEORA
	th_info ti;
	th_comment tc;
	th_dec_ctx *td;
	th_setup_info *ts;
	int hdr_count;
	int pp_level_max;
	int pp_level;
	int pp_inc;
	ogg_int64_t video_granule;
public:
	OggDecTheora();
	~OggDecTheora();
	virtual double GranuleTime(ogg_int64_t gpos);
	virtual bool GotHeaders();
	virtual void GetVideoInfo(float &rate, unsigned int &width,unsigned int &height,int &format);
	virtual bool PacketIn(ogg_packet *op);
	virtual bool HasVideoFrame(ogg_int64_t &gpos);
	virtual void GetVideoFrame(VideoFrame &frame);
};

extern const OggDecType theora_cinfo;

#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDECTHEORA_H_ */
