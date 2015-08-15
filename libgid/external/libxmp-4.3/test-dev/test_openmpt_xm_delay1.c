#include "test.h"

/*
 A fun test for rogue note delays that outputs a nice drum beat if played
 correctly. Combinations of note delays with and without instrument number
 are tested.
*/

TEST(test_openmpt_xm_delay1)
{
	compare_mixer_data("openmpt/xm/delay1.xm", "openmpt/xm/delay1.data");
}
END_TEST
