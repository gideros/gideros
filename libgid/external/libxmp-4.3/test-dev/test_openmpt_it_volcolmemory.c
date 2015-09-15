#include "test.h"

/*
 Volume column commands a, b, c and d (volume slide) share one effect memory,
 but it should not be shared with Dxy in the effect column. Furthermore, there
 is no unified effect memory across different kinds of volume column effects
 (that's how OpenMPT used to handle it up to revision 1544).
*/

TEST(test_openmpt_it_volcolmemory)
{
	compare_mixer_data(
		"openmpt/it/VolColMemory.it",
		"openmpt/it/VolColMemory.data");
}
END_TEST
