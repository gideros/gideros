
// This code was originally part of a program called SFXR written by Dr Petter Circa 2007.
// http://www.drpetter.se/project_sfxr.html
// I took this code and modified it somewhat for Gilderos procedural sound effects. I also added
// a new sfx type so that sfx can morph between two sets of settings and produce evolving
// sfx that sound very much like a game called Robotron.
// Using this system means each sound effect is around 200 bytes

// The original code was released under the MIT license, and the modifications that I have made
// are also under the same license.
//
// Paul Carter 2018

//////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "retrosfxvoice.h"

#ifdef SFXR_EXTERNAL_TOOL
#include <stdio.h>
#include <stdlib.h>
#define G_FILE FILE
#define g_fopen fopen
#define g_ftell ftell
#define g_fread fread
#define g_fwrite fwrite
#define g_fclose fclose
#define g_fseek fseek
#define GAUDIO_NO_ERROR 0
#define GAUDIO_CANNOT_OPEN_FILE -1
#define GAUDIO_UNRECOGNIZED_FORMAT -2
#define GAUDIO_ERROR_WHILE_READING -3
#define GAUDIO_UNSUPPORTED_FORMAT -4
#define g_id unsigned int	// wont work for 64bit
#define gaudio_Error int
#else
#include <gstdio.h>
#include <gaudio.h>
#endif

#include <math.h>
#include <string.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////

#define rnd(n) (rand()%(n+1))

#define PI 3.14159265f

//////////////////////////////////////////////////////////////////////////

RetroSFXVoice::RetroSFXVoice()
{
	m_fMasterVol = 0.05f;
	m_Voice.fSoundVol = 0.5f;
	m_bPlayingSample = false;
	m_WavSamplesRendered = 0;

	ResetParams();
}

//////////////////////////////////////////////////////////////////////////

