#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"
#include "glog.h"

#include <gaudio.h>
#include <ggaudiomanager.h>
#include <gstdio.h>
#include <Shaders.h>
#include <texturebase.h>

#include <math.h>
#include <platformutil.h>
#include "screen.h"
#include "ticker.h"
#include "luaapplication.h"

#include <OggDec.h>
#include <OggEnc.h>

#ifndef FLAVOUR
#define FLAVOUR_F
#endif

static lua_State *L = NULL;
struct GGOggHandle {
	//OGG GENERIC
	G_FILE *file;
	ogg_packet op;
	ogg_sync_state oy;
	ogg_page og;
	ogg_stream_state ao;
	ogg_stream_state vo;
	//COMMON
	OggDec *video_p;
	OggDec *audio_p;
	unsigned int frame_width,frame_height;
	float frame_rate;
	int frame_format;
	ogg_int64_t audio_granulepos;
	ogg_int64_t video_granulepos;
	TextureBase *yplane;
	TextureBase *uplane;
	TextureBase *vplane;
	int planeWidth, planeHeight;
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

	OggEnc *audio_p;
};

static std::map<g_id, GGOggHandle *> ctxmap;

int buffer_data(G_FILE *in, ogg_sync_state *oy) {
	char *buffer = ogg_sync_buffer(oy, 4096);
	int bytes = g_fread(buffer, 1, 4096, in);
	ogg_sync_wrote(oy, bytes);
	return (bytes);
}

