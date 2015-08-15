#include "test.h"

/*
 When using the Lxx effect, Fasttracker 2 only sets the panning envelope
 position if the volume envelope’s sustain flag is set.
*/

TEST(test_openmpt_xm_setenvpos)
{
	compare_mixer_data(
		"openmpt/xm/SetEnvPos.xm",
		"openmpt/xm/SetEnvPos.data");
}
END_TEST
