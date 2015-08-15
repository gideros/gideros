#include "test.h"

/*
 E1x, E2x, X1x and X2x memory should not be shared. Both channels should
 sound identical if effect memory is applied correctly.
*/

TEST(test_openmpt_xm_porta_linkmem)
{
	compare_mixer_data(
		"openmpt/xm/Porta-LinkMem.xm",
		"openmpt/xm/Porta-LinkMem.data");
}
END_TEST
