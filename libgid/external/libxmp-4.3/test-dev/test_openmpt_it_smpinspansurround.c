#include "test.h"

/*
 Sample and instrument panning override the channel surround status, i.e.
 surround is turned off by samples or instruments with panning enabled.
*/

TEST(test_openmpt_it_smpinspansurround)
{
	compare_mixer_data(
		"openmpt/it/SmpInsPanSurround.it",
		"openmpt/it/SmpInsPanSurround.data");
}
END_TEST
