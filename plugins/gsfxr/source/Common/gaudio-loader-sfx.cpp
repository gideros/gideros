#ifdef SFXR_EXTERNAL_TOOL
#include <stdio.h>
#define G_FILE FILE
#define g_fopen fopen
#define g_ftell ftell
#define g_fread fread
#define g_fclose fclose
#define g_fseek fseek
#define GAUDIO_NO_ERROR 0
#define GAUDIO_CANNOT_OPEN_FILE -1
#define GAUDIO_UNRECOGNIZED_FORMAT -2
#define GAUDIO_ERROR_WHILE_READING -3
#define GAUDIO_UNSUPPORTED_FORMAT -4
#define g_id unsigned int	// wont work for 64bit
#define gaudio_Error int
using namespace std;
#include <iostream>
#define LOG1(a) cout << a << "\n";
#define LOG2(a,b) cout << a << b << "\n";
#define LOG3(a,b,c) cout << a << b << c << "\n";
#else
#include <gideros.h>
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

#include <gaudio.h>
#include <ggaudiomanager.h>
#include <gstdio.h>
#include <gaudio.h>
#define LOG1(a) //qDebug() << a << "\n";
#define LOG2(a,b) //qDebug() << a << b << "\n";
#define LOG3(a,b,c) //qDebug() << a << b << c << "\n";
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include "retrosfxvoice.h"

#ifdef QT_CORE_LIB
#include <QDebug>
#endif

struct GGSFXHandle
{
	int nBytesRead;
	RetroSFXVoice *pSFXVoice;
};
static std::map<g_id, GGSFXHandle *> ctxmap;

extern "C" {

g_id gaudio_SFXOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error)
{
    GGSFXHandle *handle = new GGSFXHandle();

	handle->nBytesRead = 0;
	handle->pSFXVoice = new RetroSFXVoice();

	if (handle->pSFXVoice->LoadSettings(fileName) == false)
	{
		if (error)
		{
			*error = GAUDIO_ERROR_WHILE_READING;
			delete handle;
			return 0;
		}
	}

	handle->pSFXVoice->Play();

	if (numChannels)
	{
		*numChannels = 1;
		LOG2("gaudio_SFXOpen numChannels ", *numChannels);
	}

	if (sampleRate)
	{
		*sampleRate = 44100;
		LOG2("gaudio_SFXOpen sampleRate ", *sampleRate);
	}

	if (bitsPerSample)
	{
		*bitsPerSample = 16;
		LOG2("gaudio_SFXOpen bitsPerSample ", *bitsPerSample);
	}

	if (numSamples)
	{
		*numSamples = handle->pSFXVoice->GetVoiceLengthInSamples();
		LOG2("gaudio_SFXOpen numSamples ", *numSamples);
	}

	if (error)
	{
		*error = GAUDIO_NO_ERROR;
		LOG2("gaudio_SFXOpen error ", *error);
	}

    g_id gid = g_NextId();
    ctxmap[gid]=handle;

    return gid;
}

void gaudio_SFXClose(g_id id)
{
	LOG1("gaudio_SFXClose");
    GGSFXHandle *handle = ctxmap[id];
    ctxmap.erase(id);
	if (handle)
	{
		if (handle->pSFXVoice)
		{
			delete handle->pSFXVoice;
		}
		delete handle;
	}
}

int gaudio_SFXSeek(g_id id, long int offset, int whence)
{
    GGSFXHandle *handle = ctxmap[id];

	LOG3("SEEK_SET:", offset, whence);

	if ( (offset == 0) && (whence == SEEK_SET) )
	{
		LOG2("SEEK_SET:", offset);
		handle->pSFXVoice->Play();
		handle->nBytesRead = 0;
	}
	else if (whence == SEEK_CUR)
	{
		LOG2("SEEK_CUR:", offset);
		handle->nBytesRead += offset;
	}
	else if ((offset == 0) && (whence == SEEK_END))
	{
		LOG2("SEEK_END:", offset);
		handle->pSFXVoice->Play();
		handle->nBytesRead = handle->pSFXVoice->GetVoiceLengthInSamples()*2;
	}

	return handle->nBytesRead;
}

long int gaudio_SFXTell(g_id id)
{
    GGSFXHandle *handle = ctxmap[id];
	LOG2("gaudio_SFXTell ", handle->nBytesRead);
	return handle->nBytesRead;
}

size_t gaudio_SFXRead(g_id id, size_t size, void *data)
{
    GGSFXHandle *handle = ctxmap[id];

	LOG2("gaudio_SFXRead ", size);

	// clamp the size we will be rendering
	int nActualSize = handle->pSFXVoice->GetVoiceLengthInSamples()*2;
	if ((handle->nBytesRead + size) > nActualSize)
	{
		size = (nActualSize - handle->nBytesRead);
		LOG2("gaudio_SFXRead ClampSize ", size);
	}

	memset(data, 0, size);

	// i hope size is a multiple of two here....
	int nLocalBytesRead = handle->pSFXVoice->Render(size / 2, (short*)data) * 2;
	handle->nBytesRead += nLocalBytesRead;
	
	LOG2("gaudio_SFXRead nLocalBytesRead ", nLocalBytesRead);
	return nLocalBytesRead;
}

}

#ifndef SFXR_EXTERNAL_TOOL
GGAudioLoader audioSfx(gaudio_SFXOpen, gaudio_SFXClose, gaudio_SFXRead, gaudio_SFXSeek, gaudio_SFXTell);

static void g_initializePlugin(lua_State *L)
{
    gaudio_registerType("sfx",audioSfx);
}

static void g_deinitializePlugin(lua_State *L)
{
	gaudio_unregisterType("sfx");
}

#if (!defined(QT_NO_DEBUG)) && (defined(TARGET_OS_MAC) || defined(_MSC_VER)  || defined(TARGET_OS_OSX))
REGISTER_PLUGIN_STATICNAMED_CPP("GSFXR", "1.0",Gsfxr)
#else
REGISTER_PLUGIN_NAMED("GSFXR", "1.0", Gsfxr)
#endif

#endif
