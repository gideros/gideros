#include "test.h"

/*
 This is a nice test for E6x + Dxx behaviour. First make sure that E6x is
 played correctly by your player. A position jump should not clear the pattern
 loop memory (just like in Impulse Tracker).
*/

/*
 Claudio's note: without looping, xmp plays this as 123-123-123-2 instead of
 the expected 123-123-123-23. It stops before the final 3 because it detects
 that it already passed there and end of module is detected. If looped, it
 correctly plays 123-123-123-23.
*/

TEST(test_openmpt_xm_patloop_break)
{
	compare_mixer_data(
		"openmpt/xm/PatLoop-Break.xm",
		"openmpt/xm/PatLoop-Break.data");
}
END_TEST
