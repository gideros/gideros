// Waves.h: interface for the CWaves class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CWAVES_H_
#define _CWAVES_H_

//#include <windows.h>
#include <stdio.h>
#include <gfile.h>
#include <gstdio.h>

typedef unsigned short      WORD;
typedef unsigned int       DWORD;
//typedef unsigned long ULONG;
//typedef long long LONGLONG;
//typedef long LONG;
//typedef unsigned long long DWORDLONG;
//typedef void *PVOID;

typedef struct _GUID {
	unsigned int  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[ 8 ];
} GUID;

//typedef LONG HRESULT;
#define SUCCEEDED(hr) ((hr) >= 0)

#define MAX_NUM_WAVEID			1024

enum WAVEFILETYPE
{
	WF_EX  = 1,
	WF_EXT = 2
};

enum WAVERESULT
{
	WR_OK = 0,
	WR_INVALIDFILENAME					= - 1,
	WR_BADWAVEFILE						= - 2,
	WR_INVALIDPARAM						= - 3,
	WR_INVALIDWAVEID					= - 4,
	WR_NOTSUPPORTEDYET					= - 5,
	WR_WAVEMUSTBEMONO					= - 6,
	WR_WAVEMUSTBEWAVEFORMATPCM			= - 7,
	WR_WAVESMUSTHAVESAMEBITRESOLUTION	= - 8,
	WR_WAVESMUSTHAVESAMEFREQUENCY		= - 9,
	WR_WAVESMUSTHAVESAMEBITRATE			= -10,
	WR_WAVESMUSTHAVESAMEBLOCKALIGNMENT	= -11,
	WR_OFFSETOUTOFDATARANGE				= -12,
	WR_FILEERROR						= -13,
	WR_OUTOFMEMORY						= -14,
	WR_INVALIDSPEAKERPOS				= -15,
	WR_INVALIDWAVEFILETYPE				= -16,
	WR_NOTWAVEFORMATEXTENSIBLEFORMAT	= -17
};

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;
    WORD    nChannels;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
    WORD    wBitsPerSample;
    WORD    cbSize;
} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif // !_WAVEFORMATEXTENSIBLE_

typedef struct
{
	WAVEFILETYPE	wfType;
	WAVEFORMATEXTENSIBLE wfEXT;		// For non-WAVEFORMATEXTENSIBLE wavefiles, the header is stored in the Format member of wfEXT
	char			*pData;
	unsigned int	ulDataSize;
	G_FILE			*pFile;
	unsigned int	ulDataOffset;
} WAVEFILEINFO, *LPWAVEFILEINFO;

typedef int (/*__cdecl*/ *PFNALGETENUMVALUE)( const char *szEnumName );
typedef int	WAVEID;

class CWaves  
{
public:
	CWaves();
	virtual ~CWaves();

	WAVERESULT LoadWaveFile(const char *szFilename, WAVEID *WaveID);
	WAVERESULT OpenWaveFile(const char *szFilename, WAVEID *WaveID);
	WAVERESULT ReadWaveData(WAVEID WaveID, void *pData, unsigned long ulDataSize, unsigned long *pulBytesWritten);
	WAVERESULT SetWaveDataOffset(WAVEID WaveID, unsigned long ulOffset);
	WAVERESULT GetWaveDataOffset(WAVEID WaveID, unsigned long *pulOffset);
	WAVERESULT GetWaveType(WAVEID WaveID, WAVEFILETYPE *pwfType);
	WAVERESULT GetWaveFormatExHeader(WAVEID WaveID, WAVEFORMATEX *pWFEX);
	WAVERESULT GetWaveFormatExtensibleHeader(WAVEID WaveID, WAVEFORMATEXTENSIBLE *pWFEXT);
	WAVERESULT GetWaveData(WAVEID WaveID, void **ppAudioData);
	WAVERESULT GetWaveSize(WAVEID WaveID, unsigned long *pulDataSize);
	WAVERESULT GetWaveFrequency(WAVEID WaveID, unsigned long *pulFrequency);
	WAVERESULT GetWaveALBufferFormat(WAVEID WaveID, PFNALGETENUMVALUE pfnGetEnumValue, unsigned long *pulFormat);
	WAVERESULT DeleteWaveFile(WAVEID WaveID);

	char *GetErrorString(WAVERESULT wr, char *szErrorString, unsigned long nSizeOfErrorString);
	bool IsWaveID(WAVEID WaveID);

private:
	WAVERESULT ParseFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo);
	WAVEID InsertWaveID(LPWAVEFILEINFO pWaveFileInfo);
	
	LPWAVEFILEINFO	m_WaveIDs[MAX_NUM_WAVEID];
};

#endif // _CWAVES_H_
