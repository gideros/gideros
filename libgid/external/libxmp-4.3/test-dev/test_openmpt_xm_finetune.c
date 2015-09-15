#include "test.h"

/*
 A test for the E5x finetune command. Behaviour is very different from the
 MOD format - first off, E50 is the lowest finetuning and E5F is the highest.
 E5x is only applied if there is a real note next to the command (this
 simplifies a few things).
*/

TEST(test_openmpt_xm_finetune)
{
	compare_mixer_data(
		"openmpt/xm/finetune.xm",
		"openmpt/xm/finetune.data");
}
END_TEST
