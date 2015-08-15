#include "../include/xmp.h"
#include "../src/common.h"
#include "../src/player.h"
#include "test.h"

TEST(test_player_invalid_period)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct channel_data *xc;
	struct xmp_frame_info info;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 49, 1, 0, 0, 0, 0, 0);

	xmp_start_player(opaque, 44100, 0);
	xc = &p->xc_data[0];

	/* Frame 0 */

	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.channel_info[0].period == 3505664, "period error");
	fail_unless(info.channel_info[0].volume == 64, "period error");

	/* Frame 1 */

	xc->period = 1;
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.channel_info[0].period == 4096, "period error");
	fail_unless(info.channel_info[0].volume == 64, "period error");

	/* Frame 2 */
	xc->period = 0;
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.channel_info[0].period == 4096, "period error");
	fail_unless(info.channel_info[0].volume == 64, "period error");

	/* Frame 3 -- periods are updated in update_frequency() so it
	 * will appear one frame later **/
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.channel_info[0].period == 4096, "period error");
	fail_unless(info.channel_info[0].volume == 0, "period error");
}
END_TEST
