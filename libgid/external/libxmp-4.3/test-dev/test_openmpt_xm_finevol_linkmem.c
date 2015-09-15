#include "test.h"

/*
 EAx and EBx memory should not be shared. If effect memory is applied
 correctly, the module should stay silent.
*/

TEST(test_openmpt_xm_finevol_linkmem)
{
	compare_mixer_data(
		"openmpt/xm/FineVol-LinkMem.xm",
		"openmpt/xm/FineVol-LinkMem.data");
}
END_TEST
