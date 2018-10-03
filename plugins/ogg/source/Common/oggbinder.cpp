#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

#include <gaudio.h>
#include <ggaudiomanager.h>
#include <gstdio.h>
#include <Shaders.h>
#include <texturebase.h>

#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
#include <theora/theora.h>
#include <theora/theoradec.h>
#include <math.h>
#include <platformutil.h>
#include "screen.h"
#include "ticker.h"
#include "luaapplication.h"

static lua_State *L = NULL;
struct GGOggHandle {
	//OGG GENERIC
	G_FILE *file;
	ogg_packet op;
	ogg_sync_state oy;
	ogg_page og;
	ogg_stream_state vo;
	ogg_stream_state to;
	th_info ti;
	th_comment tc;
	th_dec_ctx *td;
	th_setup_info *ts;
	vorbis_info vi;
	vorbis_dsp_state vd;
	vorbis_block vb;
	vorbis_comment vc;
	th_pixel_fmt px_fmt;
	int pp_level_max;
	int pp_level;
	int pp_inc;
	ogg_int64_t audio_granulepos;
	ogg_int64_t video_granulepos;

	TextureBase *yplane;
	TextureBase *uplane;
	TextureBase *vplane;
	int planeWidth, planeHeight;

	int theora_p;
	int vorbis_p;
	int stateflag;
	int videobuf_ready;
	double videobuf_time;
	double audio_time;
	double playstart;
	int sampleRate;
	int sampleSize;
	int sampleMax;

	//LUA
	int tref;
};

struct GGOggEncHandle {
	//OGG GENERIC
	FILE *fos;
	int bytesPerSample;
	ogg_stream_state os; /* take physical pages, weld into a logical
	 stream of packets */
	ogg_page og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet op; /* one raw packet of data for decode */

	vorbis_info vi; /* struct that stores all the static vorbis bitstream
	 settings */
	vorbis_comment vc; /* struct that stores all the user comments */

	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block vb; /* local working space for packet->PCM decode */
};

static std::map<g_id, GGOggHandle *> ctxmap;

int buffer_data(G_FILE *in, ogg_sync_state *oy) {
	char *buffer = ogg_sync_buffer(oy, 4096);
	int bytes = g_fread(buffer, 1, 4096, in);
	ogg_sync_wrote(oy, bytes);
	return (bytes);
}

static int queue_page(GGOggHandle *h, ogg_page *page) {
	if (h->theora_p)
		ogg_stream_pagein(&h->to, page);
	if (h->vorbis_p)
		ogg_stream_pagein(&h->vo, page);
	return 0;
}

