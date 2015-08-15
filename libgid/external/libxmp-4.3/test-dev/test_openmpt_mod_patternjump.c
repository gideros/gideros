#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../src/mixer.h"
#include "../src/virtual.h"

/*
 A Bxx command should reset the effect of a Dxx command that is left of
 the Bxx command. You should hear a voice saying “success”.
*/

TEST(test_openmpt_mod_patternjump)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct module_data *m;
        struct player_data *p;
        struct mixer_voice *vi;
	struct xmp_frame_info fi;
	int time, row, frame, chan, period, note, ins, vol, pan, pos0;
	char line[200];
	FILE *f;
	int i, voc;

	f = fopen("openmpt/mod/PatternJump.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "openmpt/mod/PatternJump.mod");

	ctx = (struct context_data *)opaque;
	m = &ctx->m;
	p = &ctx->p;

	xmp_start_player(opaque, 44100, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &fi);
		if (fi.loop_count > 0)
			break;
		if (fi.pos == 1 && fi.row == 4)
			break;

		for (i = 0; i < m->mod.chn; i++) {
			struct xmp_channel_info *ci = &fi.channel_info[i];

			voc = map_channel(p, i);
			if (voc < 0)
				continue;

			vi = &p->virt.voice_array[voc];

			fgets(line, 200, f);
			sscanf(line, "%d %d %d %d %d %d %d %d %d %d",
				&time, &row, &frame, &chan, &period,
				&note, &ins, &vol, &pan, &pos0);

			fail_unless(fi.time    == time,   "time mismatch");
			fail_unless(fi.row     == row,    "row mismatch");
			fail_unless(fi.frame   == frame,  "frame mismatch");
			fail_unless(ci->period == period, "period mismatch");
			fail_unless(vi->note   == note,   "note mismatch");
			fail_unless(vi->ins    == ins,    "instrument");
			fail_unless(vi->vol    == vol,    "volume mismatch");
			fail_unless(vi->pan    == pan,    "pan mismatch");
			fail_unless(vi->pos0   == pos0,   "position mismatch");
		}
		
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST
