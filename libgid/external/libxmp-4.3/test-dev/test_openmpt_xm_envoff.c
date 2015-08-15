#include "test.h"

/*
 Fixing EnvLoops.xm made OpenMPT cut off envelopes one tick to early in some
 cases, so modules sounded kind of "dry".
*/

TEST(test_openmpt_xm_envoff)
{
	compare_mixer_data("openmpt/xm/EnvOff.xm", "openmpt/xm/EnvOff.data");
}
END_TEST
