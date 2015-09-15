#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

TEST(test_effect_ed_delay)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	new_event(ctx, 0, 0, 0, 60, 2, 40, 0x0f, 0x02, 0, 0);
	new_event(ctx, 0, 1, 0, 61, 1,  0, 0, 0, 0x0e, 0xd0);
	new_event(ctx, 0, 2, 0, 62, 2,  0, 0x0e, 0xd1, 0, 0);
	new_event(ctx, 0, 3, 0, 63, 2,  0, 0, 0, 0x0e, 0xd3);
	new_event(ctx, 0, 4, 0,  0, 0,  0, 0, 0,    0,    0);
	new_event(ctx, 0, 5, 0,  0, 0,  0, 0, 0, 0x0e, 0xd1);
	new_event(ctx, 0, 6, 0,  0, 0,  0, 0, 0, 0x0e, 0xd1);
	new_event(ctx, 0, 7, 0,  0, 0,  0, 0, 0, 0x0e, 0xd0);
	new_event(ctx, 0, 8, 0,  0, 3,  0, 0, 0,    0,    0);
	new_event(ctx, 0, 9, 0,  0, 0,  0, 0, 0, 0x0e, 0xd1);

	xmp_start_player(opaque, 44100, 0);

	/* Row 0 */
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "row 0 frame 0");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "row 0 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Row 1 */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 60, "row 1 frame 0");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 60, "row 1 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Row 2: delay this frame */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 60, "row 2 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* note changes in frame 1 */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 2 frame 1");
	fail_unless(vi->pos0 ==  0, "sample position");

	/* Row 3: delay larger than speed */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 3 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 3 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Row 4: nothing should happen */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 4 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 4 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Now test retrigs on delay effect */

	/* Row 5: in standard mode */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 5 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 5 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	set_quirk(ctx, QUIRK_RTDELAY, READ_EVENT_FT2);

	/* Row 6: in retrig mode */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 6 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 6 frame 1");
	fail_unless(vi->pos0 ==  0, "sample position");

	/* Row 7: don't retrig */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 7 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 7 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Row 8: invalid instrument is ignored */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 8 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 8 frame 1");
	fail_unless(vi->pos0 !=  0, "sample position");

	/* Row 9: don't retrig */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 61, "row 9 frame 0");
	fail_unless(vi->pos0 !=  0, "sample position");
	fail_unless(vi->vol  !=  0, "voice volume");

	xmp_play_frame(opaque);
	/* we cut the virtual voice on invalid instrument */
	fail_unless(vi->note ==  0, "row 9 frame 1");
	fail_unless(vi->pos0 ==  0, "sample position");
	fail_unless(vi->vol  ==  0, "voice volume");
}
END_TEST
