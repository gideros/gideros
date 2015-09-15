#include "test.h"

/*
 An instrument number should not reset the current portamento target. The
 portamento target is valid until a new target is specified by combining a
 note and a portamento effect.
*/

TEST(test_openmpt_xm_porta_offset)
{
	compare_mixer_data(
		"openmpt/xm/porta-offset.xm",
		"openmpt/xm/porta-offset.data");
}
END_TEST
