#include "test.h"
#include "../src/effects.h"


static int vals[] = {
	48, 48, 48,	/* set 0x30 */
	64, 64, 64,	/* instrument default */
	0, 0, 0,	/* set 0x00 */
	64, 64, 64,	/* set 0x40 */
	64, 64, 64,	/* set 0x41 */
	64, 64, 64,	/* set 0x50 */
	64, 64, 64,	/* set 0x51 */
	64, 64, 64	/* set 0xa0 */
};

static int vals2[] = {
	38, 38, 38,	/* set 0x30 */
	51, 51, 51,	/* instrument default */
	0, 0, 0,	/* set 0x00 */
	51, 51, 51,	/* set 0x40 */
	52, 52, 52,	/* set 0x41 */
	64, 64, 64,	/* set 0x50 */
	64, 64, 64,	/* set 0x51 */
	64, 64, 64 	/* set 0xa0 */
};

static int vals3[] = {
	16, 16, 16,	/* set 0x30 */
	0, 0, 0,	/* instrument default */
	64, 64, 64,	/* set 0x00 */
	0, 0, 0,	/* set 0x40 */
	0, 0, 0,	/* set 0x41 */
	0, 0, 0,	/* set 0x50 */
	0, 0, 0,	/* set 0x51 */
	0, 0, 0 	/* set 0xa0 */
};

TEST(test_effect_c_volset)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct module_data *m;
	struct xmp_frame_info info;
	int volmap[65];
	int i;

	for (i = 0; i < 65; i++) {
		volmap[i] = 64 - i;
	}

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
 	m = &ctx->m;

 	create_simple_module(ctx, 2, 2);

	/* slide down & up with memory */
	new_event(ctx, 0, 0, 0, 60, 1, 0x20, FX_VOLSET, 0x30, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0x20, FX_VOLSET, 0x00, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_VOLSET, 0x40, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_VOLSET, 0x41, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_VOLSET, 0x50, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_VOLSET, 0x51, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_VOLSET, 0xa0, 0, 0);

	/* play it */
	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == vals[i], "volume set error");

	}

	/* again different volbase */
	m->volbase = 0x50;
	xmp_restart_module(opaque);

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == vals2[i], "volume set error");
	}

	/* again with volume map */
	m->volbase = 0x40;
	m->vol_table = volmap;
	xmp_restart_module(opaque);

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == vals3[i], "volume set error");
	}
}
END_TEST
