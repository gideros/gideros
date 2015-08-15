#include "test.h"

/*
 More K00 fun! You should ignore any note or instrument data next to a K00
 effect. Probably the best thing to do is to wipe them from your internal
 channel memory in such a situation.
*/

TEST(test_openmpt_xm_keyoff2)
{
	compare_mixer_data(
		"openmpt/xm/KeyOff2.xm",
		"openmpt/xm/KeyOff2.data");
}
END_TEST
