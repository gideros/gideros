/*
 * OggEncOpus.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggEncTheora.h"
#include <math.h>
#include <cstring>

OggEncTheora::OggEncTheora() {
    eof=false;
}

OggEncTheora::~OggEncTheora() {
    /* - Call th_encode_free() to release all encoder memory.*/
    if (te) {
        th_encode_free(te);
        te=NULL;
	}
}

bool OggEncTheora::PacketOut(ogg_packet *op)
{
    /*   - Repeatedly call th_encode_packetout() to retrieve any video data packets
    *      that are ready.*/
    return th_encode_packetout(te,eof,op)>0;
}

bool OggEncTheora::InitVideo(float rate,unsigned int width, unsigned int height, int format, float quality)
{
    G_UNUSED(quality);
    /* - Fill in a #th_info structure with details on the format of the video you
    *    wish to encode.*/
    th_info_init(&ti);
    ti.frame_height=(height+15)&(~15);
    ti.frame_width=(width+15)&(~15);
    ti.pic_height=height;
    ti.pic_width=width;
    ti.fps_numerator=rate;
    ti.fps_denominator=1;
    ti.pixel_fmt=(th_pixel_fmt)format;
    ti.quality=quality*63;

    /* - Allocate a #th_enc_ctx handle with th_encode_alloc().*/
    te=th_encode_alloc(&ti);
    if (!te) return false;
    /* - Perform any additional encoder configuration required with
    *    th_encode_ctl().*/

    th_comment_init(&tc);
    return true;
}

bool OggEncTheora::GenHeaderPage(ogg_stream_state *os) {
    /* Repeatedly call th_encode_flushheader() to retrieve all the header
        packets.*/
    ogg_packet header;
    if (th_encode_flushheader(te,&tc,&header)>0) {
        ogg_stream_packetin(os, &header);
        return true;
    }
    return false;
}

void OggEncTheora::EncodeVideoFrame(ogg_int64_t gpos,VideoFrame *frame,bool last) {
    G_UNUSED(gpos);
    /* - For each uncompressed frame:
    *   - Submit the uncompressed frame via th_encode_ycbcr_in()*/
    th_ycbcr_buffer yuv;
    yuv[0].data=(unsigned char *)frame->y.data;
    yuv[0].stride=frame->y.stride;
    yuv[0].width=frame->y.width;
    yuv[0].height=frame->y.height;

    yuv[1].data=(unsigned char *)frame->u.data;
    yuv[1].stride=frame->u.stride;
    yuv[1].width=frame->u.width;
    yuv[1].height=frame->u.height;

    yuv[2].data=(unsigned char *)frame->v.data;
    yuv[2].stride=frame->v.stride;
    yuv[2].width=frame->v.width;
    yuv[2].height=frame->v.height;

    th_encode_ycbcr_in(te,yuv);
    eof=last;
}
