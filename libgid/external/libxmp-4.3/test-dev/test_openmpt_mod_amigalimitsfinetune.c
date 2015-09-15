#include "test.h"

/*
 ProTracker does not always clamp samples to the exact same range of periods
 -- it rather depends on the actual finetune value of the sample. In contrast
 to that, ScreamTracker 3 always clamps periods to the same range in its Amiga
 mode. This test file should stay completely in tune at all times.
*/

TEST(test_openmpt_mod_amigalimitsfinetune)
{
	compare_mixer_data(
		"openmpt/mod/AmigaLimitsFinetune.mod",
		"openmpt/mod/AmigaLimitsFinetune.data");
}
END_TEST
