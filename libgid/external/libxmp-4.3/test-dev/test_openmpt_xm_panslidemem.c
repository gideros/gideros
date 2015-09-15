#include "test.h"

/*
 As the panning slide commands in the volume column don't have memory, they
 should also not interfere with the effect memory of the Pxy command.
*/

TEST(test_openmpt_xm_panslidemem)
{
	compare_mixer_data(
		"openmpt/xm/PanSlideMem.xm",
		"openmpt/xm/PanSlideMem.data");
}
END_TEST
