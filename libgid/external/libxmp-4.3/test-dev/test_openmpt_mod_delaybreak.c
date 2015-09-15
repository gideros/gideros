#include "test.h"

/*
 If there is a row delay (EEx) on the same row as a pattern break (Dxx), the
 target row of that jump is not played. Instead, the next row is played.
*/

TEST(test_openmpt_mod_delaybreak)
{
	compare_mixer_data(
		"openmpt/mod/DelayBreak.mod",
		"openmpt/mod/DelayBreak.data");
}
END_TEST
