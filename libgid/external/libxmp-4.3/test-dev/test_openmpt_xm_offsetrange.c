#include "test.h"

/*
 Test to verify the exact behaviour for out-of-range offset commands. Only
 the first sample should be played, because it is the only sample that is
 longer than 256 samples.
*/

TEST(test_openmpt_xm_offsetrange)
{
	compare_mixer_data(
		"openmpt/xm/OffsetRange.xm",
		"openmpt/xm/OffsetRange.data");
}
END_TEST
