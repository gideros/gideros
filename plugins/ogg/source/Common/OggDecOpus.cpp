/*
 * OggDecOpus.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggDecOpus.h"
//#include "OggEncOpus.h"
#include <math.h>
#include <string.h>

OggDecOpus::OggDecOpus() {
	hdr_count=0;
	opusFs=48000;
	opusChannels=0;
	dec=NULL;
	pcm_count=pcm_offset=0;
	granulepos=0;
}

OggDecOpus::~OggDecOpus() {
	if (dec)
		opus_decoder_destroy(dec);
}

double OggDecOpus::GranuleTime(ogg_int64_t gpos)
{
	//in opus, granule time is the number of samples at 48kHz
	return (1.0/48000)*gpos;
}

bool OggDecOpus::GotHeaders()
{
	return hdr_count>=2;
}

bool OggDecOpus::PacketIn(ogg_packet *op)
{
	if (!GotHeaders()) {
		switch (hdr_count) {
		case 0:
			if (!op->b_o_s) return false;
			if (op->bytes<18) return false;
			if (memcmp(op->packet,"OpusHead",8)) return false;
			if (op->packet[8]!=1) return false;
			opusChannels=op->packet[9];
			break;
		case 1:
			if (op->bytes<8) return false;
			if (memcmp(op->packet,"OpusTags",8)) return false;
			break;
		}
		hdr_count++;
		if (hdr_count==2) {
			int error;
			dec=opus_decoder_create(opusFs,opusChannels,&error);
		}
		return true;
	}
    pcm_count=opus_decode(dec,op->packet,op->bytes,pcm,OPUS_MAX_SAMPLES/opusChannels,0);
	pcm_offset=0;
	granulepos=op->granulepos;
    if (pcm_count<0) {
        pcm_count=0;
        return false;
    }
    pcm_count*=opusChannels;
    if (granulepos>=0) {
        int ginc=(pcm_count/opusChannels);
        if (opusFs!=48000) {
            ginc*=48000/opusFs;
        }
        granulepos-=pcm_count;
    }
	return true;
}

void OggDecOpus::GetAudioInfo(unsigned int &rate, unsigned int &channels)
{
	rate=opusFs;
	channels=opusChannels;
}

void OggDecOpus::Restart()
{
	if (dec) {
		opus_decoder_init(dec,opusFs,opusChannels);
	}
}

int OggDecOpus::GetAudio(ogg_int16_t *buffer,int max,ogg_int64_t &gpos)
{
	if (max>(pcm_count-pcm_offset))
		max=pcm_count-pcm_offset;
    memcpy(buffer,pcm+pcm_offset,max*2);
	pcm_offset+=max;
    if (granulepos>=0)  {
        int ginc=(max/opusChannels);
        if (opusFs!=48000) {
            ginc*=48000/opusFs;
        }
        granulepos+=ginc;
    }
	gpos=granulepos;
	return max;
}


static OggDec *probe_opus(ogg_packet *op) {
	OggDec *d=new OggDecOpus();
	if (d->PacketIn(op))
		return d;
	delete d;
	return NULL;
}
extern const OggDecType opus_cinfo= {
		CODEC_TYPE_AUDIO,
		probe_opus
};
/*
static OggEnc *build_opus() {
	return new OggEncOpus();
}
extern const OggEncType einfo= {
		build: build_opus
};*/

#ifdef PART_Opus
#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

static void g_initializePlugin(lua_State *L) {
	register_oggdec("opus",opus_cinfo);
	//register_oggenc("opus",opus_einfo);
}

static void g_deinitializePlugin(lua_State *L) {
    unregister_oggdec("opus");
	//unregister_oggenc("opus");
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("OggOpus", "1.0",OggOpus)
#else
REGISTER_PLUGIN_NAMED("OggOpus", "1.0", OggOpus)
#endif
#endif
