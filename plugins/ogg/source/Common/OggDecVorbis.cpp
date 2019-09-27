/*
 * OggDecVorbis.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggDecVorbis.h"
#include "OggEncVorbis.h"
#include <math.h>

OggDecVorbis::OggDecVorbis() {
	hdr_count=0;
	/* init supporting Vorbis structures needed in header parsing */
	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);
}

OggDecVorbis::~OggDecVorbis() {
	if (hdr_count>=3) {
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
	}
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
}

double OggDecVorbis::GranuleTime(ogg_int64_t gpos)
{
 return vorbis_granule_time(&vd, gpos);
}

bool OggDecVorbis::GotHeaders()
{
	return hdr_count>=3;
}

bool OggDecVorbis::PacketIn(ogg_packet *op)
{
	if (!GotHeaders()) {
		if (vorbis_synthesis_headerin(&vi, &vc,op)) return false;
		hdr_count++;
		if (hdr_count==3) {
			vorbis_synthesis_init(&vd, &vi);
			vorbis_block_init(&vd, &vb);
		}
		return true;
	}
	if (vorbis_synthesis(&vb, op) == 0) /* test for success! */
	{
		vorbis_synthesis_blockin(&vd, &vb);
		return true;
	}
	return false;
}

void OggDecVorbis::GetAudioInfo(unsigned int &rate, unsigned int &channels)
{
	rate=vi.rate;
	channels=vi.channels;
}

void OggDecVorbis::Restart()
{
	vorbis_synthesis_restart(&vd);
}

int OggDecVorbis::GetAudio(ogg_int16_t *buffer,int max,ogg_int64_t &gpos)
{
	float **pcm;
	int ret=vorbis_synthesis_pcmout(&vd, &pcm);
	int count=0;
	size_t i=0;
	max/=vi.channels;
	for (i = 0; i < ret && i < max; i++)
		for (size_t j = 0; j < vi.channels; j++) {
			int val = rint(pcm[j][i] * 32767.f);
			if (val > 32767)
				val = 32767;
			if (val < -32768)
				val = -32768;
			buffer[count++] = val;
		}
	vorbis_synthesis_read(&vd, i);

	return i*2;
}


static OggDec *probe_vorbis(ogg_packet *op) {
	OggDec *d=new OggDecVorbis();
	if (d->PacketIn(op))
		return d;
	delete d;
	return NULL;
}
extern const OggDecType vorbis_cinfo= {
		CODEC_TYPE_AUDIO,
		probe_vorbis
};

#ifndef FLAVOUR
#define FLAVOUR_F
#endif

#if defined(FLAVOUR_F)
static OggEnc *build_vorbis() {
	return new OggEncVorbis();
}
extern const OggEncType vorbis_einfo= {
		build_vorbis
};
#endif

#ifdef PART_Vorbis
#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

static void g_initializePlugin(lua_State *L) {
    register_oggdec("vorbis",vorbis_cinfo);
#if defined(FLAVOUR_F)
    register_oggenc("vorbis",vorbis_einfo);
#endif
}

static void g_deinitializePlugin(lua_State *L) {
	unregister_oggdec("vorbis");
#if defined(FLAVOUR_F)
	unregister_oggenc("vorbis");
#endif
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("OggVorbis", "1.0",OggVorbis)
#else
REGISTER_PLUGIN_NAMED("OggVorbis", "1.0", OggVorbis)
#endif
#endif
