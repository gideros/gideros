#include "test.h"

/*
 All notes in this test should be panned hard right.
*/

TEST(test_openmpt_xm_panmemory)
{
	compare_mixer_data(
		"openmpt/xm/PanMemory.xm",
		"openmpt/xm/PanMemory.data");
}
END_TEST