static int queue_page(GGOggHandle *h, ogg_page *page) {
	if (h->video_p)
		ogg_stream_pagein(&h->vo, page);
	if (h->audio_p)
		ogg_stream_pagein(&h->ao, page);
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
		if (handle->audio_p && (psn == handle->ao.serialno))
		{
			gt=handle->audio_p->GranuleTime(granulepos);
			if (agr) *agr=granulepos;
			if (atm) *atm=gt;
		}
		if (handle->video_p && (psn == handle->vo.serialno))
		{
			gt=handle->video_p->GranuleTime(granulepos);
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

	g_id gid = g_NextId();
	ctxmap[gid] = handle;
	handle->audio_p=NULL;
	handle->video_p=NULL;

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
			if (!handle->video_p
					&& ((handle->video_p=probe_oggdec(&handle->op,CODEC_TYPE_VIDEO)) !=NULL)) {
				/* it is theora */
				memcpy(&handle->vo, &test, sizeof(test));
			} else
				if (!handle->audio_p
						&& ((handle->audio_p=probe_oggdec(&handle->op,CODEC_TYPE_AUDIO)) !=NULL)) {
				/* it is vorbis */
				memcpy(&handle->ao, &test, sizeof(test));
			} else {
				/* whatever it is, we don't care about it */
				ogg_stream_clear(&test);
			}
		}
		/* fall through to non-bos page parsing */
	}

	/* we're expecting more header packets. */
	while ((handle->video_p && (!handle->video_p->GotHeaders()))
			|| (handle->audio_p && (!handle->audio_p->GotHeaders()))) {
		int ret;

		/* look for further theora headers */
		while (handle->video_p && (!handle->video_p->GotHeaders()) && (ret =
				ogg_stream_packetout(&handle->vo, &handle->op))) {
			if (ret < 0) {
                glog_w("Error parsing Video stream headers; "
						"corrupt stream?\n");
                delete handle;
                gaudio_OggClose(gid);
                if (error)
                    *error = GAUDIO_UNRECOGNIZED_FORMAT;
                return 0;
            }
			if (!handle->video_p->PacketIn(&handle->op)) {
				fprintf(stderr, "Error parsing Theora stream headers; "
						"corrupt stream?\n");
                delete handle;
                gaudio_OggClose(gid);
                if (error)
                    *error = GAUDIO_UNRECOGNIZED_FORMAT;
                return 0;
            }
		}
		/* look for more vorbis header packets */
		while (handle->audio_p && (!handle->audio_p->GotHeaders()) && (ret =
				ogg_stream_packetout(&handle->ao, &handle->op))) {
			if (ret < 0) {
                glog_w(	"Error parsing Audio stream headers; corrupt stream?\n");
                delete handle;
                gaudio_OggClose(gid);
                if (error)
                    *error = GAUDIO_UNRECOGNIZED_FORMAT;
                return 0;
            }

			if (!handle->audio_p->PacketIn(&handle->op)) {
                glog_w(	"Error parsing Audio stream headers; corrupt stream?\n");
                delete handle;
                gaudio_OggClose(gid);
                if (error)
                    *error = GAUDIO_UNRECOGNIZED_FORMAT;
                return 0;
            }
		}

		/* The header pages/packets will arrive before anything else we
		 care about, or the stream is not obeying spec */

		if (ogg_sync_pageout(&handle->oy, &handle->og) > 0) {
			queue_page(handle, &handle->og); /* demux into the appropriate stream */
		} else {
			int ret = buffer_data(file, &handle->oy); /* someone needs more data */
			if (ret == 0) {
                glog_w("End of file while searching for codec headers.\n");
                delete handle;
				gaudio_OggClose(gid);
				if (error)
					*error = GAUDIO_UNRECOGNIZED_FORMAT;
				return 0;
			}
		}
	}

	/* and now we have it all.  initialize decoders */
	if (handle->video_p) {
		handle->video_p->GetVideoInfo(handle->frame_rate, handle->frame_width, handle->frame_height, handle->frame_format);
        glog_i("Ogg logical stream %lx is Video %dx%d %.02f fps",
				handle->vo.serialno, handle->frame_width,
				handle->frame_height,
				handle->frame_rate);
		switch (handle->frame_format) {
		case 0:
            glog_i(" 4:2:0 video\n");
			break;
		case 2:
            glog_i(" 4:2:2 video\n");
			break;
		case 3:
            glog_i(" 4:4:4 video\n");
			break;
		default:
            glog_i(" video\n  (UNKNOWN Chroma sampling!)\n");
			break;
		}
	}


	unsigned int rate=22050,channels=2;
	if (handle->audio_p) {
		handle->audio_p->GetAudioInfo(rate, channels);
        glog_i(	"Ogg logical stream %lx is Audio %d channel %ld Hz audio.\n",
				handle->ao.serialno, channels, rate);
	}

	handle->file = file;
	handle->tref = LUA_NOREF;
	/*	  if (!handle->audio_p)
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
	GGOggHandle *handle = ctxmap[gid];
	double time = handle->audio_p ? handle->audio_time : handle->videobuf_time;

	return (long int) (time * handle->sampleRate);
}

int gaudio_OggSeek(g_id gid, long int offset, int whence) {
	GGOggHandle *handle = ctxmap[gid];
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

	if (handle->audio_p)
	{
		handle->audio_p->Restart();
	    ogg_stream_reset_serialno(&handle->ao,handle->ao.serialno);
	}
	if (handle->video_p) {
		handle->video_p->Restart();
	    ogg_stream_reset_serialno(&handle->vo,handle->vo.serialno);
	}
	return cgp;
}

void gaudio_OggFormat(g_id gid, int *csr, int *chn) {
	GGOggHandle *handle = ctxmap[gid];
	unsigned int rate,channels;
	handle->audio_p->GetAudioInfo(rate, channels);
	*csr = rate;
	*chn = channels;
}

size_t gaudio_OggRead(g_id gid, size_t size, void *data) {
	GGOggHandle *handle = ctxmap[gid];
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
		while (handle->audio_p && !audiobuf_ready) {
			int ret;

			/* if there's pending, decoded audio, grab it */
			int maxsamples = (audiofd_fragsize - audiobuf_fill) / 2;
			ogg_int64_t gpos=0;
			if ((ret = handle->audio_p->GetAudio(audiobuf+(audiobuf_fill/2), maxsamples,gpos)) > 0) {
				audiobuf_fill += ret * 2;
				if (audiobuf_fill == audiofd_fragsize)
					audiobuf_ready = 1;
				if (gpos >= 0)
                    handle->audio_granulepos = gpos;
				else
					handle->audio_granulepos += i;
				handle->audio_time = handle->audio_p->GranuleTime(handle->audio_granulepos);
			} else {

				/* no pending audio; is there a pending packet to decode? */
				if (ogg_stream_packetout(&handle->ao, &handle->op) > 0) {
					handle->audio_p->PacketIn(&handle->op);
				} else
					/* we need more data; break out to suck in another page */
					break;
			}
		}

		while (handle->video_p && !handle->videobuf_ready) {
			/* theora is one in, one out... */
			if (ogg_stream_packetout(&handle->vo, &handle->op) > 0) {
				if (handle->video_p->PacketIn(&handle->op)) {
					if (handle->video_p->HasVideoFrame(handle->video_granulepos)) {
						handle->videobuf_time = handle->video_p->GranuleTime(handle->video_granulepos);

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
							//handle->pp_inc = handle->pp_level > 0 ? -1 : 0;
							//dropped++;
						}
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
				queued=true;
			}
		}

		/* are we at or past time for this video frame? */

		if (handle->stateflag && (audiobuf_ready || !handle->audio_p)
				&& (handle->videobuf_ready || !handle->video_p)) {
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

			if(video_p) {
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
		if ((audiobuf_ready || (!handle->audio_p))
				&& (handle->videobuf_ready || (!handle->video_p)))
			break;
	}

	handle->stateflag = 1;
	if (!handle->playstart)
		handle->playstart = g_iclock();
    //glog_i("Audio fill:%d avl:%d rdy:%d q:%d",audiobuf_fill,handle->audio_p,audiobuf_ready,queued);
	return audiobuf_fill;
}

void gaudio_OggClose(g_id gid) {
	GGOggHandle *handle = ctxmap[gid];

	ctxmap.erase(gid);
	if (!handle) return;
	if (handle->tref != LUA_NOREF) {
		lua_unref(::L, handle->tref);
		handle->tref = LUA_NOREF;
	}

	if (handle->audio_p) {
		ogg_stream_clear(&handle->ao);
		delete handle->audio_p;
	}
	if (handle->video_p) {
		ogg_stream_clear(&handle->vo);
		delete handle->video_p;
	}
	ogg_sync_clear(&handle->oy);
	g_fclose(handle->file);
	delete handle;
}

//Encoder
static std::map<g_id, GGOggEncHandle *> ctxmap2;
g_id gsoundencoder_OggCreate(const char *fileName, int numChannels,
		int sampleRate, int bitsPerSample, float quality) {
	FILE *fos = fopen(gpath_transform(fileName), "wb");
	if (fos == NULL)
		return 0;
	GGOggEncHandle *handle = new GGOggEncHandle;
	handle->fos = fos;
	handle->bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;

	handle->audio_p=build_oggenc("vorbis");
    if (!handle->audio_p) {
		delete handle;
		return (g_id) 0;
	}

	/* set up our packet->stream encoder */
	/* pick a random serial number; that way we can more likely build
	 chained streams just by concatenation */
	srand(time(NULL));
	ogg_stream_init(&handle->os, rand());

	if (!(handle->audio_p->InitAudio(numChannels,sampleRate,quality,&handle->os))) {
		/* do not continue if setup failed; this can happen if we ask for a
		 mode that libVorbis does not support (eg, too low a bitrate, etc,
		 will return 'OV_EIMPL') */
		ogg_stream_clear(&handle->os);
		delete handle->audio_p;
		delete handle;
		return (g_id) 0;
	}

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

	g_id gid= g_NextId();
	ctxmap2[gid]=handle;

	return gid;
}

size_t gsoundencoder_OggWrite(g_id id, size_t size, void *data) {
	GGOggEncHandle *handle = ctxmap2[id];
	handle->audio_p->WriteAudio(data,size);
	int eos = 0;

	/* vorbis does some data preanalysis, then divvies up blocks for
	 more involved (potentially parallel) processing.  Get a single
	 block for encoding now */
	while (handle->audio_p->PacketOut(&handle->op)) {

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
	return size;
}

void gsoundencoder_OggClose(g_id id) {
	GGOggEncHandle *handle = ctxmap2[id];
	if (!handle) return;
	/* clean up and exit.  vorbis_info_clear() must be called last */
	gsoundencoder_OggWrite(id, 0, NULL);
	ctxmap2.erase(id);

	ogg_stream_clear(&handle->os);
	if (handle->audio_p)
		delete handle->audio_p;
	fclose(handle->fos);
	delete handle;
}
GGAudioEncoder audioEncOgg(gsoundencoder_OggCreate, gsoundencoder_OggClose,
		gsoundencoder_OggWrite);
}

class Renderer: public Ticker {
	void tick();
	void renderContext(GGOggHandle *hnd);
};

void Renderer::renderContext(GGOggHandle *handle) {
	if (handle->stateflag && handle->videobuf_ready
			&& handle->videobuf_time <= (g_iclock() - handle->playstart)) {
		OggDec::VideoFrame vf;
		handle->video_p->GetVideoFrame(vf);
		ScreenManager *sm = gtexture_get_screenmanager();
		if (sm)
			sm->screenDestroyed();
		if (handle->yplane) {
			gtexture_getInternalTexture(handle->yplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					vf.y.stride, vf.y.height, vf.y.data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		}
		if (handle->uplane)
			gtexture_getInternalTexture(handle->uplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					vf.u.stride, vf.u.height, vf.u.data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		if (handle->vplane)
			gtexture_getInternalTexture(handle->vplane->data->gid)->updateData(
					ShaderTexture::FMT_Y, ShaderTexture::PK_UBYTE,
					vf.v.stride, vf.v.height, vf.v.data,
					ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
		bool changed = (handle->planeWidth != vf.y.stride)
				|| (handle->planeHeight != vf.y.height);
		handle->videobuf_ready = 0;
		handle->planeWidth = vf.y.stride;
		handle->planeHeight = vf.y.height;
		if (changed && (handle->tref != LUA_NOREF)) {
			lua_State *L = ::L;
			lua_getref(L, handle->tref);
			lua_pushinteger(L, handle->planeWidth);
			lua_pushinteger(L, handle->planeHeight);
			if (lua_pcall(L, 2, 0, 0) != 0)
				lua_pop(L, 1); //Pop error message, as we have no way to display it in this context
		}
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
	if ((!hnd) || (!hnd->video_p))
		lua_pushnil(L);
	else {
		lua_newtable(L);
		lua_pushinteger(L, hnd->frame_width);
		lua_setfield(L, -2, "width");
		lua_pushinteger(L, hnd->frame_height);
		lua_setfield(L, -2, "height");
		lua_pushinteger(L, hnd->planeWidth);
		lua_setfield(L, -2, "surfaceWidth");
		lua_pushinteger(L, hnd->planeHeight);
		lua_setfield(L, -2, "surfaceHeight");
		lua_pushnumber(L, hnd->frame_rate);
		lua_setfield(L, -2, "fps");
		lua_pushinteger(L, hnd->frame_format);
		lua_setfield(L, -2, "format");
	}
	return 1;
}

static int setVideoSurface(lua_State* L) {
	g_id gid = luaL_checkinteger(L, 1);
	GGOggHandle *hnd = ctxmap[gid];
	if ((!hnd) || (!hnd->video_p)) {
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

#ifndef PART_Core
extern const OggDecType theora_cinfo;
extern const OggDecType opus_cinfo;
extern const OggEncType opus_einfo;
extern const OggDecType vorbis_cinfo;
extern const OggEncType vorbis_einfo;
#endif

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

#ifndef PART_Core
    register_oggdec("vorbis",vorbis_cinfo);
    register_oggdec("opus",opus_cinfo);
    register_oggdec("theora",theora_cinfo);
    register_oggenc("vorbis",vorbis_einfo);
    //register_oggenc("opus",opus_cinfo);
#endif
}

static void g_deinitializePlugin(lua_State *L) {
	lua_getglobal(L, "application");
	LuaApplication* application = static_cast<LuaApplication *>(luaL_getdata(L));
	application->removeTicker(&renderer);
	lua_pop(L, 1);

	gaudio_unregisterType("ogg");
	gaudio_unregisterType("oga");
	gaudio_unregisterType("ogv");
#ifndef PART_Core
    unregister_oggdec("vorbis");
    unregister_oggdec("opus");
    unregister_oggdec("theora");
    unregister_oggenc("vorbis");
    //unregister_oggenc("opus");
#endif
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("Ogg", "1.0",Ogg)
#else
REGISTER_PLUGIN_NAMED("Ogg", "1.0", Ogg)
#endif