extern "C" {

static int _fseek64_wrap(G_FILE *f, ogg_int64_t off, int whence) {
	if (f == NULL)
		return (-1);
	return g_fseek(f, off, whence);
}

static int sampleTell(GGOggHandle *handle,ogg_int64_t *agr,double *atm,ogg_int64_t *vgr,double *vtm) {
	G_FILE *file = handle->file;
	long cpos = g_ftell(file);
	long rewind=1<<17;
	if (cpos<rewind) rewind=cpos;
	g_fseek(file, cpos-rewind, SEEK_SET); //More than max ogg page size
	unsigned char *buff = (unsigned char *) malloc(rewind);
	size_t buffs = g_fread(buff, 1, rewind, file);
	unsigned char *bufe = buff + buffs - 27; //At least one ogg header
	unsigned char *bufr = buff;
	double gp = -1;
	while (bufr <= bufe) {
		if ((bufr[0] != 'O') || (bufr[1] != 'g') || (bufr[2] != 'g')
				|| (bufr[3] != 'S') || (bufr[4] != 0)) {
			bufr++;
			continue;
		}
		ogg_int64_t granulepos = bufr[13] & (0xff);
		granulepos = (granulepos << 8) | (bufr[12] & 0xff);
		granulepos = (granulepos << 8) | (bufr[11] & 0xff);
		granulepos = (granulepos << 8) | (bufr[10] & 0xff);
		granulepos = (granulepos << 8) | (bufr[9] & 0xff);
		granulepos = (granulepos << 8) | (bufr[8] & 0xff);
		granulepos = (granulepos << 8) | (bufr[7] & 0xff);
		granulepos = (granulepos << 8) | (bufr[6] & 0xff);
		int psn = (bufr[14] | (bufr[15] << 8) | (bufr[16] << 16)
				| (bufr[17] << 24));
		double gt = -1;
		if (handle->vorbis_p && (psn == handle->vo.serialno))
		{
			gt = vorbis_granule_time(&handle->vd, granulepos);
			if (agr) *agr=granulepos;
			if (atm) *atm=gt;
		}
		if (handle->theora_p && (psn == handle->to.serialno))
		{
			gt = th_granule_time(&handle->td, granulepos);
			if (vgr) *vgr=granulepos;
			if (vtm) *vtm=gt;
		}
		if (gt > gp)
			gp = gt;
		bufr += 27;
	}
	free(buff);
	g_fseek(file, cpos, SEEK_SET);
	return (gp > 0) ? (long int) (gp * handle->sampleRate) : -1;
}

void gaudio_OggClose(g_id gid);

g_id gaudio_OggOpen(const char *fileName, int *numChannels, int *sampleRate,
		int *bitsPerSample, int *numSamples, gaudio_Error *error) {
	G_FILE *file = g_fopen(fileName, "rb");
	if (file == NULL) {
		if (error)
			*error = GAUDIO_CANNOT_OPEN_FILE;
		return 0;
	}

	GGOggHandle *handle = new GGOggHandle();

	/* start up Ogg stream synchronization layer */
	ogg_sync_init(&handle->oy);

	/* init supporting Vorbis structures needed in header parsing */
	vorbis_info_init(&handle->vi);
	vorbis_comment_init(&handle->vc);

	/* init supporting Theora structures needed in header parsing */
	th_comment_init(&handle->tc);
	th_info_init(&handle->ti);

	g_id gid = (g_id) handle;
	ctxmap[gid] = handle;

	/* Ogg file open; parse the headers */
	/* Only interested in Vorbis/Theora streams */
	handle->stateflag = 0; /* playback has not begun */
	while (!handle->stateflag) {
		int ret = buffer_data(file, &handle->oy);
		if (ret == 0)
			break;
		while (ogg_sync_pageout(&handle->oy, &handle->og) > 0) {
			ogg_stream_state test;

			/* is this a mandated initial header? If not, stop parsing */
			if (!ogg_page_bos(&handle->og)) {
				/* don't leak the page; get it into the appropriate stream */
				queue_page(handle, &handle->og);
				handle->stateflag = 1;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&handle->og));
			ogg_stream_pagein(&test, &handle->og);
			ogg_stream_packetout(&test, &handle->op);

			/* identify the codec: try theora */
			if (!handle->theora_p
					&& th_decode_headerin(&handle->ti, &handle->tc, &handle->ts,
							&handle->op) >= 0) {
				/* it is theora */
				memcpy(&handle->to, &test, sizeof(test));
				handle->theora_p = 1;
			} else if (!handle->vorbis_p
					&& vorbis_synthesis_headerin(&handle->vi, &handle->vc,
							&handle->op) >= 0) {
				/* it is vorbis */
				memcpy(&handle->vo, &test, sizeof(test));
				handle->vorbis_p = 1;
			} else {
				/* whatever it is, we don't care about it */
				ogg_stream_clear(&test);
			}
		}
		/* fall through to non-bos page parsing */
	}

	/* we're expecting more header packets. */
	while ((handle->theora_p && handle->theora_p < 3)
			|| (handle->vorbis_p && handle->vorbis_p < 3)) {
		int ret;

		/* look for further theora headers */
		while (handle->theora_p && (handle->theora_p < 3) && (ret =
				ogg_stream_packetout(&handle->to, &handle->op))) {
			if (ret < 0) {
				fprintf(stderr, "Error parsing Theora stream headers; "
						"corrupt stream?\n");
				exit(1);
			}
			if (!th_decode_headerin(&handle->ti, &handle->tc, &handle->ts,
					&handle->op)) {
				fprintf(stderr, "Error parsing Theora stream headers; "
						"corrupt stream?\n");
				exit(1);
			}
			handle->theora_p++;
		}

		/* look for more vorbis header packets */
		while (handle->vorbis_p && (handle->vorbis_p < 3) && (ret =
				ogg_stream_packetout(&handle->vo, &handle->op))) {
			if (ret < 0) {
				fprintf(stderr,
						"Error parsing Vorbis stream headers; corrupt stream?\n");
				exit(1);
			}
			if (vorbis_synthesis_headerin(&handle->vi, &handle->vc,
					&handle->op)) {
				fprintf(stderr,
						"Error parsing Vorbis stream headers; corrupt stream?\n");
				exit(1);
			}
			handle->vorbis_p++;
			if (handle->vorbis_p == 3)
				break;
		}

		/* The header pages/packets will arrive before anything else we
		 care about, or the stream is not obeying spec */

		if (ogg_sync_pageout(&handle->oy, &handle->og) > 0) {
			queue_page(handle, &handle->og); /* demux into the appropriate stream */
		} else {
			int ret = buffer_data(file, &handle->oy); /* someone needs more data */
			if (ret == 0) {
				fprintf(stderr,
						"End of file while searching for codec headers.\n");
				gaudio_OggClose(gid);
				if (error)
					*error = GAUDIO_UNRECOGNIZED_FORMAT;
				return 0;
			}
		}
	}

	/* and now we have it all.  initialize decoders */
	if (handle->theora_p) {
		handle->td = th_decode_alloc(&handle->ti, handle->ts);
		printf("Ogg logical stream %lx is Theora %dx%d %.02f fps",
				handle->to.serialno, handle->ti.pic_width,
				handle->ti.pic_height,
				(double) handle->ti.fps_numerator / handle->ti.fps_denominator);
		handle->px_fmt = handle->ti.pixel_fmt;
		switch (handle->ti.pixel_fmt) {
		case TH_PF_420:
			printf(" 4:2:0 video\n");
			break;
		case TH_PF_422:
			printf(" 4:2:2 video\n");
			break;
		case TH_PF_444:
			printf(" 4:4:4 video\n");
			break;
		case TH_PF_RSVD:
		default:
			printf(" video\n  (UNKNOWN Chroma sampling!)\n");
			break;
		}
		if (handle->ti.pic_width != handle->ti.frame_width
				|| handle->ti.pic_height != handle->ti.frame_height)
			printf("  Frame content is %dx%d with offset (%d,%d).\n",
					handle->ti.frame_width, handle->ti.frame_height,
					handle->ti.pic_x, handle->ti.pic_y);
		th_decode_ctl(handle->td, TH_DECCTL_GET_PPLEVEL_MAX,
				&handle->pp_level_max, sizeof(handle->pp_level_max));
		handle->pp_level = handle->pp_level_max;
		th_decode_ctl(handle->td, TH_DECCTL_SET_PPLEVEL, &handle->pp_level,
				sizeof(handle->pp_level));
		handle->pp_inc = 0;

		/*{
		 int arg = 0xffff;
		 th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_MBMODE,&arg,sizeof(arg));
		 th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_MV,&arg,sizeof(arg));
		 th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_QI,&arg,sizeof(arg));
		 arg=10;
		 th_decode_ctl(td,TH_DECCTL_SET_TELEMETRY_BITS,&arg,sizeof(arg));
		 }*/
	} else {
		/* tear down the partial theora setup */
		th_info_clear(&handle->ti);
		th_comment_clear(&handle->tc);
		handle->theora_p = 0;
	}

	th_setup_free(handle->ts);

	if (handle->vorbis_p) {
		vorbis_synthesis_init(&handle->vd, &handle->vi);
		vorbis_block_init(&handle->vd, &handle->vb);
		fprintf(stderr,
				"Ogg logical stream %lx is Vorbis %d channel %ld Hz audio.\n",
				handle->vo.serialno, handle->vi.channels, handle->vi.rate);
	} else {
		/* tear down the partial vorbis setup */
		vorbis_info_clear(&handle->vi);
		vorbis_comment_clear(&handle->vc);
		handle->vorbis_p = 0;
	}

	handle->file = file;
	handle->tref = LUA_NOREF;
	/*	  if (!handle->vorbis_p)
	 {
	 //We need an audio track
	 gaudio_OggClose(gid);
	 if (error)
	 *error = GAUDIO_UNRECOGNIZED_FORMAT;
	 return 0;
	 }*/

	handle->audio_time = 0;
	handle->videobuf_time = 0;
	handle->video_granulepos = -1;
	handle->audio_granulepos = 0; /* time position of last sample */

	int rate = handle->vorbis_p ? handle->vi.rate : 22050;
	int channels = handle->vorbis_p ? handle->vi.channels : 2;
	handle->sampleRate = rate;
	handle->sampleSize = channels * 2;
	if (numChannels)
		*numChannels = channels;
	if (sampleRate)
		*sampleRate = rate;

	if (bitsPerSample)
		*bitsPerSample = 16;
	long cpos = g_ftell(file);
	g_fseek(file, 0, SEEK_END);
	handle->sampleMax = sampleTell(handle,NULL,NULL,NULL,NULL);
	g_fseek(file, cpos, SEEK_SET);
	if (numSamples)
		*numSamples = handle->sampleMax;
	if (error)
		*error = GAUDIO_NO_ERROR;

	return gid;
}

