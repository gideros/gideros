#include "test.h"

/*
 Even more idiosyncrasies with the EDx command, this time in combination
 with note offs.
*/

TEST(test_openmpt_xm_delaycut)
{
	compare_mixer_data(
		"openmpt/xm/delaycut.xm",
		"openmpt/xm/delaycut.data");
}
END_TEST
