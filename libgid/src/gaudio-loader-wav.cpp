#include <gstdio.h>
#include <gaudio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>

struct GGWaveHeader
{
    unsigned int chunkID;
    unsigned int chunkSize;
    unsigned int format;
};

struct GGChunkHeader
{
    unsigned int chunkId;
    unsigned int chunkSize;
};

struct GGFmtChunk
{
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
};

struct GGWavHandle
{
    G_FILE *fis;
    int sampleSize;
    unsigned int dataPos;
    unsigned int dataSize;
    size_t playPos;
};
static std::map<g_id, GGWavHandle *> ctxmap;

extern "C" {

g_id gaudio_WavOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error)
{
    G_FILE *fis = g_fopen(fileName, "rb");
    if (fis == NULL)
    {
        if (error)
            *error = GAUDIO_CANNOT_OPEN_FILE;
        return 0;
    }

    GGWaveHeader header;
    if (g_fread(&header, sizeof(GGWaveHeader), 1, fis) != 1)
    {
        g_fclose(fis);
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        return 0;
    }

    if (header.chunkID != 0x46464952/*RIFF*/ || header.format != 0x45564157/*WAVE*/)
    {
        g_fclose(fis);
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        return 0;
    }

    // read "fmt " chunk
    GGFmtChunk format = {0};
    while (true)
    {
        GGChunkHeader chunk;
        if (g_fread(&chunk, sizeof(GGChunkHeader), 1, fis) != 1)
            break;

        if (chunk.chunkId == 0x20746D66/*fmt */)
        {
            if (g_fread(&format, sizeof(GGFmtChunk), 1, fis) != 1)
            {
                if (error)
                    *error = GAUDIO_ERROR_WHILE_READING;
                g_fclose(fis);
                return 0;
            }

            break;
        }

        g_fseek(fis, chunk.chunkSize, SEEK_CUR);
    }

    g_fseek(fis, sizeof(GGWaveHeader), SEEK_SET);

    // read "data" chunk
    unsigned int dataSize = 0;
    unsigned int dataPos = 0;
    while (true)
    {
        GGChunkHeader chunk;
        if (g_fread(&chunk, sizeof(GGChunkHeader), 1, fis) != 1)
            break;

        if (chunk.chunkId == 0x61746164/*data */)
        {
            dataPos = g_ftell(fis);
            dataSize = chunk.chunkSize;
            break;
        }

        g_fseek(fis, chunk.chunkSize, SEEK_CUR);
    }

    // data chunk is missing or zero sized
    if (dataPos == 0 || dataSize == 0)
    {
        if (error)
            *error = GAUDIO_UNRECOGNIZED_FORMAT;
        g_fclose(fis);
        return 0;
    }

    // only support PCM
    if (format.audioFormat != 1)
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        g_fclose(fis);
        return 0;
    }

    // only support 8 and 16 bits per sample
    if (format.bitsPerSample != 8 && format.bitsPerSample != 16)
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        g_fclose(fis);
        return 0;
    }

    g_fseek(fis, dataPos, SEEK_SET);

    // some writers put a bad blockAlign and/or byteRate value => recalculate them.
    format.blockAlign = (format.bitsPerSample / 8) * format.numChannels;
    format.byteRate = format.blockAlign * format.sampleRate;

    if (numChannels)
        *numChannels = format.numChannels;
    if (sampleRate)
        *sampleRate = format.sampleRate;
    if (bitsPerSample)
        *bitsPerSample = format.bitsPerSample;
    if (numSamples)
        *numSamples = dataSize / format.blockAlign;
    if (error)
        *error = GAUDIO_NO_ERROR;

    GGWavHandle *handle = new GGWavHandle();
    handle->fis = fis;
    handle->sampleSize = format.blockAlign;
    handle->dataPos = dataPos;
    handle->dataSize = dataSize;
    handle->playPos = 0;

    g_id gid = g_NextId();
    ctxmap[gid]=handle;

    return gid;
}

void gaudio_WavClose(g_id id)
{
    GGWavHandle *handle = ctxmap[id];
    ctxmap.erase(id);
    g_fclose(handle->fis);
    delete handle;
}

int gaudio_WavSeek(g_id id, long int offset, int whence)
{
    GGWavHandle *handle = ctxmap[id];

    if (whence == SEEK_SET)
    {
        offset = handle->dataPos + offset * handle->sampleSize;
    }
    else if (whence == SEEK_CUR)
    {
        offset = g_ftell(handle->fis) + offset * handle->sampleSize;
    }
    else if (whence == SEEK_END)
    {
        offset = handle->dataPos + handle->dataSize - offset * handle->sampleSize;
    }
    else
    {
        return -1;
    }

    offset = std::max(offset, (long int)handle->dataPos);

    g_fseek(handle->fis, offset, SEEK_SET);

    handle->playPos = (g_ftell(handle->fis) - handle->dataPos) / handle->sampleSize;

    return handle->playPos;
}

long int gaudio_WavTell(g_id id)
{
    GGWavHandle *handle = ctxmap[id];
    return (g_ftell(handle->fis) - handle->dataPos) / handle->sampleSize;
}

size_t gaudio_WavRead(g_id id, size_t size, void *data, unsigned int *streamPos)
{
    GGWavHandle *handle = ctxmap[id];

    size_t size2 = size / handle->sampleSize;
    size_t remain = (handle->dataSize / handle->sampleSize) - gaudio_WavTell(id);

    size_t adv=g_fread(data, handle->sampleSize, std::min(size2, remain), handle->fis);
    if (streamPos) *streamPos=handle->playPos;
    handle->playPos+=adv;
    return adv * handle->sampleSize;
}

}
