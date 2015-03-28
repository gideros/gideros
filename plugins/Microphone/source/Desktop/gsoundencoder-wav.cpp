#include "gsoundencoder.h"
#include <stdio.h>
#include <stdint.h>
#include <gpath.h>

namespace {

struct GWaveHeader
{
    uint32_t chunkID;
    uint32_t chunkSize;
    uint32_t format;
};

struct GChunkHeader
{
    uint32_t chunkId;
    uint32_t chunkSize;
};

struct GFmtChunk
{
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct GWavHandle
{
    FILE *fos;
    int numChannels;
    int sampleRate;
    int bitsPerSample;
    size_t dataSize;
    size_t bytesPerSample;
};

#define SWAP_16(num) (num>>8) | (num<<8)
#define SWAP_32(num) ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000

#ifdef PLATFORM_BIG_ENDIAN
static inline uint16_t to_le16(uint16_t num)
{
    return SWAP_16(num);
}

static inline uint32_t to_le32(uint32_t num)
{
    return SWAP_32(num);
}
#else
static inline uint16_t to_le16(uint16_t num)
{
    return num;
}

static inline uint32_t to_le32(uint32_t num)
{
    return num;
}
#endif

}

extern "C" {

g_id gsoundencoder_WavCreate(const char *fileName, int numChannels, int sampleRate, int bitsPerSample)
{
    FILE *fos = fopen(gpath_transform(fileName), "wb");
    if (fos == NULL)
        return 0;

    {
        GWaveHeader header;
        header.chunkID = to_le32(0x46464952)/*RIFF*/;
        header.chunkSize = 0;
        header.format = to_le32(0x45564157)/*WAVE*/;
        fwrite(&header, sizeof(header), 1, fos);
    }

    {
        uint32_t chunkId = to_le32(0x20746D66)/*fmt */;
        fwrite(&chunkId, sizeof(chunkId), 1, fos);

        uint32_t chunkSize = to_le32(sizeof(GFmtChunk));
        fwrite(&chunkSize, sizeof(chunkSize), 1, fos);

        GFmtChunk format;
        format.audioFormat = to_le16(1);
        format.numChannels = to_le16(numChannels);
        format.sampleRate = to_le32(sampleRate);
        format.bitsPerSample = to_le16(bitsPerSample);
        format.blockAlign = to_le16(((format.bitsPerSample + 7) / 8) * format.numChannels);
        format.byteRate = to_le32(format.blockAlign * format.sampleRate);
        fwrite(&format, sizeof(format), 1, fos);
    }

    {
        uint32_t chunkId = to_le32(0x61746164)/*data */;
        fwrite(&chunkId, sizeof(chunkId), 1, fos);

        uint32_t chunkSize = 0;
        fwrite(&chunkSize, sizeof(chunkSize), 1, fos);
    }

    GWavHandle *handle = new GWavHandle;
    handle->fos = fos;
    handle->numChannels = numChannels;
    handle->sampleRate = sampleRate;
    handle->bitsPerSample = bitsPerSample;
    handle->dataSize = 0;
    handle->bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;

    return (g_id)handle;
}

void gsoundencoder_WavClose(g_id id)
{
    GWavHandle *handle = (GWavHandle*)id;

    if (handle->dataSize == 0)
    {
        if (handle->bitsPerSample == 8)
        {
            unsigned char data[] = {128, 128};
            gsoundencoder_WavWrite(id, 1, data);
        }
        else if (handle->bitsPerSample == 16)
        {
            short data[] = {0, 0};
            gsoundencoder_WavWrite(id, 1, data);
        }
    }

    {
        fseek(handle->fos, 4, SEEK_SET);
        uint32_t chunkSize = to_le32(4 + (8 + sizeof(GFmtChunk)) + (8 + handle->dataSize));
        fwrite(&chunkSize, sizeof(chunkSize), 1, handle->fos);
    }

    {
        fseek(handle->fos, sizeof(GWaveHeader) + sizeof(GFmtChunk) + 12, SEEK_SET);
        uint32_t chunkSize = to_le32(handle->dataSize);
        fwrite(&chunkSize, sizeof(chunkSize), 1, handle->fos);
    }

    fclose(handle->fos);

    delete handle;
}

void gsoundencoder_WavWrite(g_id id, size_t sampleCount, void *data)
{
    GWavHandle *handle = (GWavHandle*)id;
    size_t size = sampleCount * handle->bytesPerSample;
    fwrite(data, size, 1, handle->fos);
    handle->dataSize += size;
}

}
