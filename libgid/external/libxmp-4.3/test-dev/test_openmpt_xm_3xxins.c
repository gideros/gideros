#include "test.h"

/*
 If a tone portamento effect is encountered, the instrument number next to
 it is always interpreted as the instrument number of the currently playing
 instrument. This test shows how the instrument envelope is reset when such
 an event is encountered.
*/

TEST(test_openmpt_xm_3xxins)
{
	compare_mixer_data("openmpt/xm/3xxins.xm", "openmpt/xm/3xxins.data");
}
END_TEST
