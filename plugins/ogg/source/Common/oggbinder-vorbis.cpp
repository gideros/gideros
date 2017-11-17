#include "lauxlib.h"
#include <gideros.h>

#include <gaudio.h>
#include <ggaudiomanager.h>
#include <gstdio.h>

#include <vorbis/vorbisfile.h>
#include <theora/theora.h>
#include <theora/theoradec.h>

struct GGOggHandle
{
	OggVorbis_File vf;
	int samplerate;
	int channels;
	//OGG GENERIC
	ogg_sync_state   oy;
	ogg_page         og;
	ogg_stream_state vo;
	ogg_stream_state to;
	th_info      ti;
	th_comment   tc;
	th_dec_ctx       *td;
	th_setup_info    *ts;
	vorbis_info      vi;
	vorbis_dsp_state vd;
	vorbis_block     vb;
	vorbis_comment   vc;
	th_pixel_fmt     px_fmt;

	int              theora_p=0;
	int              vorbis_p=0;
	int              stateflag=0;
};

extern "C" {

static int _fseek64_wrap(G_FILE *f,ogg_int64_t off,int whence){
  if(f==NULL)return(-1);
  return g_fseek(f,off,whence);
}

g_id gaudio_OggOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error)
{


	  ov_callbacks callbacks = {
	    (size_t (*)(void *, size_t, size_t, void *))  g_fread,
	    (int (*)(void *, ogg_int64_t, int))           _fseek64_wrap,
	    (int (*)(void *))                             g_fclose,
	    (long (*)(void *))                            g_ftell
	  };


    G_FILE *file = g_fopen(fileName, "rb");
    if (file == NULL)
    {
        if (error)
            *error = GAUDIO_CANNOT_OPEN_FILE;
        return 0;
    }

    GGOggHandle *handle = new GGOggHandle();
	if (ov_open_callbacks((void *)file, &handle->vf, NULL, 0, callbacks)<0)
	{
		delete handle;
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        return 0;
	}

	vorbis_info *vi=ov_info(&handle->vf,-1);
	handle->samplerate=vi->rate;
	handle->channels=vi->channels;

    if (numChannels)
        *numChannels = vi->channels;
    if (sampleRate)
        *sampleRate = vi->rate;
    if (bitsPerSample)
        *bitsPerSample = 16;
    if (numSamples)
        *numSamples = ov_pcm_total(&handle->vf,-1);
    if (error)
        *error = GAUDIO_NO_ERROR;


    return (g_id)handle;
}

int gaudio_OggSeek(g_id gid, long int offset, int whence)
{
    GGOggHandle *handle = (GGOggHandle*)gid;

    return ov_raw_seek(&handle->vf, offset);
}

long int gaudio_OggTell(g_id gid)
{
    GGOggHandle *handle = (GGOggHandle*)gid;

    return ov_raw_tell(&handle->vf);
}

void gaudio_OggFormat(g_id gid,int *csr,int *chn)
{
    GGOggHandle *handle = (GGOggHandle*)gid;

	vorbis_info *vi=ov_info(&handle->vf,-1);
	handle->samplerate=vi->rate;
	handle->channels=vi->channels;
	*csr=vi->rate;
	*chn=vi->channels;
}

size_t gaudio_OggRead(g_id gid, size_t size, void *data)
{
	GGOggHandle *handle = (GGOggHandle*)gid;
	char *d=(char *)data;
	int bs;
	int done=0;
	while (size>128) {
		int err=ov_read(&handle->vf,d,size,0,2,1,&bs);
	    if (err == OV_HOLE || err >= 0)
	    {
	    	if (err>0)
	    	{
	    		d=d+err;
	    		done=done+err;
	    		size=size-err;
	    		continue;
	    	}
    		break;
	    }

	    return 0;
	}


    return done;
}

void gaudio_OggClose(g_id gid)
{
	GGOggHandle *handle = (GGOggHandle*)gid;

	ov_clear(&handle->vf);
    delete handle;
}

}


static int loader(lua_State* L)
{
    const luaL_Reg functionlist[] = {
        {NULL, NULL},
    };

    //g_createClass(L, "Theora", "Sound", create, destruct, functionlist);

    lua_getglobal(L, "Theora");
    return 1;
}

GGAudioLoader audioOgg(gaudio_OggOpen, gaudio_OggClose, gaudio_OggRead, gaudio_OggSeek, gaudio_OggTell);

static void g_initializePlugin(lua_State *L)
{
    /*lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "theora");

    lua_pop(L, 2);*/

	audioOgg.format=gaudio_OggFormat;
    gaudio_registerType("ogg",audioOgg);
}

static void g_deinitializePlugin(lua_State *L)
{
	gaudio_unregisterType("ogg");
}

REGISTER_PLUGIN("Ogg", "1.0")
