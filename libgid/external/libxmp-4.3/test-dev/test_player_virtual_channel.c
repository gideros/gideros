#include "test.h"


TEST(test_player_virtual_channel)
{
	xmp_context opaque;
	struct xmp_frame_info fi;
	struct context_data *ctx;
	struct module_data *m;
	int i, j, used;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	m = &ctx->m;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 22);
	set_instrument_volume(ctx, 1, 0, 33);
	set_instrument_nna(ctx, 0, 0, XMP_INST_NNA_CONT, XMP_INST_DCT_OFF,
							XMP_INST_DCA_CUT);

	m->mod.spd = 3;
	xmp_scan_module(opaque);

	set_instrument_envelope(ctx, 0, 0, 0, 64);
	set_instrument_envelope(ctx, 0, 1, 34 * 3, 0);

	for (i = 0; i < 34; i++) {
		for (j = 0; j < 4; j++) {
			new_event(ctx, 0, i, j, 60, 1, 44, 0, 0, 0, 0);
		}
	}
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 34; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &fi);

		used = (i + 1) * 4;
		if (used > 128)
			used = 128;

		fail_unless(fi.virt_used == used, "number of virtual channels");

		xmp_play_frame(opaque);
		xmp_play_frame(opaque);
	}

	for (i = 0; i < 34; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &fi);

		used = (33 - i) * 4;
		if (used > 128)
			used = 128;
		else if (used < 4)
			used = 4;

		fail_unless(fi.virt_used == used, "number of virtual channels");

		xmp_play_frame(opaque);
		xmp_play_frame(opaque);
	}
}
END_TEST
