#include "test.h"

/*
 The E90 effect does not use any effect memory. Instead, it retriggers the
 note on the first tick.
*/

TEST(test_openmpt_xm_e90)
{
	compare_mixer_data("openmpt/xm/E90.xm", "openmpt/xm/E90.data");
}
END_TEST
