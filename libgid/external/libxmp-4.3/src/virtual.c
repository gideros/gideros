/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "virtual.h"
#include "mixer.h"

#define	FREE	-1

/* For virt_pastnote() */
void player_set_release(struct context_data *, int);
void player_set_fadeout(struct context_data *, int);


/* Get parent channel */
int virt_getroot(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi;
	int voc;

	voc = p->virt.virt_channel[chn].map;
	if (voc < 0)
		return -1;

	vi = &p->virt.voice_array[voc];

	return vi->root;
}

void virt_resetvoice(struct context_data *ctx, int voc, int mute)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];

	if ((uint32)voc >= p->virt.maxvoc)
		return;

	if (mute) {
		mixer_setvol(ctx, voc, 0);
	}

	p->virt.virt_used--;
	p->virt.virt_channel[vi->root].count--;
	p->virt.virt_channel[vi->chn].map = FREE;
	memset(vi, 0, sizeof(struct mixer_voice));
	vi->chn = vi->root = FREE;
}

/* virt_on (number of tracks) */
int virt_on(struct context_data *ctx, int num)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	int i;

	p->virt.num_tracks = num;
	num = mixer_numvoices(ctx, -1);

	p->virt.virt_channels = p->virt.num_tracks;

	if (HAS_QUIRK(QUIRK_VIRTUAL)) {
		p->virt.virt_channels += num;
	} else if (num > p->virt.virt_channels) {
		num = p->virt.virt_channels;
	}

	p->virt.maxvoc = mixer_numvoices(ctx, num);

	p->virt.voice_array = calloc(p->virt.maxvoc,
				sizeof(struct mixer_voice));
	if (p->virt.voice_array == NULL)
		goto err;

	for (i = 0; i < p->virt.maxvoc; i++) {
		p->virt.voice_array[i].chn = FREE;
		p->virt.voice_array[i].root = FREE;
	}

	p->virt.virt_channel = malloc(p->virt.virt_channels *
				sizeof(struct virt_channel));
	if (p->virt.virt_channel == NULL)
		goto err1;

	for (i = 0; i < p->virt.virt_channels; i++) {
		p->virt.virt_channel[i].map = FREE;
		p->virt.virt_channel[i].count = 0;
	}

	p->virt.virt_used = 0;

	return 0;

      err1:
	free(p->virt.voice_array);
      err:
	return -1;
}

void virt_off(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;

	p->virt.virt_used = p->virt.maxvoc = 0;
	p->virt.virt_channels = 0;
	p->virt.num_tracks = 0;
	free(p->virt.voice_array);
	free(p->virt.virt_channel);
}

void virt_reset(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	int i;

	if (p->virt.virt_channels < 1)
		return;

	mixer_numvoices(ctx, p->virt.maxvoc);

	memset(p->virt.voice_array, 0,
	       p->virt.maxvoc * sizeof(struct mixer_voice));
	for (i = 0; i < p->virt.maxvoc; i++) {
		p->virt.voice_array[i].chn = FREE;
		p->virt.voice_array[i].root = FREE;
	}

	for (i = 0; i < p->virt.virt_channels; i++) {
		p->virt.virt_channel[i].map = FREE;
		p->virt.virt_channel[i].count = 0;
	}

	p->virt.virt_used = 0;
}

static int free_voice(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	int i, num, vol;

	/* Find background voice with lowest volume*/
	num = FREE;
	vol = INT_MAX;
	for (i = 0; i < p->virt.maxvoc; i++) {
		struct mixer_voice *vi = &p->virt.voice_array[i];

		if (vi->chn >= p->virt.num_tracks && vi->vol < vol) {
			num = i;
			vol = vi->vol;
		}
	}

	/* Free voice */
	p->virt.virt_channel[p->virt.voice_array[num].chn].map = FREE;
	p->virt.virt_channel[p->virt.voice_array[num].root].count--;
	p->virt.virt_used--;

	return num;
}

static int alloc_voice(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int i;

	/* Find free voice */
	for (i = 0; i < p->virt.maxvoc; i++) {
		if (p->virt.voice_array[i].chn == FREE)
			break;
	}

	/* not found */
	if (i == p->virt.maxvoc)
		i = free_voice(ctx);

	p->virt.virt_channel[chn].count++;
	p->virt.virt_used++;

	p->virt.voice_array[i].chn = chn;
	p->virt.voice_array[i].root = chn;
	p->virt.virt_channel[chn].map = i;

	return i;
}

static int map_virt_channel(struct player_data *p, int chn)
{
	int voc;

	if ((uint32)chn >= p->virt.virt_channels)
		return -1;

	voc = p->virt.virt_channel[chn].map;

	if ((uint32)voc >= p->virt.maxvoc)
		return -1;

	return voc;
}

int virt_mapchannel(struct context_data *ctx, int chn)
{
	return map_virt_channel(&ctx->p, chn);
}

void virt_resetchannel(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_setvol(ctx, voc, 0);

	p->virt.virt_used--;
	p->virt.virt_channel[p->virt.voice_array[voc].root].count--;
	p->virt.virt_channel[chn].map = FREE;
	memset(&p->virt.voice_array[voc], 0, sizeof(struct mixer_voice));
	p->virt.voice_array[voc].chn = p->virt.voice_array[voc].root = FREE;
}

