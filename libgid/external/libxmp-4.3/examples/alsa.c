/* Alsa driver for libxmp examples */
/* This file is in public domain */

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include "sound.h"

static snd_pcm_t *pcm_handle;


int sound_init(int sampling_rate, int channels)
{
	snd_pcm_hw_params_t *hwparams;
	int ret;
	unsigned int chan, rate;
	unsigned int btime = 250000;	/* 250ms */
	unsigned int ptime = 50000;	/* 50ms */
	char *card_name = "default";

	if ((ret = snd_pcm_open(&pcm_handle, card_name,
		SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "Unable to initialize ALSA pcm device: %s\n",
					snd_strerror(ret));
		return -1;
	}

	chan = channels;
	rate = sampling_rate;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_hw_params_any(pcm_handle, hwparams);
	snd_pcm_hw_params_set_access(pcm_handle, hwparams,
				SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16);
	snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &rate, 0);
	snd_pcm_hw_params_set_channels_near(pcm_handle, hwparams, &chan);
	snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &btime, 0);
	snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &ptime, 0);
	snd_pcm_nonblock(pcm_handle, 0);
	
	if ((ret = snd_pcm_hw_params(pcm_handle, hwparams)) < 0) {
		fprintf(stderr, "Unable to set ALSA output parameters: %s\n",
					snd_strerror(ret));
		return -1;
	}

	if ((ret = snd_pcm_prepare(pcm_handle)) < 0) {
		fprintf(stderr, "Unable to prepare ALSA: %s\n",
					snd_strerror(ret));
		return -1;
	}
  
	return 0;
}

void sound_play(void *b, int i)
{
	int frames;

	frames = snd_pcm_bytes_to_frames(pcm_handle, i);
	if (snd_pcm_writei(pcm_handle, b, frames) < 0) {
		snd_pcm_prepare(pcm_handle);
	}
}

void sound_deinit()
{
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
}
