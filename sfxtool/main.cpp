/*

   Porting: Search for WIN32 to find windows-specific code
            snippets which need to be replaced or removed.

*/

#undef WIN32// WARN_PC PORTED BACK FROM LINUX SO WIN32 STUFF IS BORKED
#define _CRT_SECURE_NO_WARNINGS

#include "sdlkit.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SDL.h"

#include "fileio.h"

//////////////////////////////////////////////////////////////////////////

#include "RetroSFXVoice.h"

RetroSFXVoice g_RetroSFXVoice;
RetroSFXVoice g_VerifySaveVoice;

//////////////////////////////////////////////////////////////////////////

int vcurbutton = -1;
float* vselected = NULL;

//////////////////////////////////////////////////////////////////////////

extern "C" {
#define g_id unsigned int	// wont work for 64bit
#define gaudio_Error int

	extern g_id gaudio_SFXOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error);
	extern void gaudio_SFXClose(g_id id);
	extern int gaudio_SFXSeek(g_id id, long int offset, int whence);
	extern long int gaudio_SFXTell(g_id id);
	extern size_t gaudio_SFXRead(g_id id, size_t size, void *data);
};

//////////////////////////////////////////////////////////////////////////

#define rnd(n) (rand()%(n+1))

#define PI 3.14159265f

float frnd(float range)
{
	return (float)rnd(10000)/10000*range;
}

struct Spriteset
{
	DWORD *data;
	int width;
	int height;
	int pitch;
};

Spriteset font;

struct Category
{
	char name[32];
};

Category categories[10];

DPInput *input;
bool mute_stream;

//////////////////////////////////////////////////////////////////////////

int wav_bits = 16;
int wav_freq = 44100;

//////////////////////////////////////////////////////////////////////////

void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file)
{
    unsigned buf;
    while(num_bytes>0)
    {   buf = word & 0xff;
        fwrite(&buf, 1,1, wav_file);
        num_bytes--;
		word >>= 8;
    }
}
 
/* information about the WAV file format from
 
http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 
 */
 
