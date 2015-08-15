#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	64, 62, 60,	/* down 2 */
	60, 58, 56,	/* memory */
	56, 57, 58,	/* up 1 */
	58, 59, 60,	/* memory */
	59, 59, 59,	/* fine down 1 */
	58, 58, 58,	/* memory */
	60, 60, 60,	/* fine up 2 */
	62, 62, 62,	/* memory */
	64, 64, 64,	/* memory */
	64, 64, 64	/* limit */
};

TEST(test_effect_gvol_slide)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x02, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x10, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_GVOL_SLIDE, 0xf1, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x2f, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 8, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 9, 0, 0, 0, 0, FX_GVOL_SLIDE, 0x00, 0, 0);

	/* Set format as IT so silent channels will be reset. Global
	 * volume slides shouldn't be ignored!
	 */
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 10 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.volume == vals[i], "global volume error");
	}
}
END_TEST
