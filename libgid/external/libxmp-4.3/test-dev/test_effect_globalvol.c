#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	32, 32, 32,	/* set as 0x20 */
	0, 0, 0,	/* set as 0x00 */
	64, 64, 64,	/* set as 0x40 */
	64, 64, 64,	/* set as 0x41 */
	64, 64, 64	/* set as 0x90 */
};

TEST(test_effect_globalvol)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_GLOBALVOL, 0x20, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_GLOBALVOL, 0x00, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_GLOBALVOL, 0x40, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_GLOBALVOL, 0x41, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_GLOBALVOL, 0x90, 0, 0);

	/* Set format as IT so silent channels will be reset. Global
	 * volume slides shouldn't be ignored!
	 */
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 5 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.volume == vals[i], "global volume error");
	}
}
END_TEST
