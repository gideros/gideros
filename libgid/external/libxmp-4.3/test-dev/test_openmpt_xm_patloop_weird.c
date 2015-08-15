#include "test.h"

/*
 This is similar to PatLoop-Break.xm. The voice should say "1 4 2" and then
 repeat "3 4 2" forever.
*/

TEST(test_openmpt_xm_patloop_weird)
{
	compare_mixer_data_loops(
		"openmpt/xm/PatLoop-Weird.xm",
		"openmpt/xm/PatLoop-Weird.data", 4);
}
END_TEST
