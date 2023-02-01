#include <gaudio.h>
#include <gstdio.h>
#include <glog.h>
#include <xmp.h>
#include <map>

static std::map<g_id, xmp_context> ctxmap;

extern "C" {

g_id gaudio_XmpOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error)
{
	char fn[1024];
	xmp_context xc=xmp_create_context();
	strcpy(fn,fileName);
	int err=xmp_load_module(xc,fn);
	if (numChannels)
		*numChannels=2;
	if (sampleRate)
		*sampleRate=44100;
	if (bitsPerSample)
		*bitsPerSample=16;
	if (numSamples)
		*numSamples=0;
	if (err>=0)
	{
	    struct xmp_frame_info fmi;
	    xmp_get_frame_info(xc, &fmi);
	    xmp_start_player(xc,44100,0);
		if (numSamples)
			*numSamples=(int)(44.1*fmi.total_time);
	    g_id gid = g_NextId();
	    ctxmap[gid]=xc;
		if (error)
			*error = GAUDIO_NO_ERROR;
		return gid;
	}
	xmp_free_context(xc);
    if (error)
        *error = GAUDIO_CANNOT_OPEN_FILE;
    return 0;
}

int gaudio_XmpSeek(g_id gid, long int offset, int whence)
{
    struct xmp_frame_info fmi;
    xmp_context xc = ctxmap[gid];
	long int p=offset/44.1;
    xmp_get_frame_info(xc, &fmi);
	switch (whence)
	{
	case SEEK_SET: break;
	case SEEK_CUR: p=p+fmi.time; break;
	case SEEK_END: p=fmi.total_time-p; break;
	}
	if (p<0) p=0;
	if (p>fmi.total_time) p=fmi.total_time;
	xmp_seek_time(xc,p);
	return p;
}

long int gaudio_XmpTell(g_id gid)
{
    xmp_context xc = ctxmap[gid];
    struct xmp_frame_info fmi;
    xmp_get_frame_info(xc, &fmi);
    long int pos=44.1*fmi.time;
    return pos;
}

size_t gaudio_XmpRead(g_id gid, size_t size, void *data, unsigned int *streamPos)
{
    xmp_context xc = ctxmap[gid];
    size = (size / 4) * 4;
    xmp_play_buffer(xc,data,size,0);
    return size;
}

void gaudio_XmpClose(g_id gid)
{
    xmp_context xc = ctxmap[gid];
    ctxmap.erase(gid);
	xmp_end_player(xc);
	xmp_release_module(xc);
	xmp_free_context(xc);
}

}