void virt_setvol(struct context_data *ctx, int chn, int vol)
{
	struct player_data *p = &ctx->p;
	int voc, root;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	root = p->virt.voice_array[voc].root;
	if (root < XMP_MAX_CHANNELS && p->channel_mute[root])
		vol = 0;

	mixer_setvol(ctx, voc, vol);

	if (vol == 0 && chn >= p->virt.num_tracks)
		virt_resetvoice(ctx, voc, 1);
}

void virt_setpan(struct context_data *ctx, int chn, int pan)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_setpan(ctx, voc, pan);
}

void virt_seteffect(struct context_data *ctx, int chn, int type, int val)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_seteffect(ctx, voc, type, val);
}

int virt_getvoicepos(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return -1;

	return mixer_getvoicepos(ctx, voc);
}

#ifndef LIBXMP_CORE_PLAYER

void virt_setsmp(struct context_data *ctx, int chn, int smp)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi;
	int voc, pos, frac;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	vi = &p->virt.voice_array[voc];
	if (vi->smp == smp)
		return;

	pos = vi->pos;
	frac = vi->frac;

	mixer_setpatch(ctx, voc, smp);
	mixer_voicepos(ctx, voc, pos, frac);	/* Restore old position */
}

#endif

#ifndef LIBXMP_CORE_DISABLE_IT

void virt_setnna(struct context_data *ctx, int chn, int nna)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	p->virt.voice_array[voc].act = nna;
}

static void check_dct(struct context_data *ctx, int i, int chn, int ins,
			int smp, int note, int dct, int dca)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[i];
	int voc;

	voc = p->virt.virt_channel[chn].map;

	if (vi->root == chn && vi->ins == ins) {
		if (dct == XMP_INST_DCT_INST ||
			(dct == XMP_INST_DCT_SMP && vi->smp == smp) ||
			(dct == XMP_INST_DCT_NOTE && vi->note == note)) {

			if (dca) {
				if (i != voc || vi->act)
					vi->act = dca;
			} else {
				virt_resetvoice(ctx, i, 1);
			}
		}
	}
}

#endif

/* For note slides */
void virt_setnote(struct context_data *ctx, int chn, int note)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_setnote(ctx, voc, note);
}

int virt_setpatch(struct context_data *ctx, int chn, int ins, int smp,
		    			int note, int nna, int dct, int dca)
{
	struct player_data *p = &ctx->p;
	int voc, vfree;

	if ((uint32)chn >= p->virt.virt_channels)
		return -1;

	if (ins < 0)
		smp = -1;

#ifndef LIBXMP_CORE_DISABLE_IT
	if (dct) {
		int i;

		for (i = 0; i < p->virt.maxvoc; i++)
			check_dct(ctx, i, chn, ins, smp, note, dct, dca);
	}
#endif

	voc = p->virt.virt_channel[chn].map;

	if (voc > FREE) {
		if (p->virt.voice_array[voc].act) {
			vfree = alloc_voice(ctx, chn);

			for (chn = p->virt.num_tracks;
			     p->virt.virt_channel[chn++].map > FREE;) ;

			p->virt.voice_array[voc].chn = --chn;
			p->virt.virt_channel[chn].map = voc;
			voc = vfree;
		}
	} else {
		voc = alloc_voice(ctx, chn);
	}

	if (smp < 0) {
		virt_resetvoice(ctx, voc, 1);
		return chn;	/* was -1 */
	}

	mixer_setpatch(ctx, voc, smp);
	mixer_setnote(ctx, voc, note);
	p->virt.voice_array[voc].ins = ins;
	p->virt.voice_array[voc].act = nna;

	return chn;
}

void virt_setbend(struct context_data *ctx, int chn, int bend)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_setbend(ctx, voc, bend);
}

void virt_voicepos(struct context_data *ctx, int chn, int pos)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return;

	mixer_voicepos(ctx, voc, pos, 0);
}

#ifndef LIBXMP_CORE_DISABLE_IT

void virt_pastnote(struct context_data *ctx, int chn, int act)
{
	struct player_data *p = &ctx->p;
	int c, voc;

	for (c = p->virt.num_tracks; c < p->virt.virt_channels; c++) {
		if ((voc = map_virt_channel(p, c)) < 0)
			continue;

		if (p->virt.voice_array[voc].root == chn) {
			switch (act) {
			case VIRT_ACTION_CUT:
				virt_resetvoice(ctx, voc, 1);
				break;
			case VIRT_ACTION_OFF:
				player_set_release(ctx, c);
				break;
			case VIRT_ACTION_FADE:
				player_set_fadeout(ctx, c);
				break;
			}
		}
	}
}

#endif

int virt_cstat(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int voc;

	if ((voc = map_virt_channel(p, chn)) < 0)
		return VIRT_INVALID;

	if (chn < p->virt.num_tracks)
		return VIRT_ACTIVE;

	return p->virt.voice_array[voc].act;
}
