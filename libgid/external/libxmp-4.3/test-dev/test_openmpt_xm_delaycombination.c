#include "test.h"

/*
 Naturally, Fasttracker 2 ignores notes next to an out-of-range note delay.
 However, to check whether the delay is out of range, it is simply compared
 against the current song speed, not taking any pattern delays into account.
 No notes should be triggered in this test case, even though the second row
 is technically longer than six ticks.
*/

TEST(test_openmpt_xm_delaycombination)
{
	compare_mixer_data(
		"openmpt/xm/DelayCombination.xm",
		"openmpt/xm/DelayCombination.data");
}
END_TEST
