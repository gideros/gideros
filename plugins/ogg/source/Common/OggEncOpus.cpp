/*
 * OggEncOpus.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggEncOpus.h"
#include <math.h>
#include <cstring>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

PACK(union unal
{
 uint8_t b;
 uint16_t s;
 uint32_t l;
 int8_t bi;
 int16_t si;
 int32_t li;
 uint64_t ll;
 int64_t lli;
 float f;
 double d;
});

#define PULONG(p)  ((union unal *)(p))->l
#define PUSHORT(p) ((union unal *)(p))->s
#define PLONG(p)   ((union unal *)(p))->li
#define PSHORT(p)  ((union unal *)(p))->si
#define PUCHAR(p)  ((union unal *)(p))->b
#define PCHAR(p)   ((union unal *)(p))->bi
#define PULONGLONG(p) ((union unal *)(p))->ll
#define PLONGLONG(p) ((union unal *)(p))->lli

#define htoles(a)		a
#define	htolel(a)		a

OggEncOpus::OggEncOpus() {
	buf0=NULL;
	buf1=NULL;
    buf0sz=0;
    buf1sz=0;
    buf0done=0;
	enc=NULL;
	eof=false;
}

OggEncOpus::~OggEncOpus() {
	if (enc) {
		free(enc);
		free(smpbuf);
	}
	if (buf0) free(buf0);
	if (buf1) free(buf1);
}

bool OggEncOpus::PacketOut(ogg_packet *op)
{
	while (true) {
		if (buf0) {
			int ck=smpbufsz-smpbufc;
			int mb=buf0sz-buf0done;
			if (mb>ck) mb=ck;
			memcpy(((char *)smpbuf)+smpbufc, buf0+buf0done,mb);
			smpbufc+=mb;
			buf0done+=mb;
			if (buf0done==buf0sz) {
				free(buf0);
				buf0=buf1;
				buf0sz=buf1sz;
				buf0done=0;
				buf1=NULL;
                buf1sz=0;
				continue;
			}
		}
        if (smpbufc<smpbufsz) {
            if (eof) {
                op->bytes=0;
                op->packet=pkt;
                op->e_o_s=1;
                op->b_o_s=0;
                op->packetno=pnum++;
                op->granulepos=gpos;
                eof=false;
                return true;
            }
            return false;
        }
		smpbufc=0;
		gpos+=48*fsz;
		int es=opus_encode(enc,smpbuf,fsk*fsz,pkt,1276);
		if (es>1)
		{
			op->bytes=es;
			op->packet=pkt;
            op->e_o_s=eof&&((buf0sz+buf1sz-buf0done)>smpbufsz)?1:0;
            if (op->e_o_s) eof=false;
			op->b_o_s=0;
			op->packetno=pnum++;
			op->granulepos=gpos;
			return true;
		}
	}
}

bool OggEncOpus::InitAudio(unsigned int channels, unsigned int rate, float quality)
{
	int es=opus_encoder_get_size(channels);
	enc=(OpusEncoder *) malloc(es);
	fsk=rate/1000;
	if (fsk==0) fsk=24; //Default
	else if (fsk<=8) fsk=8;
	else if (fsk<=12) fsk=12;
	else if (fsk<=16) fsk=16;
	else if (fsk<=24) fsk=24;
	else fsk=48;
	fsz=20;
	gpos=0;
	opus_encoder_init(enc,fsk*1000,channels,OPUS_APPLICATION_AUDIO);
	smpbufsz=fsk*fsz*2*channels;
	smpbufc=0;
	smpbuf=(opus_int16 *)malloc(smpbufsz);    
    pnum=0;
    nChannels=channels;

	return true;
}

bool OggEncOpus::GenHeaderPage(ogg_stream_state *os) {
    ogg_packet header;
    opus_int32 delay_samples;

    switch (pnum) {
    case 0:
        opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&delay_samples));
        header.b_o_s=1;
        header.e_o_s=0;
        header.granulepos=0;
        header.packetno=pnum++;
        header.bytes=19; //Header size
        header.packet=pkt;
        memcpy(pkt+0,"OpusHead",8);
        pkt[8]=1; //Version
        pkt[9]=nChannels; //Channels
        PUSHORT(pkt+10)=htoles(delay_samples); //Pre-Skip
        PULONG(pkt+12)=htolel(fsk*1000); //SAMPLE-RATE
        PUSHORT(pkt+16)=htoles(0); //Gain
        pkt[18]=0; //Mode
        ogg_stream_packetin(os, &header);
        return true;
    case 1:
        header.bytes=16+7; //Header size
        header.packet=pkt;
        header.b_o_s=0;
        header.e_o_s=0;
        header.granulepos=0;
        header.packetno=pnum++;
        memcpy(pkt+0,"OpusTags",8);
        PULONG(pkt+8)=htolel(7); //VENDOR
        memcpy(pkt+12,"Gideros",7);
        PULONG(pkt+19)=htolel(0); //COMMENTS
        ogg_stream_packetin(os, &header);
        return true;
    default:
        return false;
    }
}

void OggEncOpus::WriteAudio(void *buffer,size_t size) {
	if (size == 0) {
		eof=true;
	} else {
		if (!buf0) {
			buf0=(char *)malloc(size);
			memcpy(buf0,buffer,size);
			buf0done=0;
			buf0sz=size;
		}
		else if (!buf1) {
			buf1=(char *)malloc(size);
			memcpy(buf1,buffer,size);
			buf1sz=size;
		}
	}
}
