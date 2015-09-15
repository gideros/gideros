#include <stdio.h>
#include <stdlib.h>
#include "../include/xmp.h"
#include "../src/common.h"
#include "../src/mixer.h"
#include "../src/virtual.h"
#include "../src/player.h"


static int map_channel(struct player_data *p, int chn)
{
	int voc;

	if ((uint32)chn >= p->virt.virt_channels)
		return -1;

	voc = p->virt.virt_channel[chn].map;

	if ((uint32)voc >= p->virt.maxvoc)
		return -1;

	return voc;
}



int main(int argc, char **argv)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct module_data *m;
	struct player_data *p;
	struct mixer_voice *vi;
	struct xmp_frame_info fi;
	int i, voc;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <module>\n", argv[0]);
		exit(1);
	}

	opaque = xmp_create_context();
	if (xmp_load_module(opaque, argv[1]) < 0) {
		fprintf(stderr, "can't load module\n");
		exit(1);
	}

	ctx = (struct context_data *)opaque;
	m = &ctx->m;
	p = &ctx->p;

	xmp_start_player(opaque, 44100, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (xmp_play_frame(opaque) == 0) {
		xmp_get_frame_info(opaque, &fi);
		if (fi.loop_count >= 4)
			break;

		for (i = 0; i < m->mod.chn; i++) {
			struct xmp_channel_info *ci = &fi.channel_info[i];
			struct channel_data *xc = &p->xc_data[i];

			voc = map_channel(p, i);
			if (voc < 0 || TEST_NOTE(NOTE_SAMPLE_END))
				continue;

			vi = &p->virt.voice_array[voc];

			printf("%d %d %d %d %d %d %d %d %d %d %d\n",
				fi.time, fi.row, fi.frame, i, ci->period,
				vi->note, vi->ins, vi->vol, vi->pan, vi->pos0,
				vi->filter.cutoff);
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
}