long int gaudio_OggTell(g_id gid) {
	GGOggHandle *handle = (GGOggHandle*) gid;
	double time = handle->vorbis_p ? handle->audio_time : handle->videobuf_time;

	return (long int) (time * handle->sampleRate);
}

int gaudio_OggSeek(g_id gid, long int offset, int whence) {
	GGOggHandle *handle = (GGOggHandle*) gid;
	if (whence == SEEK_CUR)
		offset += gaudio_OggTell(gid);
	if (whence == SEEK_END)
		offset += ((handle->sampleMax > 0) ? handle->sampleMax : 0);

	int cgp = -1;
	handle->audio_time = 0;
	handle->videobuf_time = 0;
	handle->video_granulepos = -1;
	handle->audio_granulepos = 0; /* time position of last sample */
	if (offset == 0) //Rewind, only case needed for looping
	{
		cgp = g_fseek(handle->file, 0, SEEK_SET);
	} else {
		g_fseek(handle->file, 0, SEEK_END);
		long re = g_ftell(handle->file);
		long rs = 0;
		double stime=0;
		ogg_int64_t sgrn=-1;
		while (true) {
			long rm = (rs + re) / 2;
			g_fseek(handle->file, rm, SEEK_SET);
			int gp = sampleTell(handle,&handle->audio_granulepos,&handle->audio_time,&handle->video_granulepos,&handle->videobuf_time);
			if ((gp == cgp) || (gp == offset))
				break;
			cgp = gp;
			if (gp > offset)
				re = rm;
			else
				rs = rm;
		}
	}

	if (handle->vorbis_p)
	{
		vorbis_synthesis_restart(&handle->vd);
	    ogg_stream_reset_serialno(&handle->vo,handle->vo.serialno);
	}
	if (handle->theora_p)
	    ogg_stream_reset_serialno(&handle->to,handle->to.serialno);

	return cgp;
}

