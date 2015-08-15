#include "test.h"

/*
 Note delays retrigger envelopes.
*/

TEST(test_openmpt_xm_envretrig)
{
	compare_mixer_data(
		"openmpt/xm/envretrig.xm",
		"openmpt/xm/envretrig.data");
}
END_TEST
