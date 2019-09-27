/*
 * OggDecTheora.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggDecTheora.h"
#include "glog.h"

OggDecTheora::OggDecTheora() {
	hdr_count=0;
	th_comment_init(&tc);
	th_info_init(&ti);
    ts=NULL;
    pp_inc=0;
    pp_level=0;
    pp_level_max=0;
     td=NULL;
    video_granule=0;
}

OggDecTheora::~OggDecTheora() {
	if (ts)
		th_setup_free(ts);
	th_decode_free(td);
	th_comment_clear(&tc);
	th_info_clear(&ti);
}

double OggDecTheora::GranuleTime(ogg_int64_t gpos)
{
	return th_granule_time(td, gpos);
}

bool OggDecTheora::GotHeaders()
{
	return hdr_count>=3;
}

void OggDecTheora::GetVideoInfo(float &rate, unsigned int &width,unsigned int &height,int &format)
{
	width=ti.frame_width;
	height=ti.frame_height;
	rate=(double)ti.fps_numerator/ti.fps_denominator;
	format=ti.pixel_fmt;
}

bool OggDecTheora::PacketIn(ogg_packet *op)
{
	if (!GotHeaders()) {
        if (th_decode_headerin(&ti, &tc, &ts,op)<0) return false;
		hdr_count++;
		if (hdr_count==3) {
			td = th_decode_alloc(&ti, ts);
			if (ti.pic_width != ti.frame_width
					|| ti.pic_height != ti.frame_height)
				glog_v("  Frame content is %dx%d with offset (%d,%d).\n",
						ti.frame_width, ti.frame_height,
						ti.pic_x, ti.pic_y);
			th_decode_ctl(td, TH_DECCTL_GET_PPLEVEL_MAX,
					&pp_level_max, sizeof(pp_level_max));
			pp_level = pp_level_max;
			th_decode_ctl(td, TH_DECCTL_SET_PPLEVEL, &pp_level,
					sizeof(pp_level));
			pp_inc = 0;

			th_setup_free(ts);
			ts=NULL;
		}
		return true;
	}
	if (pp_inc) {
		pp_level += pp_inc;
		th_decode_ctl(td, TH_DECCTL_SET_PPLEVEL,
				&pp_level, sizeof(pp_level));
		pp_inc = 0;
	}
	/*HACK: This should be set after a seek or a gap, but we might not have
	 a granulepos for the first packet (we only have them for the last
	 packet on a page), so we just set it as often as we get it.
	 To do this right, we should back-track from the last packet on the
	 page and compute the correct granulepos for the first packet after
	 a seek or a gap.*/
	if (op->granulepos >= 0) {
		th_decode_ctl(td, TH_DECCTL_SET_GRANPOS,
				&op->granulepos,
				sizeof(op->granulepos));
	}
	video_granule=0;
	return th_decode_packetin(td, op,	&video_granule) == 0;

}

bool OggDecTheora::HasVideoFrame(ogg_int64_t &gpos)
{
	if (video_granule!=0)
	{
		gpos=video_granule;
		return true;
	}
	return false;
}

void OggDecTheora::GetVideoFrame(VideoFrame &frame)
{
	th_ycbcr_buffer yuv;
	th_decode_ycbcr_out(td, yuv);
	frame.y.height=yuv[0].height;
	frame.y.stride=yuv[0].stride;
	frame.y.data=yuv[0].data;
    frame.u.height=yuv[1].height;
    frame.u.stride=yuv[1].stride;
    frame.u.data=yuv[1].data;
    frame.v.height=yuv[2].height;
    frame.v.stride=yuv[2].stride;
    frame.v.data=yuv[2].data;
	video_granule=0;
}

static OggDec *probe_theora(ogg_packet *op) {
	OggDec *d=new OggDecTheora();
	if (d->PacketIn(op))
		return d;
	delete d;
	return NULL;
}
extern const OggDecType theora_cinfo= {
		CODEC_TYPE_VIDEO,
		probe_theora
};

#ifdef PART_Theora
#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

static void g_initializePlugin(lua_State *L) {
    register_oggdec("theora",theora_cinfo);
}

static void g_deinitializePlugin(lua_State *L) {
	unregister_oggdec("theora");
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("OggTheora", "1.0",OggTheora)
#else
REGISTER_PLUGIN_NAMED("OggTheora", "1.0", OggTheora)
#endif
#endif

