#include <gaudio.h>
#include <gstdio.h>
#include <map>
#include <mpg123.h>

static ssize_t mpg123read(void* fd, void* buf, size_t size)
{
    G_FILE* file = (G_FILE*)fd;
    return g_fread(buf, 1, size, file);
}

static off_t mpg123lseek(void* fd, off_t offset, int whence)
{
    G_FILE* file = (G_FILE*)fd;

    if (g_fseek(file, offset, whence) == 0)
        return g_ftell(file);

    return (off_t)-1;
}

static void mpg123cleanup(void* fd)
{
    G_FILE* file = (G_FILE*)fd;
    g_fclose(file);
}

struct GGMp3Handle
{
    mpg123_handle *mh;
    int sampleSize;
};

static std::map<g_id, GGMp3Handle *> ctxmap;

extern "C" {

g_id gaudio_Mp3Open(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error)
{
    int err = MPG123_OK;
    mpg123_handle *mh = mpg123_new(NULL, &err);

    if (mh == NULL || err != MPG123_OK)
    {
        if (error)
            *error = GAUDIO_INTERNAL_ERROR;
        return 0;
    }

    G_FILE *file = g_fopen(fileName, "rb");
    if (file == NULL)
    {
        mpg123_delete(mh);
        if (error)
            *error = GAUDIO_CANNOT_OPEN_FILE;
        return 0;
    }

    mpg123_replace_reader_handle(mh, mpg123read, mpg123lseek, mpg123cleanup);
    if (mpg123_open_handle(mh, file) != MPG123_OK)
    {
        mpg123_delete(mh);
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        return 0;
    }

    int channels = 0, encoding = 0;
    long rate = 0;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) !=  MPG123_OK)
    {
        mpg123_delete(mh);
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        return 0;
    }

    // although signed 16 is the default output format, we ensure that anyway.
    encoding = MPG123_ENC_SIGNED_16;

    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    mpg123_scan(mh);
    off_t sampleCount = mpg123_length(mh);

    if (numChannels)
        *numChannels = channels;
    if (sampleRate)
        *sampleRate = rate;
    if (bitsPerSample)
        *bitsPerSample = 16;
    if (numSamples)
        *numSamples = sampleCount;
    if (error)
        *error = GAUDIO_NO_ERROR;

    GGMp3Handle *handle = new GGMp3Handle();
    handle->mh = mh;
    handle->sampleSize = channels * 2;

    g_id gid = g_NextId();
    ctxmap[gid]=handle;

    return gid;
}

int gaudio_Mp3Seek(g_id gid, long int offset, int whence)
{
    GGMp3Handle *handle = ctxmap[gid];

    return mpg123_seek(handle->mh, offset, whence);
}

long int gaudio_Mp3Tell(g_id gid)
{
    GGMp3Handle *handle = ctxmap[gid];

    return mpg123_tell(handle->mh);
}

size_t gaudio_Mp3Read(g_id gid, size_t size, void *data)
{
    GGMp3Handle *handle = ctxmap[gid];

    size = (size / handle->sampleSize) * handle->sampleSize;

    size_t done;
    int err = mpg123_read(handle->mh, (unsigned char*)data, size, &done);

    if (err == MPG123_OK || err == MPG123_DONE)
        return done;

    return 0;
}

void gaudio_Mp3Close(g_id gid)
{
    GGMp3Handle *handle = ctxmap[gid];
    ctxmap.erase(gid);

    mpg123_close(handle->mh);
    mpg123_delete(handle->mh);
    delete handle;
}

}