void gaudio_OggFormat(g_id gid, int *csr, int *chn) {
	GGOggHandle *handle = (GGOggHandle*) gid;

	*csr = handle->vi.rate;
	*chn = handle->vi.channels;
}

size_t gaudio_OggRead(g_id gid, size_t size, void *data) {
	GGOggHandle *handle = (GGOggHandle*) gid;
	/* on to the main decode loop.  We assume in this example that audio
	 and video start roughly together, and don't begin playback until
	 we have a start frame for both.  This is not necessarily a valid
	 assumption in Ogg A/V streams! It will always be true of the
	 example_encoder (and most streams) though. */

	/* single frame video buffering */

	/* single audio fragment audio buffering */
	int audiobuf_fill = 0;
	int audiobuf_ready = 0;
	int audiofd_fragsize = size;
	ogg_int16_t *audiobuf = (ogg_int16_t *) data;
	int i, j;
    bool queued=true;

    while (queued) {

		/* we want a video and audio frame ready to go at all times.  If
		 we have to buffer incoming, buffer the compressed data (ie, let
		 ogg do the buffering) */
		while (handle->vorbis_p && !audiobuf_ready) {
			int ret;
			float **pcm;

			/* if there's pending, decoded audio, grab it */
			if ((ret = vorbis_synthesis_pcmout(&handle->vd, &pcm)) > 0) {
				int count = audiobuf_fill / 2;
				int maxsamples = (audiofd_fragsize - audiobuf_fill) / 2
						/ handle->vi.channels;
				for (i = 0; i < ret && i < maxsamples; i++)
					for (j = 0; j < handle->vi.channels; j++) {
						int val = rint(pcm[j][i] * 32767.f);
						if (val > 32767)
							val = 32767;
						if (val < -32768)
							val = -32768;
						audiobuf[count++] = val;
					}
				vorbis_synthesis_read(&handle->vd, i);
				audiobuf_fill += i * handle->vi.channels * 2;
				if (audiobuf_fill == audiofd_fragsize)
					audiobuf_ready = 1;
				if (handle->vd.granulepos >= 0)
					handle->audio_granulepos = handle->vd.granulepos - ret + i;
				else
					handle->audio_granulepos += i;
				handle->audio_time = vorbis_granule_time(&handle->vd,
						handle->audio_granulepos);
			} else {

				/* no pending audio; is there a pending packet to decode? */
				if (ogg_stream_packetout(&handle->vo, &handle->op) > 0) {
					if (vorbis_synthesis(&handle->vb, &handle->op) == 0) /* test for success! */
						vorbis_synthesis_blockin(&handle->vd, &handle->vb);
				} else
					/* we need more data; break out to suck in another page */
					break;
			}
		}

		while (handle->theora_p && !handle->videobuf_ready) {
			/* theora is one in, one out... */
			if (ogg_stream_packetout(&handle->to, &handle->op) > 0) {

				if (handle->pp_inc) {
					handle->pp_level += handle->pp_inc;
					th_decode_ctl(handle->td, TH_DECCTL_SET_PPLEVEL,
							&handle->pp_level, sizeof(handle->pp_level));
					handle->pp_inc = 0;
				}
				/*HACK: This should be set after a seek or a gap, but we might not have
				 a granulepos for the first packet (we only have them for the last
				 packet on a page), so we just set it as often as we get it.
				 To do this right, we should back-track from the last packet on the
				 page and compute the correct granulepos for the first packet after
				 a seek or a gap.*/
				if (handle->op.granulepos >= 0) {
					th_decode_ctl(handle->td, TH_DECCTL_SET_GRANPOS,
							&handle->op.granulepos,
							sizeof(handle->op.granulepos));
				}
				if (th_decode_packetin(handle->td, &handle->op,
						&handle->video_granulepos) == 0) {
					handle->videobuf_time = th_granule_time(handle->td,
							handle->video_granulepos);

					/* is it already too old to be useful?  This is only actually
					 useful cosmetically after a SIGSTOP.  Note that we have to
					 decode the frame even if we don't show it (for now) due to
					 keyframing.  Soon enough libtheora will be able to deal
					 with non-keyframe seeks.  */

					if ((handle->videobuf_time
							>= (g_iclock() - (handle->playstart)))
							|| (!handle->playstart))
						handle->videobuf_ready = 1;
					else {
						//If we are too slow, reduce the pp level.
						handle->pp_inc = handle->pp_level > 0 ? -1 : 0;
						//dropped++;
					}
				}

			} else
				break;
		}

    /*	if (!handle->videobuf_ready && !audiobuf_ready && g_feof(handle->file))
            break;*/

		if (!handle->videobuf_ready || !audiobuf_ready) {
			/* no data yet for somebody.  Grab another page */
            queued=(buffer_data(handle->file, &handle->oy)>0);
			while (ogg_sync_pageout(&handle->oy, &handle->og) > 0) {
				queue_page(handle, &handle->og);
			}
		}

		/* are we at or past time for this video frame? */

		if (handle->stateflag && (audiobuf_ready || !handle->vorbis_p)
				&& (handle->videobuf_ready || !handle->theora_p)) {
			/* we have an audio frame ready (which means the audio buffer is
			 full), it's not time to play video, so wait until one of the
			 audio buffer is ready or it's near time to play video */
			break;
#if 0
			/* set up select wait on the audiobuffer and a timeout for video */
			struct timeval timeout;
			fd_set writefs;
			fd_set empty;
			int n=0;

			FD_ZERO(&writefs);
			FD_ZERO(&empty);
			if(audiofd>=0) {
				FD_SET(audiofd,&writefs);
				n=audiofd+1;
			}

			if(theora_p) {
				double tdiff;
				long milliseconds;
				tdiff=videobuf_time-get_time();
				/*If we have lots of extra time, increase the post-processing level.*/
				if(tdiff>ti.fps_denominator*0.25/ti.fps_numerator) {
					pp_inc=pp_level<pp_level_max?1:0;
				}
				else if(tdiff<ti.fps_denominator*0.05/ti.fps_numerator) {
					pp_inc=pp_level>0?-1:0;
				}
				milliseconds=tdiff*1000-5;
				if(milliseconds>500)milliseconds=500;
				if(milliseconds>0) {
					timeout.tv_sec=milliseconds/1000;
					timeout.tv_usec=(milliseconds%1000)*1000;

					n=select(n,&empty,&writefs,&empty,&timeout);
					if(n)audio_calibrate_timer(0);
				}
			} else {
				select(n,&empty,&writefs,&empty,NULL);
			}
#endif
		}
		if ((audiobuf_ready || (!handle->vorbis_p))
				&& (handle->videobuf_ready || (!handle->theora_p)))
			break;
	}

	handle->stateflag = 1;
	if (!handle->playstart)
		handle->playstart = g_iclock();
	return audiobuf_fill;
}

