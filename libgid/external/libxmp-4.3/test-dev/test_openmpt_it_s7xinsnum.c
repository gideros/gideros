#include "test.h"

/*
 Changing the NNA action through the S7x command only affects the current
 note - The NNA action is reset on every note change, and not on every
 instrument change.
*/

TEST(test_openmpt_it_s7xinsnum)
{
	compare_mixer_data(
		"openmpt/it/s7xinsnum.it",
		"openmpt/it/s7xinsnum.data");
}
END_TEST
