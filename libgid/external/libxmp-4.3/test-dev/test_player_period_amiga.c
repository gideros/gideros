#include "test.h"

TEST(test_player_period_amiga)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i, p0, p1;
	FILE *f;

	f = fopen("data/periods.data", "r");
	fail_unless(f != NULL, "can't open data file");

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	xmp_start_player(opaque, 44100, 0);

	/* test note periods */
	for (i = 0; i < 60; i++) {
		new_event(ctx, 0, i, 0, i, 1, 0, 0x0f, 1, 0, 0);
		new_event(ctx, 0, i, 1, 60 + i, 1, 0, 0, 0, 0, 0);
	}

	for (i = 0; i < 60; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fscanf(f, "%d %d", &p0, &p1);
		fail_unless(info.channel_info[0].period == p0, "Bad period");
		fail_unless(info.channel_info[1].period == p1, "Bad period");
	}

	fscanf(f, "%d %d", &p0, &p1);
	fail_unless(feof(f), "not end of data file");

}
END_TEST