void gaudio_OggClose(g_id gid) {
	GGOggHandle *handle = (GGOggHandle*) gid;

	ctxmap.erase(gid);
	if (handle->tref != LUA_NOREF) {
		lua_unref(::L, handle->tref);
		handle->tref = LUA_NOREF;
	}

	if (handle->vorbis_p) {
		ogg_stream_clear(&handle->vo);
		vorbis_block_clear(&handle->vb);
		vorbis_dsp_clear(&handle->vd);
		vorbis_comment_clear(&handle->vc);
		vorbis_info_clear(&handle->vi);
	}
	if (handle->theora_p) {
		ogg_stream_clear(&handle->to);
		th_decode_free(handle->td);
		th_comment_clear(&handle->tc);
		th_info_clear(&handle->ti);
	}
	ogg_sync_clear(&handle->oy);
	g_fclose(handle->file);
	delete handle;
}

//Encoder
g_id gsoundencoder_OggCreate(const char *fileName, int numChannels,
		int sampleRate, int bitsPerSample, float quality) {
	FILE *fos = fopen(gpath_transform(fileName), "wb");
	if (fos == NULL)
		return 0;
	GGOggEncHandle *handle = new GGOggEncHandle;
	handle->fos = fos;
	handle->bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;

	vorbis_info_init(&handle->vi);
	int ret = vorbis_encode_init_vbr(&handle->vi, numChannels, sampleRate,
			quality);

	/* do not continue if setup failed; this can happen if we ask for a
	 mode that libVorbis does not support (eg, too low a bitrate, etc,
	 will return 'OV_EIMPL') */

	if (ret) {
		delete handle;
		return (g_id) 0;
	}

	/* add a comment */
	vorbis_comment_init(&handle->vc);
	vorbis_comment_add_tag(&handle->vc, "ENCODER", "Gideros");

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init(&handle->vd, &handle->vi);
	vorbis_block_init(&handle->vd, &handle->vb);

	/* set up our packet->stream encoder */
	/* pick a random serial number; that way we can more likely build
	 chained streams just by concatenation */
	srand(time(NULL));
	ogg_stream_init(&handle->os, rand());

	/* Vorbis streams begin with three headers; the initial header (with
	 most of the codec setup parameters) which is mandated by the Ogg
	 bitstream spec.  The second header holds any comment fields.  The
	 third header holds the bitstream codebook.  We merely need to
	 make the headers, then pass them to libvorbis one at a time;
	 libvorbis handles the additional Ogg bitstream constraints */

	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;

		vorbis_analysis_headerout(&handle->vd, &handle->vc, &header,
				&header_comm, &header_code);
		ogg_stream_packetin(&handle->os, &header); /* automatically placed in its own
		 page */
		ogg_stream_packetin(&handle->os, &header_comm);
		ogg_stream_packetin(&handle->os, &header_code);

		/* This ensures the actual
		 * audio data will start on a new page, as per spec
		 */
		while (true) {
			int result = ogg_stream_flush(&handle->os, &handle->og);
			if (result == 0)
				break;
			fwrite(handle->og.header, 1, handle->og.header_len, handle->fos);
			fwrite(handle->og.body, 1, handle->og.body_len, handle->fos);
		}

	}

	return (g_id) handle;
}

