#include "test.h"
#include "../src/effects.h"


static int vals_ft2[] = {
	128, 128, 128,		/* C-5 1 880   play instrument w/ center pan */
	255, 255, 255,		/* --- - 8FF   set pan to right */
	255, 255, 255,		/* F-5 - ---   new note keeps previous pan */
	255, 255, 255,		/* --- - 8FF   set new pan */
	0, 0, 0			/* --- 1 ---   new inst resets to left pan */
};

static int vals_st3[] = {
	128, 128, 128,		/* C-5 1 880   play instrument w/ center pan */
	255, 255, 255,		/* --- - 8FF   set pan to right */
	255, 255, 255,		/* F-5 - ---   new note keeps previous pan */
	255, 255, 255,		/* --- - 8FF   set new pan */
	0, 0, 0			/* --- 1 ---   new inst resets to left pan */
};

static int vals_it[] = {
	128, 128, 128,		/* C-5 1 880   play instrument w/ center pan */
	255, 255, 255,		/* --- - 8FF   set pan to right */
	0, 0, 0,		/* F-5 - ---   new note uses instrument pan */
	255, 255, 255,		/* --- - 8FF   set new pan */
	0, 0, 0			/* --- 1 ---   new inst resets to left pan */
};

static int vals_it_dp[] = {
	128, 128, 128,		/* C-5 1 880   play instrument w/ center pan */
	255, 255, 255,		/* --- - 8FF   set pan to right */
	255, 255, 255,		/* F-5 - ---   instrument pan is disabled */
	255, 255, 255,		/* --- - 8FF   set new pan */
	255, 255, 255		/* --- 1 ---   new inst resets to left pan */
};

TEST(test_effect_8_setpan)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* set instrument 1 pan to left */
	ctx->m.mod.xxi[0].sub[0].pan = 0;

	/* slide down & up with memory */
	new_event(ctx, 0, 0, 0, 60, 1, 0, FX_SETPAN, 0x80, FX_SPEED, 0x03);
	new_event(ctx, 0, 1, 0,  0, 0, 0, FX_SETPAN, 0xff, 0, 0);
	new_event(ctx, 0, 2, 0, 65, 0, 0, 0x00,      0x00, 0, 0);
	new_event(ctx, 0, 3, 0,  0, 0, 0, FX_SETPAN, 0xff, 0, 0);
	new_event(ctx, 0, 4, 0,  0, 1, 0, 0x00,      0x00, 0, 0);

	/* play it */
	xmp_start_player(opaque, 44100, 0);

	/* set mix to 100% pan separation */
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	/* check FT2 event reader */
	set_quirk(ctx, QUIRKS_FT2, READ_EVENT_FT2);

	for (i = 0; i < 4 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].pan == vals_ft2[i], "pan error");
	}

	xmp_restart_module(opaque);

	/* check ST3 event reader */
	set_quirk(ctx, QUIRKS_ST3, READ_EVENT_ST3);

	for (i = 0; i < 4 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].pan == vals_st3[i], "pan error");
	}

	xmp_restart_module(opaque);

	/* check IT event reader */
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

	for (i = 0; i < 4 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].pan == vals_it[i], "pan error");
	}

	xmp_restart_module(opaque);

	/* set instrument pan as disabled */
	ctx->m.mod.xxi[0].sub[0].pan = -1;

	for (i = 0; i < 4 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].pan == vals_it_dp[i], "pan error");
	}
}
END_TEST