RetroSFXVoice::~RetroSFXVoice()
{

}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::ResetParams()
{
	m_Voice.nWaveformType=0;

	m_Voice.FXBaseParams.fBaseFreq=0.3f;
	m_Voice.FXBaseParams.fFreqLimit=0.0f;
	m_Voice.FXBaseParams.fFreqRamp=0.0f;
	m_Voice.FXBaseParams.fFreqDRamp=0.0f;
	m_Voice.FXBaseParams.fDuty=0.0f;
	m_Voice.FXBaseParams.fDutyRamp=0.0f;

	m_Voice.FXBaseParams.fVibStrength=0.0f;
	m_Voice.FXBaseParams.fVibSpeed=0.0f;
	m_Voice.FXBaseParams.fVibDelay=0.0f;

	m_Voice.FXBaseParams.fEnvAttack=0.0f;
	m_Voice.FXBaseParams.fEnvSustain=0.3f;
	m_Voice.FXBaseParams.fEnvDecay=0.4f;
	m_Voice.FXBaseParams.fEnvPunch=0.0f;

	m_Voice.FXBaseParams.fLPFResonance=0.0f;
	m_Voice.FXBaseParams.fLPFFreq=1.0f;
	m_Voice.FXBaseParams.fLPFRamp=0.0f;
	m_Voice.FXBaseParams.fHPFFreq=0.0f;
	m_Voice.FXBaseParams.fHPFRamp=0.0f;

	m_Voice.FXBaseParams.fPHAOffset=0.0f;
	m_Voice.FXBaseParams.fPHARamp=0.0f;

	m_Voice.FXBaseParams.fRepeatSpeed=0.0f;

	m_Voice.FXBaseParams.fArmSpeed=1.0f;
	m_Voice.FXBaseParams.fArmMod=0.0f;

	//////////////////////////////////////////////////////////////////////////

	m_Voice.fMorphRate = 0.0f;

	//////////////////////////////////////////////////////////////////////////

	m_Voice.FXMorphParams.fBaseFreq=0.3f;
	m_Voice.FXMorphParams.fFreqLimit=0.0f;
	m_Voice.FXMorphParams.fFreqRamp=0.0f;
	m_Voice.FXMorphParams.fFreqDRamp=0.0f;
	m_Voice.FXMorphParams.fDuty=0.0f;
	m_Voice.FXMorphParams.fDutyRamp=0.0f;

	m_Voice.FXMorphParams.fVibStrength=0.0f;
	m_Voice.FXMorphParams.fVibSpeed=0.0f;
	m_Voice.FXMorphParams.fVibDelay=0.0f;

//	m_morph_env_attack=0.0f;
//	m_morph_env_sustain=0.3f;
//	m_morph_env_decay=0.4f;
//	m_morph_env_punch=0.0f;

	m_Voice.FXMorphParams.fLPFResonance=0.0f;
	m_Voice.FXMorphParams.fLPFFreq=1.0f;
	m_Voice.FXMorphParams.fLPFRamp=0.0f;
	m_Voice.FXMorphParams.fHPFFreq=0.0f;
	m_Voice.FXMorphParams.fHPFRamp=0.0f;

	m_Voice.FXMorphParams.fPHAOffset=0.0f;
	m_Voice.FXMorphParams.fPHARamp=0.0f;

	m_Voice.FXMorphParams.fRepeatSpeed=0.0f;

	m_Voice.FXMorphParams.fArmSpeed=1.0f;
	m_Voice.FXMorphParams.fArmMod=0.0f;

}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::ReadData(void *pDest, int nSize, int nUnits, unsigned char *&pRAWData)
{
	int nReadSize = nSize*nUnits;
	memcpy(pDest, pRAWData, nSize*nUnits);
	pRAWData += nReadSize;

	return nReadSize;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(unsigned char *pRAWData)
{
	int version=0;
	ReadData(&version, 1, sizeof(int), pRAWData);
	if(version!=SFXR0100)
	{
		return false;
	}

	ReadData(&m_Voice.nWaveformType, 1, sizeof(int), pRAWData);

	m_Voice.fSoundVol=0.5f;
	ReadData(&m_Voice.fSoundVol, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.fMorphRate, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.nLengthInSamples, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXBaseParams.fBaseFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqLimit, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqDRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fDuty, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fDutyRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fVibStrength, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fVibSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fVibDelay, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fEnvAttack, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvSustain, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvDecay, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvPunch, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fLPFResonance, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fLPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fLPFRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fHPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fHPFRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fPHAOffset, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fPHARamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fRepeatSpeed, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fArmSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fArmMod, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXMorphParams.fBaseFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqLimit, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqDRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fDuty, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fDutyRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fVibStrength, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fVibSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fVibDelay, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fEnvAttack, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvSustain, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvDecay, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvPunch, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fLPFResonance, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fLPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fLPFRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fHPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fHPFRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fPHAOffset, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fPHARamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(const char* filename)
{
	if (G_FILE *fp = g_fopen(filename, "rb"))
	{
		g_fseek(fp, 0, SEEK_END);
		int nFileSize = g_ftell(fp);
		g_fseek(fp, 0, SEEK_SET);
		g_ftell(fp);
		unsigned char *ptr = (unsigned char *)malloc(nFileSize);
		int bytesread = g_fread(ptr, nFileSize, 1, fp);
		int nSize = nFileSize;
		g_fclose(fp);

		LoadSettings(ptr);

		free(ptr);

		return true;
	}
	else
	{
		return false;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::SaveSettings(const char* filename)
{
	G_FILE* file=g_fopen(filename, "wb");
	if(!file)
	{
		return false;
	}

	int version=SFXR0100;
	g_fwrite(&version, 1, sizeof(int), file);

	g_fwrite(&m_Voice.nWaveformType, 1, sizeof(int), file);

	g_fwrite(&m_Voice.fSoundVol, 1, sizeof(float), file);

	g_fwrite(&m_Voice.fMorphRate, 1, sizeof(float), file);

	g_fwrite(&m_Voice.nLengthInSamples, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fBaseFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fFreqLimit, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fFreqRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fFreqDRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fDuty, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fDutyRamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fVibStrength, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fVibSpeed, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fVibDelay, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fEnvAttack, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fEnvSustain, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fEnvDecay, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fEnvPunch, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fLPFResonance, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fLPFFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fLPFRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fHPFFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fHPFRamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fPHAOffset, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fPHARamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fRepeatSpeed, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXBaseParams.fArmSpeed, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXBaseParams.fArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_fwrite(&m_Voice.FXMorphParams.fBaseFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fFreqLimit, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fFreqRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fFreqDRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fDuty, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fDutyRamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fVibStrength, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fVibSpeed, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fVibDelay, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fEnvAttack, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fEnvSustain, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fEnvDecay, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fEnvPunch, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fLPFResonance, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fLPFFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fLPFRamp, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fHPFFreq, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fHPFRamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fPHAOffset, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fPHARamp, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), file);

	g_fwrite(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), file);
	g_fwrite(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_fclose(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::CompareSettings(RetroSFXVoice *pOther)
{
	if (memcmp(&this->m_Voice, &pOther->m_Voice, sizeof(this->m_Voice)) == 0)
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Reset(bool restart)
{
	if(!restart)
	{
		phase=0;

		m_FXWorkParams.fBaseFreq = m_Voice.FXBaseParams.fBaseFreq;
		m_FXWorkParams.fFreqLimit = m_Voice.FXBaseParams.fFreqLimit;
		m_FXWorkParams.fFreqRamp = m_Voice.FXBaseParams.fFreqRamp;
		m_FXWorkParams.fFreqDRamp = m_Voice.FXBaseParams.fFreqDRamp;
		m_FXWorkParams.fDuty = m_Voice.FXBaseParams.fDuty;
		m_FXWorkParams.fDutyRamp = m_Voice.FXBaseParams.fDutyRamp;

		m_FXWorkParams.fVibStrength = m_Voice.FXBaseParams.fVibStrength;
		m_FXWorkParams.fVibSpeed = m_Voice.FXBaseParams.fVibSpeed;
		m_FXWorkParams.fVibDelay = m_Voice.FXBaseParams.fVibDelay;

		m_FXWorkParams.fEnvAttack = m_Voice.FXBaseParams.fEnvAttack;
		m_FXWorkParams.fEnvSustain = m_Voice.FXBaseParams.fEnvSustain;
		m_FXWorkParams.fEnvDecay = m_Voice.FXBaseParams.fEnvDecay;
		m_FXWorkParams.fEnvPunch = m_Voice.FXBaseParams.fEnvPunch;

		m_FXWorkParams.fLPFResonance = m_Voice.FXBaseParams.fLPFResonance ;
		m_FXWorkParams.fLPFFreq = m_Voice.FXBaseParams.fLPFFreq;
		m_FXWorkParams.fLPFRamp = m_Voice.FXBaseParams.fLPFRamp;
		m_FXWorkParams.fHPFFreq = m_Voice.FXBaseParams.fHPFFreq;
		m_FXWorkParams.fHPFRamp = m_Voice.FXBaseParams.fHPFRamp;

		m_FXWorkParams.fPHAOffset = m_Voice.FXBaseParams.fPHAOffset;
		m_FXWorkParams.fPHARamp = m_Voice.FXBaseParams.fPHARamp;

		m_FXWorkParams.fRepeatSpeed = m_Voice.FXBaseParams.fRepeatSpeed;

		m_FXWorkParams.fArmSpeed = m_Voice.FXBaseParams.fArmSpeed;
		m_FXWorkParams.fArmMod = m_Voice.FXBaseParams.fArmMod;
	}

	fperiod=100.0/(m_FXWorkParams.fBaseFreq*m_FXWorkParams.fBaseFreq+0.001);
	period=(int)fperiod;
	fmaxperiod=100.0/(m_FXWorkParams.fFreqLimit*m_FXWorkParams.fFreqLimit+0.001);
	fslide=1.0-pow((double)m_FXWorkParams.fFreqRamp, 3.0)*0.01;
	fdslide=-pow((double)m_FXWorkParams.fFreqDRamp, 3.0)*0.000001;
	square_duty=0.5f-m_FXWorkParams.fDuty*0.5f;
	square_slide=-m_FXWorkParams.fDutyRamp*0.00005f;
	if(m_FXWorkParams.fArmMod>=0.0f)
	{
		arm_mod=1.0-pow((double)m_FXWorkParams.fArmMod, 2.0)*0.9;
	}
	else
	{
		arm_mod=1.0+pow((double)m_FXWorkParams.fArmMod, 2.0)*10.0;
	}
	arm_time=0;
	arm_limit=(int)(pow(1.0f-m_FXWorkParams.fArmSpeed, 2.0f)*20000+32);
	if(m_FXWorkParams.fArmSpeed==1.0f)
	{
		arm_limit=0;
	}

	if(!restart)
	{
		// reset filter
		fltp=0.0f;
		fltdp=0.0f;
		fltw=pow(m_FXWorkParams.fLPFFreq, 3.0f)*0.1f;
		fltw_d=1.0f+m_FXWorkParams.fLPFRamp*0.0001f;
		fltdmp=5.0f/(1.0f+pow(m_FXWorkParams.fLPFResonance, 2.0f)*20.0f)*(0.01f+fltw);
		if(fltdmp>0.8f) fltdmp=0.8f;
		fltphp=0.0f;
		flthp=pow(m_FXWorkParams.fHPFFreq, 2.0f)*0.1f;
		flthm_d=1.0f+m_FXWorkParams.fHPFRamp*0.0003f;
		// reset vibrato
		vib_phase=0.0f;
		vib_speed=pow(m_Voice.FXBaseParams.fVibSpeed, 2.0f)*0.01f;
		vib_amp=m_FXWorkParams.fVibStrength*0.5f;
		// reset envelope
		env_vol=0.0f;
		env_stage=0;
		env_time=0;
		env_length[0]=(int)(m_FXWorkParams.fEnvAttack*m_FXWorkParams.fEnvAttack*100000.0f);
		env_length[1]=(int)(m_FXWorkParams.fEnvSustain*m_FXWorkParams.fEnvSustain*100000.0f);
		env_length[2]=(int)(m_FXWorkParams.fEnvDecay*m_FXWorkParams.fEnvDecay*100000.0f);

		fphase=pow(m_FXWorkParams.fPHAOffset, 2.0f)*1020.0f;
		if(m_FXWorkParams.fPHAOffset<0.0f) fphase=-fphase;
		fdphase=pow(m_FXWorkParams.fPHARamp, 2.0f)*1.0f;
		if(m_FXWorkParams.fPHARamp<0.0f) fdphase=-fdphase;
		iphase=abs((int)fphase);
		ipp=0;
		for(int i=0;i<1024;i++)
			phaser_buffer[i]=0.0f;

		for(int i=0;i<32;i++)
		{
			noise_buffer[i]=GenNoise();
		}

		rem_time=0;
		rem_limit=(int)(pow(1.0f-m_FXWorkParams.fRepeatSpeed, 2.0f)*20000+32);
		if(m_FXWorkParams.fRepeatSpeed==0.0f)
			rem_limit=0;
	}
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(void *pData)
{
	if (pData)
	{
		RetroVoice103 *pVoice103 = (RetroVoice103*)pData;
		if (pVoice103->nVersion==SFXR0100)
		{
			memcpy(&m_Voice, pData, sizeof(RetroVoice103));
		}
	}
	m_WavSamplesRendered = 0;
	Reset(false);
	m_bPlayingSample=true;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(bool bCalculateLength)
{
	if (bCalculateLength)
	{
		m_Voice.nLengthInSamples = 0;
	}
	m_WavSamplesRendered = 0;
	Reset(false);
	m_bPlayingSample = true;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::GetVoiceLengthInSamples()
{
	return m_Voice.nLengthInSamples;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::Render(int nSamples, short *pBuffer)
{
	int nSamplesRendered = 0;

	for(int i=0;i<nSamples;i++)
	{
		if(!m_bPlayingSample)
			break;

		rem_time++;
		if(rem_limit!=0 && rem_time>=rem_limit)
		{
			rem_time=0;
			Reset(true);

			if (m_Voice.fMorphRate!=0.0f)
			{
				Morph(m_FXWorkParams.fBaseFreq, m_Voice.FXMorphParams.fBaseFreq);
				Morph(m_FXWorkParams.fFreqLimit, m_Voice.FXMorphParams.fFreqLimit);
				Morph(m_FXWorkParams.fFreqRamp, m_Voice.FXMorphParams.fFreqRamp);
				Morph(m_FXWorkParams.fFreqDRamp, m_Voice.FXMorphParams.fFreqDRamp);
				Morph(m_FXWorkParams.fDuty, m_Voice.FXMorphParams.fDuty);
				Morph(m_FXWorkParams.fDutyRamp, m_Voice.FXMorphParams.fDutyRamp);

				Morph(m_FXWorkParams.fVibStrength, m_Voice.FXMorphParams.fVibStrength);
				Morph(m_FXWorkParams.fVibSpeed, m_Voice.FXMorphParams.fVibSpeed);
				Morph(m_FXWorkParams.fVibDelay, m_Voice.FXMorphParams.fVibDelay);

				Morph(m_FXWorkParams.fEnvAttack, m_Voice.FXMorphParams.fEnvAttack);
				Morph(m_FXWorkParams.fEnvSustain, m_Voice.FXMorphParams.fEnvSustain);
				Morph(m_FXWorkParams.fEnvDecay, m_Voice.FXMorphParams.fEnvDecay);
				Morph(m_FXWorkParams.fEnvPunch, m_Voice.FXMorphParams.fEnvPunch);

				Morph(m_FXWorkParams.fLPFResonance, m_Voice.FXMorphParams.fLPFResonance);
				Morph(m_FXWorkParams.fLPFFreq, m_Voice.FXMorphParams.fLPFFreq);
				Morph(m_FXWorkParams.fLPFRamp, m_Voice.FXMorphParams.fLPFRamp);
				Morph(m_FXWorkParams.fHPFFreq, m_Voice.FXMorphParams.fHPFFreq);
				Morph(m_FXWorkParams.fHPFRamp, m_Voice.FXMorphParams.fHPFRamp);

				Morph(m_FXWorkParams.fPHAOffset, m_Voice.FXMorphParams.fPHAOffset);
				Morph(m_FXWorkParams.fPHARamp, m_Voice.FXMorphParams.fPHARamp);

				Morph(m_FXWorkParams.fRepeatSpeed, m_Voice.FXMorphParams.fRepeatSpeed);

				Morph(m_FXWorkParams.fArmSpeed, m_Voice.FXMorphParams.fArmSpeed);
				Morph(m_FXWorkParams.fArmMod, m_Voice.FXMorphParams.fArmMod);
			}

		}

		// frequency envelopes/arpeggios
		arm_time++;
		if(arm_limit!=0 && arm_time>=arm_limit)
		{
			arm_time=0;
			fperiod*=arm_mod;
		}
		fslide+=fdslide;
		fperiod*=fslide;
		if(fperiod>fmaxperiod)
		{
			fperiod=fmaxperiod;
			if(m_FXWorkParams.fFreqLimit>0.0f)
			{
				m_bPlayingSample=false;
			}
		}
		float rfperiod=(float)fperiod;
		if(vib_amp>0.0f)
		{
			vib_phase+=vib_speed;
			rfperiod=(float)fperiod*(1.0f+sinf(vib_phase)*vib_amp);
		}
		period=(int)rfperiod;
		if(period<8) period=8;
		square_duty+=square_slide;
		if(square_duty<0.0f) square_duty=0.0f;
		if(square_duty>0.5f) square_duty=0.5f;		
		// volume envelope
		env_time++;
		if(env_time>env_length[env_stage])
		{
			env_time=0;
			env_stage++;
			if(env_stage==3)
			{
				m_bPlayingSample=false;
			}
		}
		if(env_stage==0)
			env_vol=(float)env_time/env_length[0];
		if(env_stage==1)
			env_vol=1.0f+pow(1.0f-(float)env_time/env_length[1], 1.0f)*2.0f*m_FXWorkParams.fEnvPunch;
		if(env_stage==2)
			env_vol=1.0f-(float)env_time/env_length[2];

		// phaser step
		fphase+=fdphase;
		iphase=abs((int)fphase);
		if(iphase>1023) iphase=1023;

		if(flthm_d!=0.0f)
		{
			flthp*=flthm_d;
			if(flthp<0.00001f) flthp=0.00001f;
			if(flthp>0.1f) flthp=0.1f;
		}

		float ssample=0.0f;
		for(int si=0;si<8;si++) // 8x oversampling
		{
			float sample=0.0f;
			phase++;
			if(phase>=period)
			{
				//				phase=0;
				phase%=period;
				if(m_Voice.nWaveformType==3)
					for(int i=0;i<32;i++)
					{
						noise_buffer[i]=GenNoise();
					}
			}
			// base waveform
			float fp=(float)phase/period;
			switch(m_Voice.nWaveformType)
			{
			case 0: // square
				if(fp<square_duty)
					sample=0.5f;
				else
					sample=-0.5f;
				break;
			case 1: // sawtooth
				sample=1.0f-fp*2;
				break;
			case 2: // sine
				sample=(float)sin(fp*2*PI);
				break;
			case 3: // noise
				sample=noise_buffer[phase*32/period];
				break;
			}
			// lp filter
			float pp=fltp;
			fltw*=fltw_d;
			if(fltw<0.0f) fltw=0.0f;
			if(fltw>0.1f) fltw=0.1f;
			if(m_FXWorkParams.fLPFFreq!=1.0f)
			{
				fltdp+=(sample-fltp)*fltw;
				fltdp-=fltdp*fltdmp;
			}
			else
			{
				fltp=sample;
				fltdp=0.0f;
			}
			fltp+=fltdp;
			// hp filter
			fltphp+=fltp-pp;
			fltphp-=fltphp*flthp;
			sample=fltphp;
			// phaser
			phaser_buffer[ipp&1023]=sample;
			sample+=phaser_buffer[(ipp-iphase+1024)&1023];
			ipp=(ipp+1)&1023;
			// final accumulation and envelope application
			ssample+=sample*env_vol;
		}
		ssample=ssample/8*m_fMasterVol;

		ssample*=2.0f*m_Voice.fSoundVol;

		if(pBuffer!=NULL)
		{
			// Clamp
			if(ssample>1.0f) ssample=1.0f;
			if(ssample<-1.0f) ssample=-1.0f;

			*pBuffer += (short)(ssample * 32767.0f);
			pBuffer++;
			nSamplesRendered++;
		}
	}

	// Calculating length here
	m_WavSamplesRendered += nSamplesRendered;
	if (m_bPlayingSample == false)
	{
		m_Voice.nLengthInSamples = m_WavSamplesRendered;
	}

	return nSamplesRendered;
}

//////////////////////////////////////////////////////////////////////////

float RetroSFXVoice::GenNoise()
{
	float range=2.0f;
	return ((float)rnd(10000)/10000*range)-1.0f;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Morph(float &fMorphVar, float fMorphDest)
{
	float fDiff = fMorphDest - fMorphVar;

	if (fDiff != 0.0f)
	{
		fMorphVar += fDiff * m_Voice.fMorphRate;
	}
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::IsActive()
{
	if (m_bPlayingSample)
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////


