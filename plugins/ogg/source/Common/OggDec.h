/*
 * OggDec.h
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#ifndef PLUGINS_OGG_SOURCE_COMMON_OGGDEC_H_
#define PLUGINS_OGG_SOURCE_COMMON_OGGDEC_H_
#include <ogg/ogg.h>
#include <stdlib.h>
#include "gplugin.h"

class OggDec {
public:
	struct VideoBuffer {
		int height;
		int stride;
		void *data;
	};
	struct VideoFrame {
		VideoBuffer y;
		VideoBuffer u;
		VideoBuffer v;
	};
	virtual ~OggDec() { };
	virtual double GranuleTime(ogg_int64_t gpos)=0;
	virtual bool GotHeaders()=0;
	virtual bool PacketIn(ogg_packet *op)=0;
	virtual void GetAudioInfo(unsigned int &rate, unsigned int &channels) { G_UNUSED(rate); G_UNUSED(channels); };
	virtual void GetVideoInfo(float &rate, unsigned int &width,unsigned int &height,int &format)
	{
		G_UNUSED(rate);
		G_UNUSED(width);
		G_UNUSED(height);
		G_UNUSED(format);
	};
	virtual void Restart() { };
	virtual int GetAudio(ogg_int16_t *buffer,int max,ogg_int64_t &gpos)
	{
		G_UNUSED(buffer);
		G_UNUSED(max);
		G_UNUSED(gpos);
		return 0;
	};
	virtual bool HasVideoFrame(ogg_int64_t &gpos) { G_UNUSED(gpos); return false; };
	virtual void GetVideoFrame(VideoFrame &frame) { G_UNUSED(frame); };
};

#define CODEC_TYPE_AUDIO	1
#define CODEC_TYPE_VIDEO	2
struct OggDecType {
	int type; //1=Audio, 2=Video
	OggDec *(*probe)(ogg_packet *op);
};

extern "C" {
G_DLLEXPORT void register_oggdec(const char *name,OggDecType codec);
G_DLLEXPORT void unregister_oggdec(const char *name);
OggDec *probe_oggdec(ogg_packet *op,int type);
}
#endif /* PLUGINS_OGG_SOURCE_COMMON_OGGDEC_H_ */
