#include "test.h"

/*
 Two tests in one: An offset effect that points beyond the sample end should
 stop playback on this channel. The note must not be picked up by further
 portamento effects.
*/

TEST(test_openmpt_xm_3xx_no_old_samp)
{
	compare_mixer_data(
		"openmpt/xm/3xx-no-old-samp.xm",
		"openmpt/xm/3xx-no-old-samp.data");
}
END_TEST