size_t gsoundencoder_OggWrite(g_id id, size_t size, void *data) {
	GGOggEncHandle *handle = (GGOggEncHandle*) id;
	int eos = 0;
	long i;
	if (size == 0) {
		/* end of file.  this can be done implicitly in the mainline,
		 but it's easier to see here in non-clever fashion.
		 Tell the library we're at end of stream so that it can handle
		 the last frame and mark end of stream in the output properly */
		vorbis_analysis_wrote(&handle->vd, 0);

	} else {
		/* data to encode */
		signed char *readbuffer = (signed char *) data;

		/* expose the buffer to submit data */
		float **buffer = vorbis_analysis_buffer(&handle->vd,
				size / handle->bytesPerSample);

		/* uninterleave samples */
		if (handle->bytesPerSample == 4) {
			for (i = 0; i < size / 4; i++) {
				buffer[0][i] = ((readbuffer[i * 4 + 1] << 8)
						| (0x00ff & (int) readbuffer[i * 4])) / 32768.f;
				buffer[1][i] = ((readbuffer[i * 4 + 3] << 8)
						| (0x00ff & (int) readbuffer[i * 4 + 2])) / 32768.f;
			}
		} else {
			for (i = 0; i < size / 2; i++) {
				buffer[0][i] = ((readbuffer[i * 2 + 1] << 8)
						| (0x00ff & (int) readbuffer[i * 2])) / 32768.f;
			}
		}

		/* tell the library how much we actually submitted */
		vorbis_analysis_wrote(&handle->vd, i);
	}

	/* vorbis does some data preanalysis, then divvies up blocks for
	 more involved (potentially parallel) processing.  Get a single
	 block for encoding now */
	while (vorbis_analysis_blockout(&handle->vd, &handle->vb) == 1) {

		/* analysis, assume we want to use bitrate management */
		vorbis_analysis(&handle->vb, NULL);
		vorbis_bitrate_addblock(&handle->vb);

		while (vorbis_bitrate_flushpacket(&handle->vd, &handle->op)) {

			/* weld the packet into the bitstream */
			ogg_stream_packetin(&handle->os, &handle->op);

			/* write out pages (if any) */
			while (!eos) {
				int result = ogg_stream_pageout(&handle->os, &handle->og);
				if (result == 0)
					break;
				fwrite(handle->og.header, 1, handle->og.header_len,
						handle->fos);
				fwrite(handle->og.body, 1, handle->og.body_len, handle->fos);

				/* this could be set above, but for illustrative purposes, I do
				 it here (to show that vorbis does know where the stream ends) */

				if (ogg_page_eos(&handle->og))
					eos = 1;
			}
		}
	}
	return size;
}

