#include "test.h"

/*
 This seems to be related to EnvOff.xm. Sound output should never go
 completely silent between the notes.
*/

TEST(test_openmpt_xm_pickup)
{
	compare_mixer_data(
		"openmpt/xm/Pickup.xm",
		"openmpt/xm/Pickup.data");
}
END_TEST