void write_wav(char * filename, unsigned long num_samples, short int * data, int s_rate)
{
    FILE* wav_file;
    unsigned int sample_rate;
    unsigned int num_channels;
    unsigned int bytes_per_sample;
    unsigned int byte_rate;
    unsigned long i;    /* counter for samples */
 
    num_channels = 1;   /* monoaural */
    bytes_per_sample = 2;
 
    if (s_rate<=0) sample_rate = 44100;
    else sample_rate = (unsigned int) s_rate;
 
    byte_rate = sample_rate*num_channels*bytes_per_sample;
 
    wav_file = fopen(filename, "wb");
//	assert(wav_file);   /* make sure it opened */
	if (wav_file == 0)
	{
		return;
	}

    /* write RIFF header */
    fwrite("RIFF", 1, 4, wav_file);
    write_little_endian(36 + bytes_per_sample* num_samples*num_channels, 4, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
 
    /* write fmt  subchunk */
    fwrite("fmt ", 1, 4, wav_file);
    write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
    write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
    write_little_endian(num_channels, 2, wav_file);
    write_little_endian(sample_rate, 4, wav_file);
    write_little_endian(byte_rate, 4, wav_file);
    write_little_endian(num_channels*bytes_per_sample, 2, wav_file);  /* block align */
    write_little_endian(8*bytes_per_sample, 2, wav_file);  /* bits/sample */
 
    /* write data subchunk */
    fwrite("data", 1, 4, wav_file);
    write_little_endian(bytes_per_sample* num_samples*num_channels, 4, wav_file);
    for (i=0; i< num_samples; i++)
    {   write_little_endian((unsigned int)(data[i]),bytes_per_sample, wav_file);
    }
 
    fclose(wav_file);
}

//////////////////////////////////////////////////////////////////////////

bool ExportWAV(char* filename)
{
	int nRetroVoiceLength = g_RetroSFXVoice.GetVoiceLengthInSamples();

	short *pBuffer = (short*)malloc(nRetroVoiceLength*2);
	memset(pBuffer, 0, nRetroVoiceLength*2);

	int nRetroVoicePos=0;
	g_RetroSFXVoice.Play();
	while(nRetroVoicePos < nRetroVoiceLength)
	{
		nRetroVoicePos += g_RetroSFXVoice.Render(nRetroVoiceLength, &pBuffer[nRetroVoicePos]);
	}

	/*for (int i=0;i<nRetroVoiceLength;i++)
	{
		pBuffer[i]*=4;
	}*/

	write_wav(filename, nRetroVoiceLength, pBuffer, wav_freq);

	/*FILE *fp;
	fp = fopen("test.raw", "wb");
	fwrite(pBuffer, nRetroVoiceLength*2, 1, fp);
	fclose(fp);*/

	free(pBuffer);

	return true;
}

//////////////////////////////////////////////////////////////////////////

#define FBUFFSIZE (44100*10)
float fbuf[FBUFFSIZE];

static void SDLAudioCallback(void *userdata, Uint8 *stream, int len)
{
	memset(stream, 0, sizeof(len));
	if (g_RetroSFXVoice.IsActive() && !mute_stream)
	{
		len/=2;
//		ASSERT(l<FBUFFSIZE);
		g_RetroSFXVoice.Render(len, (short*)stream);
	}
}

#include "tools.h"

bool firstframe=true;
int refresh_counter=0;

void Slider(int x, int y, float& value, bool bipolar, const char* text, float *fClone=NULL)
{
	if(MouseInBox(x, y, 100, 10))
	{
		if(mouse_leftclick)
			vselected=&value;
		if(mouse_rightclick)
		{
			if (fClone)
			{
				value = *fClone;
			}
			else
			{
				value=0.0f;
			}
		}
	}
	float mv=(float)(mouse_x-mouse_px);
	if(vselected!=&value)
		mv=0.0f;
	if(bipolar)
	{
		value+=mv*0.005f;
		if(value<-1.0f) value=-1.0f;
		if(value>1.0f) value=1.0f;
	}
	else
	{
		value+=mv*0.0025f;
		if(value<0.0f) value=0.0f;
		if(value>1.0f) value=1.0f;
	}
	DrawBar(x-1, y, 102, 10, 0x000000);
	int ival=(int)(value*99);
	if(bipolar)
		ival=(int)(value*49.5f+49.5f);
	DrawBar(x, y+1, ival, 8, 0xF0C090);
	DrawBar(x+ival, y+1, 100-ival, 8, 0x807060);
	DrawBar(x+ival, y+1, 1, 8, 0xFFFFFF);
	if(bipolar)
	{
		DrawBar(x+50, y-1, 1, 3, 0x000000);
		DrawBar(x+50, y+8, 1, 3, 0x000000);
	}
	DWORD tcol=0xffffff;
	if(g_RetroSFXVoice.m_Voice.nWaveformType!=0 && (&value==&g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty || &value==&g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp))
		tcol=0x808080;
	DrawText(x-4-strlen(text)*8, y+1, tcol, text);
}

bool Button(int x, int y, bool highlight, const char* text, int id)
{
	DWORD color1=0x000000;
	DWORD color2=0x808080;
	DWORD color3=0x000000;
	bool hover=MouseInBox(x, y, 100, 17);
	if(hover && mouse_leftclick)
		vcurbutton=id;
	bool current=(vcurbutton==id);
	if(highlight)
	{
		color1=0x808080;
		color2=0x988070;
		color3=0xffffff;
	}
	if(current && hover)
	{
		color1=0xffffff;
		color2=0xFFF0E0;
		color3=0x808080;
	}
	DrawBar(x-1, y-1, 102, 19, color1);
	DrawBar(x, y, 100, 17, color2);
	DrawText(x+5, y+5, color3, text);
	if(current && hover && !mouse_left)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////

void DoRandomize()
{
	g_RetroSFXVoice.m_Voice.nWaveformType = (rand()>>10)&3;

	g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=pow(frnd(1.0f), 2.0f);
	if(rnd(1))
	{
		g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=pow(frnd(1.0f), 3.0f)+0.5f;
	}
	g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit=0.0f;
	g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=pow(frnd(1.0f), 5.0f);
	if(g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq>0.7f && g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp>0.2f)
	{
		g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp;
	}
	if(g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq<0.2f && g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp<-0.05f)
	{
		g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp;
	}
	g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqDRamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fVibDelay=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=pow(frnd(1.0f), 2.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=((frnd(1.0f))/2.0f)+0.5f;;
	g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch=pow(frnd(0.8f), 2.0f);
	if(g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack+g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain+g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay<0.2f)
	{
		g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain+=0.2f+frnd(0.3f);
		g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay+=0.2f+frnd(0.3f);
	}
	g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFResonance=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq=1.0f-pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp=pow(frnd(1.0f), 3.0f);
	if(g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq<0.1f && g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp<-0.05f)
	{
		g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp=-g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp;
	}
	g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq=pow(frnd(1.0f), 5.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFRamp=pow(frnd(1.0f), 5.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod=frnd(1.0f);

	//////////////////////////////////////////////////////////////////////////
	// for morph

	if (g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed==0.0f)
	{
		// If we don't have a repeat then we don't need a morph
		g_RetroSFXVoice.m_Voice.fMorphRate = 0.0f;
	}
	else 
	{
		g_RetroSFXVoice.m_Voice.fMorphRate=frnd(2.0f);
		g_RetroSFXVoice.m_Voice.fMorphRate -= 1.0f;
		if (g_RetroSFXVoice.m_Voice.fMorphRate<0.0f)
		{
			// 50% chance of morph
			g_RetroSFXVoice.m_Voice.fMorphRate=0.0f;
		}
	}

	g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq=pow(frnd(1.0f), 2.0f);
	if(rnd(1))
	{
		g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq=pow(frnd(1.0f), 3.0f)+0.5f;
	}
	g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqLimit=0.0f;
	g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp=pow(frnd(1.0f), 5.0f);
	if(g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq>0.7f && g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp>0.2f)
	{
		g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp=-g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp;
	}
	if(g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq<0.2f && g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp<-0.05f)
	{
		g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp=-g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp;
	}
	g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqDRamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fDuty=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fDutyRamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fVibStrength=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fVibSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fVibDelay=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvAttack=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvSustain=pow(frnd(1.0f), 2.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvDecay=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvPunch=pow(frnd(0.8f), 2.0f);
	if(g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvAttack+g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvSustain+g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvDecay<0.2f)
	{
			g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvSustain+=0.2f+frnd(0.3f);
			g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvDecay+=0.2f+frnd(0.3f);
	}
	g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFResonance=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFFreq=1.0f-pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp=pow(frnd(1.0f), 3.0f);
	if(g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFFreq<0.1f && g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp<-0.05f)
	{
		g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp=-g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp;
	}
	g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFFreq=pow(frnd(1.0f), 5.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFRamp=pow(frnd(1.0f), 5.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fPHAOffset=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fPHARamp=pow(frnd(1.0f), 3.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fRepeatSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fArmSpeed=frnd(1.0f);
	g_RetroSFXVoice.m_Voice.FXMorphParams.fArmMod=frnd(1.0f);
}

//////////////////////////////////////////////////////////////////////////

int drawcount=0;

//////////////////////////////////////////////////////////////////////////

void DrawScreen()
{
	bool redraw=true;
	if(!firstframe && mouse_x-mouse_px==0 && mouse_y-mouse_py==0 && !mouse_left && !mouse_right)
		redraw=false;
	if(!mouse_left)
	{
		if(vselected!=NULL || vcurbutton>-1)
		{
			redraw=true;
			refresh_counter=2;
		}
		vselected=NULL;
	}
	if(refresh_counter>0)
	{
		refresh_counter--;
		redraw=true;
	}

	if(g_RetroSFXVoice.m_bPlayingSample)
		redraw=true;

	if(drawcount++>20)
	{
		redraw=true;
		drawcount=0;
	}

	if(!redraw)
		return;

	firstframe=false;

	ddkLock();

	ClearScreen(0x404040);

	DrawText(10, 10, 0xffffff, "GENERATOR");
	for(int i=0;i<8;i++)
	{
		if(Button(5, 35+i*30, false, categories[i].name, 300+i))
		{
			switch(i)
			{
			case 0: // pickup/coin
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.4f+frnd(0.5f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=frnd(0.1f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=0.1f+frnd(0.4f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch=0.3f+frnd(0.3f);
				if(rnd(1))
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed=0.5f+frnd(0.2f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod=0.2f+frnd(0.4f);
				}
				break;
			case 1: // laser/shoot
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.nWaveformType=rnd(2);
				if(g_RetroSFXVoice.m_Voice.nWaveformType==2 && rnd(1))
					g_RetroSFXVoice.m_Voice.nWaveformType=rnd(1);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.5f+frnd(0.5f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit=g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq-0.2f-frnd(0.6f);
				if(g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit<0.2f) g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit=0.2f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-0.15f-frnd(0.2f);
				if(rnd(2)==0)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.3f+frnd(0.6f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit=frnd(0.1f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-0.35f-frnd(0.3f);
				}
				if(rnd(1))
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(0.5f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp=frnd(0.2f);
				}
				else
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=0.4f+frnd(0.5f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp=-frnd(0.7f);
				}
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=0.1f+frnd(0.2f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=frnd(0.4f);
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch=frnd(0.3f);
				if(rnd(2)==0)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset=frnd(0.2f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp=-frnd(0.2f);
				}
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq=frnd(0.3f);
				break;
			case 2: // explosion
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.nWaveformType=3;
				if(rnd(1))
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.1f+frnd(0.4f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-0.1f+frnd(0.4f);
				}
				else
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.2f+frnd(0.7f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-0.2f-frnd(0.2f);
				}
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq*=g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq;
				if(rnd(4)==0)
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=0.0f;
				if(rnd(2)==0)
					g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed=0.3f+frnd(0.5f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=0.1f+frnd(0.3f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=frnd(0.5f);
				if(rnd(1)==0)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset=-0.3f+frnd(0.9f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp=-frnd(0.3f);
				}
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch=0.2f+frnd(0.6f);
				if(rnd(1))
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength=frnd(0.7f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed=frnd(0.6f);
				}
				if(rnd(2)==0)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed=0.6f+frnd(0.3f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod=0.8f-frnd(1.6f);
				}
				break;
			case 3: // powerup
				g_RetroSFXVoice.ResetParams();
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.nWaveformType=1;
				else
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(0.6f);
				if(rnd(1))
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.2f+frnd(0.3f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=0.1f+frnd(0.4f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed=0.4f+frnd(0.4f);
				}
				else
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.2f+frnd(0.3f);
					g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=0.05f+frnd(0.2f);
					if(rnd(1))
					{
						g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength=frnd(0.7f);
						g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed=frnd(0.6f);
					}
				}
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=frnd(0.4f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=0.1f+frnd(0.4f);
				break;
			case 4: // hit/hurt
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.nWaveformType=rnd(2);
				if(g_RetroSFXVoice.m_Voice.nWaveformType==2)
					g_RetroSFXVoice.m_Voice.nWaveformType=3;
				if(g_RetroSFXVoice.m_Voice.nWaveformType==0)
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(0.6f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.2f+frnd(0.6f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=-0.3f-frnd(0.4f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=frnd(0.1f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=0.1f+frnd(0.2f);
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq=frnd(0.3f);
				break;
			case 5: // jump
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.nWaveformType=0;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(0.6f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.3f+frnd(0.3f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp=0.1f+frnd(0.2f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=0.1f+frnd(0.3f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=0.1f+frnd(0.2f);
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq=frnd(0.3f);
				if(rnd(1))
					g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq=1.0f-frnd(0.6f);
				break;
			case 6: // blip/select
				g_RetroSFXVoice.ResetParams();
				g_RetroSFXVoice.m_Voice.nWaveformType=rnd(1);
				if(g_RetroSFXVoice.m_Voice.nWaveformType==0)
					g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty=frnd(0.6f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq=0.2f+frnd(0.4f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack=0.0f;
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain=0.1f+frnd(0.1f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay=frnd(0.2f);
				g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq=0.1f;
				break;
			case 7:	// robotron
				DoRandomize();
				if (g_RetroSFXVoice.m_Voice.fMorphRate < 0.25f)
				{
					g_RetroSFXVoice.m_Voice.fMorphRate = 0.25f;
				}
				if (g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain < 0.5f)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain = 0.5f;
				}
				if (g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay < 0.5f)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay = 0.5f;
				}
				if (g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed<=0.5f)
				{
					g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed=0.5f;
				}
				break;
			default:
				break;
			}

			g_RetroSFXVoice.Play(true);
		}
	}
	DrawBar(110, 0, 2, 600, 0x000000);
	DrawText(120, 10, 0xffffff, "MANUAL SETTINGS");

	if(Button(130, 30, g_RetroSFXVoice.m_Voice.nWaveformType==0, "SQUAREWAVE", 10))
		g_RetroSFXVoice.m_Voice.nWaveformType=0;
	if(Button(250, 30, g_RetroSFXVoice.m_Voice.nWaveformType==1, "SAWTOOTH", 11))
		g_RetroSFXVoice.m_Voice.nWaveformType=1;
	if(Button(370, 30, g_RetroSFXVoice.m_Voice.nWaveformType==2, "SINEWAVE", 12))
		g_RetroSFXVoice.m_Voice.nWaveformType=2;
	if(Button(490, 30, g_RetroSFXVoice.m_Voice.nWaveformType==3, "NOISE", 13))
		g_RetroSFXVoice.m_Voice.nWaveformType=3;

	bool do_play=false;

	DrawBar(5-1-1, 412-1-1, 102+2, 19+2, 0x000000);
	if(Button(5, 412, false, "RANDOMIZE", 40))
	{
		DoRandomize();
		do_play=true;
	}

	if(Button(5, 382, false, "MUTATE", 30))
	{
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqDRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fVibDelay+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFResonance+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod+=frnd(0.1f)-0.05f;

		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqDRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fDuty+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fDutyRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fVibStrength+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fVibSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fVibDelay+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvAttack+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvSustain+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvDecay+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvPunch+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFResonance+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFFreq+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFRamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fPHAOffset+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fPHARamp+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fRepeatSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fArmSpeed+=frnd(0.1f)-0.05f;
		if(rnd(1)) g_RetroSFXVoice.m_Voice.FXMorphParams.fArmMod+=frnd(0.1f)-0.05f;

		do_play=true;
	}

	DrawText(400+515, 170, 0x000000, "VOLUME");
	DrawBar(400+490-1-1+60, 180-1+5, 70, 2, 0x000000);
	DrawBar(400+490-1-1+60+68, 180-1+5, 2, 205, 0x000000);
	DrawBar(400+490-1-1+60, 180-1, 42+2, 10+2, 0xFF0000);
	Slider(400+490, 180, g_RetroSFXVoice.m_Voice.fSoundVol, false, " ");
	if(Button(400+490, 200, false, "PLAY SOUND", 20))
		g_RetroSFXVoice.Play(true);

	if(Button(400+490, 290, false, "LOAD SOUND", 14))
	{
		char filename[256];
		if(FileSelectorLoadRFX(filename)) // WIN32
		{
			g_RetroSFXVoice.ResetParams();
			g_RetroSFXVoice.LoadSettings(filename);
			int numChannels;
			int sampleRate;
			int bitsPerSample;
			int numSamples;
			gaudio_Error error;

			if (g_id fp = gaudio_SFXOpen(filename, &numChannels, &sampleRate, &bitsPerSample, &numSamples, &error))
			{
				int bytesread = 0;
				gaudio_SFXSeek(fp, 0, SEEK_END);
				int nFileSize = gaudio_SFXTell(fp);
				gaudio_SFXSeek(fp, 0, SEEK_SET);
				gaudio_SFXTell(fp);
				unsigned char *ptr = (unsigned char *)malloc(nFileSize);
				while (bytesread < nFileSize)
				{
					bytesread += gaudio_SFXRead(fp, 16384, &ptr[bytesread]);
				}
				int nSize = nFileSize;
				gaudio_SFXClose(fp);

				write_wav("testing.wav", nFileSize/2, (short*)ptr, sampleRate);

				free(ptr);
			}
//			g_RetroSFXVoice.LoadSettings("Settings.rfx");
			g_RetroSFXVoice.Play();
		}
	}

	// button doesn't appear unless played once all the way through to calculate length...
	if (g_RetroSFXVoice.GetVoiceLengthInSamples() != 0)
	{
		if (Button(400 + 490, 320, false, "SAVE SOUND", 15))
		{
			char filename[256];
			if (FileSelectorSaveRFX(filename)) // WIN32
			{
				g_RetroSFXVoice.SaveSettings(filename);
				// load it back in
				g_VerifySaveVoice.LoadSettings(filename);
				// verify it
				g_RetroSFXVoice.CompareSettings(&g_VerifySaveVoice);
				//g_RetroSFXVoice.SaveSettings("Settings.rfx");
			}
		}
	}

	DrawBar(400+490-1-1+60, 380-1+9, 70, 2, 0x000000);
	DrawBar(400+490-1-2, 380-1-2, 102+4, 19+4, 0x000000);
	if (g_RetroSFXVoice.GetVoiceLengthInSamples() != 0)
	{
		if (Button(400 + 490, 380, false, "EXPORT .WAV", 16))
		{
			char filename[256];
			if (FileSelectorSaveWAV(filename)) // WIN32
			{
				mute_stream = true;
				ExportWAV(filename);
				//			ExportWAV("render.wav");
				mute_stream = false;
			}
		}
	}
	char str[10];
	sprintf(str, "%i HZ", wav_freq);
	if(Button(400+490, 410, false, str, 18))
	{
		if(wav_freq==44100)
			wav_freq=22050;
		else
			wav_freq=44100;
	}
	sprintf(str, "%i-BIT", wav_bits);
	if(Button(400+490, 440, false, str, 19))
	{
		if(wav_bits==16)
			wav_bits=8;
		else
			wav_bits=16;
	}

	//////////////////////////////////////////////////////////////////////////

	int ypos=4;

	int xpos=350;

	// Left side

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvAttack, false, "ATTACK TIME");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvSustain, false, "SUSTAIN TIME");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvPunch, false, "SUSTAIN PUNCH");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fEnvDecay, false, "DECAY TIME");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq, false, "START FREQUENCY");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit, false, "MIN FREQUENCY");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp, true, "SLIDE");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqDRamp, true, "DELTA SLIDE");

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength, false, "VIBRATO DEPTH");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed, false, "VIBRATO SPEED");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod, true, "CHANGE AMOUNT");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed, false, "CHANGE SPEED");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty, false, "SQUARE DUTY");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp, true, "DUTY SWEEP");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed, false, "REPEAT SPEED");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset, true, "PHASER OFFSET");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp, true, "PHASER SWEEP");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq, false, "LP FILTER CUTOFF");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp, true, "LP FILTER CUTOFF SWEEP");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFResonance, false, "LP FILTER RESONANCE");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq, false, "HP FILTER CUTOFF");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFRamp, true, "HP FILTER CUTOFF SWEEP");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	DrawBar(xpos-190, 4*18-5, 1, (ypos-4)*18, 0x0000000);
	DrawBar(xpos-190+299, 4*18-5, 1, (ypos-4)*18, 0x0000000);

	//////////////////////////////////////////////////////////////////////////

//	DrawBar(xpos, ypos*18, 300, 2, 0x0000000);

	//////////////////////////////////////////////////////////////////////////

	Slider(xpos+100, (ypos+2)*18, g_RetroSFXVoice.m_Voice.fMorphRate, false, "MORPH RATE");

	//////////////////////////////////////////////////////////////////////////
	// Right side

	ypos = 4;
	xpos += 300;

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvAttack, false, "ATTACK TIME");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvSustain, false, "SUSTAIN TIME");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvPunch, false, "SUSTAIN PUNCH");
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fEnvDecay, false, "DECAY TIME");

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fBaseFreq, false, "END FREQUENCY",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fBaseFreq);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqLimit, false, "MIN FREQUENCY",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqLimit);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqRamp, true, "SLIDE",				&g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqRamp);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fFreqDRamp, true, "DELTA SLIDE",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fFreqDRamp);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fVibStrength, false, "VIBRATO DEPTH",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fVibStrength);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fVibSpeed, false, "VIBRATO SPEED",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fVibSpeed);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fArmMod, true, "CHANGE AMOUNT",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fArmMod);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fArmSpeed, false, "CHANGE SPEED",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fArmSpeed);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fDuty, false, "SQUARE DUTY",			&g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fDutyRamp, true, "DUTY SWEEP",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fRepeatSpeed, false, "REPEAT SPEED",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fRepeatSpeed);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fPHAOffset, true, "PHASER OFFSET",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fPHAOffset);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fPHARamp, true, "PHASER SWEEP",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fPHARamp);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFFreq, false, "LP FILTER CUTOFF",			&g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFFreq);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFRamp, true, "LP FILTER CUTOFF SWEEP",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFRamp);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fLPFResonance, false, "LP FILTER RESONANCE",	&g_RetroSFXVoice.m_Voice.FXBaseParams.fLPFResonance);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFFreq, false, "HP FILTER CUTOFF",			&g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFFreq);
	Slider(xpos, (ypos++)*18, g_RetroSFXVoice.m_Voice.FXMorphParams.fHPFRamp, true, "HP FILTER CUTOFF SWEEP",		&g_RetroSFXVoice.m_Voice.FXBaseParams.fHPFRamp);

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	DrawBar(xpos-190, 4*18-5, 1, (ypos-4)*18, 0x0000000);
	DrawBar(xpos-190+299, 4*18-5, 1, (ypos-4)*18, 0x0000000);

	//////////////////////////////////////////////////////////////////////////

	if(do_play)
	{
		g_RetroSFXVoice.Play(true);
	}

	ddkUnlock();

	if(!mouse_left)
		vcurbutton=-1;
}