void gsoundencoder_OggClose(g_id id) {
	GGOggEncHandle *handle = (GGOggEncHandle*) id;
	/* clean up and exit.  vorbis_info_clear() must be called last */
	gsoundencoder_OggWrite(id, 0, NULL);

	ogg_stream_clear(&handle->os);
	vorbis_block_clear(&handle->vb);
	vorbis_dsp_clear(&handle->vd);
	vorbis_comment_clear(&handle->vc);
	vorbis_info_clear(&handle->vi);
	fclose(handle->fos);
}

}

class Renderer: public Ticker {
	void tick();
	void renderContext(GGOggHandle *hnd);
};

void Renderer::renderContext(GGOggHandle *handle) {
	if (handle->stateflag && handle->videobuf_ready
			&& handle->videobuf_time <= (g_iclock() - handle->playstart)) {
		th_ycbcr_buffer yuv;
		th_decode_ycbcr_out(handle->td, yuv);
		ScreenManager *sm = gtexture_get_screenmanager();
		if (sm)
			sm->screenDestroyed();
		if (handle->yplane) {
			gtexture_getInternalTexture(handle->yplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					yuv[0].stride, yuv[0].height, yuv[0].data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		}
		if (handle->uplane)
			gtexture_getInternalTexture(handle->uplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					yuv[1].stride, yuv[1].height, yuv[1].data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		if (handle->vplane)
			gtexture_getInternalTexture(handle->vplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					yuv[2].stride, yuv[2].height, yuv[2].data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		bool changed = (handle->planeWidth != yuv[0].stride)
				|| (handle->planeHeight != yuv[0].height);
		handle->planeWidth = yuv[0].stride;
		handle->planeHeight = yuv[0].height;
		if (changed && (handle->tref != LUA_NOREF)) {
			lua_State *L = ::L;
			lua_getref(L, handle->tref);
			lua_pushinteger(L, handle->planeWidth);
			lua_pushinteger(L, handle->planeHeight);
			if (lua_pcall(L, 2, 0, 0) != 0)
				lua_pop(L, 1); //Pop error message, as we have no way to display it in this context
		}
		handle->videobuf_ready = 0;
	}

}

void Renderer::tick() {
	for (std::map<g_id, GGOggHandle *>::iterator it = ctxmap.begin();
			it != ctxmap.end(); it++) {
		if (it->second)
			renderContext(it->second);
	}
}

static Renderer renderer;

static int getVideoInfo(lua_State* L) {
	g_id gid = luaL_checkinteger(L, 1);
	GGOggHandle *hnd = ctxmap[gid];
	if ((!hnd) || (!hnd->theora_p))
		lua_pushnil(L);
	else {
		lua_newtable(L);
		lua_pushinteger(L, hnd->ti.frame_width);
		lua_setfield(L, -2, "width");
		lua_pushinteger(L, hnd->ti.frame_height);
		lua_setfield(L, -2, "height");
		lua_pushinteger(L, hnd->planeWidth);
		lua_setfield(L, -2, "surfaceWidth");
		lua_pushinteger(L, hnd->planeHeight);
		lua_setfield(L, -2, "surfaceHeight");
		lua_pushnumber(L,
				((double) (hnd->ti.fps_numerator)) / hnd->ti.fps_denominator);
		lua_setfield(L, -2, "fps");
		lua_pushinteger(L, hnd->ti.pixel_fmt);
		lua_setfield(L, -2, "format");
	}
	return 1;
}

static int setVideoSurface(lua_State* L) {
	g_id gid = luaL_checkinteger(L, 1);
	GGOggHandle *hnd = ctxmap[gid];
	if ((!hnd) || (!hnd->theora_p)) {
		lua_pushstring(L, "Video stream Id invalid");
		lua_error(L);
	} else {
		TextureBase* textureBase = static_cast<TextureBase*>(g_getInstance(L,
				"TextureBase", 2));
		if (hnd->yplane)
			hnd->yplane->unref();
		if (textureBase)
			textureBase->ref();
		hnd->yplane = textureBase;
		textureBase = static_cast<TextureBase*>(g_getInstance(L, "TextureBase",
				3));
		if (hnd->uplane)
			hnd->uplane->unref();
		if (textureBase)
			textureBase->ref();
		hnd->uplane = textureBase;
		textureBase = static_cast<TextureBase*>(g_getInstance(L, "TextureBase",
				4));
		if (hnd->vplane)
			hnd->vplane->unref();
		if (textureBase)
			textureBase->ref();
		hnd->vplane = textureBase;
		lua_getfield(L, 2, "_videoSizeUpdated");
		if (lua_isfunction(L, -1)) {
			if (hnd->tref == LUA_NOREF)
				lua_unref(L, hnd->tref);
			hnd->tref = lua_ref(L, 1);
		} else
			lua_pop(L, 1);
	}
	return 0;
}

static int loader(lua_State* L) {
	const luaL_Reg functionlist[] = { { "getVideoInfo", getVideoInfo }, {
			"setVideoSurface", setVideoSurface }, { NULL, NULL }, };

	lua_newtable(L);
	luaL_register(L, NULL, functionlist);

	return 1;
}

GGAudioLoader audioOgg(gaudio_OggOpen, gaudio_OggClose, gaudio_OggRead,
		gaudio_OggSeek, gaudio_OggTell);
GGAudioEncoder audioEncOgg(gsoundencoder_OggCreate, gsoundencoder_OggClose,
		gsoundencoder_OggWrite);

static void g_initializePlugin(lua_State *L) {
	::L = L;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "theora.core");

	lua_pop(L, 2);

	lua_getglobal(L, "application");
	LuaApplication* application = static_cast<LuaApplication *>(luaL_getdata(L));
	application->addTicker(&renderer);
	lua_pop(L, 1);

	audioOgg.format = gaudio_OggFormat;
	gaudio_registerType("ogg", audioOgg);
	gaudio_registerType("oga", audioOgg);
	gaudio_registerType("ogv", audioOgg);
	gaudio_registerEncoderType("ogg", audioEncOgg);
	gaudio_registerEncoderType("oga", audioEncOgg);
}

static void g_deinitializePlugin(lua_State *L) {
	lua_getglobal(L, "application");
	LuaApplication* application = static_cast<LuaApplication *>(luaL_getdata(L));
	application->removeTicker(&renderer);
	lua_pop(L, 1);

	gaudio_unregisterType("ogg");
	gaudio_unregisterType("oga");
	gaudio_unregisterType("ogv");
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("Ogg", "1.0",Ogg)
#else
REGISTER_PLUGIN_NAMED("Ogg", "1.0", Ogg)
#endif
