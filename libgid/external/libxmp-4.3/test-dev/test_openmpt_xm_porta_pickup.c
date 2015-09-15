#include "test.h"

/*
 An instrument number should not reset the current portamento target. The
 portamento target is valid until a new target is specified by combining a
 note and a portamento effect.
*/

TEST(test_openmpt_xm_porta_pickup)
{
	compare_mixer_data(
		"openmpt/xm/Porta-Pickup.xm",
		"openmpt/xm/Porta-Pickup.data");
}
END_TEST