bool keydown=false;

bool ddkCalcFrame()
{
	input->Update(); // WIN32 (for keyboard input)

	if(input->KeyPressed(DIK_SPACE) || input->KeyPressed(DIK_RETURN)) // WIN32 (keyboard input only for convenience, ok to remove)
	{
		if(!keydown)
		{
			g_RetroSFXVoice.Play(true);
			keydown=true;
		}
	}
	else
		keydown=false;

	DrawScreen();

	Sleep(5); // WIN32
	return true;
}

void ddkInit()
{
	srand(time(NULL));

	ddkSetMode(1024,600, 32, 60, DDK_WINDOW, "sfxr"); // requests window size etc from ddrawkit

	if (LoadTGA(font, "/usr/share/sfxr/font.tga")) {
        	/* Try again in cwd */
		if (LoadTGA(font, "font.tga")) {
			fprintf(stderr,
				"Error could not open /usr/share/sfxr/font.tga"
				" nor font.tga\n");
			exit(1);
		}
	}

	input=new DPInput(hWndMain, hInstanceMain); // WIN32

	strcpy(categories[0].name, "PICKUP/COIN");
	strcpy(categories[1].name, "LASER/SHOOT");
	strcpy(categories[2].name, "EXPLOSION");
	strcpy(categories[3].name, "POWERUP");
	strcpy(categories[4].name, "HIT/HURT");
	strcpy(categories[5].name, "JUMP");
	strcpy(categories[6].name, "BLIP/SELECT");
	strcpy(categories[7].name, "ROBOTRON");

	g_RetroSFXVoice.ResetParams();

	SDL_AudioSpec des;
	des.freq = 44100;
	des.format = AUDIO_S16SYS;
	des.channels = 1;
	des.samples = 512;
	des.callback = SDLAudioCallback;
	des.userdata = NULL;
	VERIFY(!SDL_OpenAudio(&des, NULL));
	SDL_PauseAudio(0);
}

void ddkFree()
{
	delete input;
	free(font.data);
	}

