#include "test.h"

/*
 Another chapter of weird FT2 bugs: Note-Off + Note Delay + Volume Column
 Panning = Panning effect is ignored.
*/

TEST(test_openmpt_xm_panoff)
{
	compare_mixer_data(
		"openmpt/xm/PanOff.xm",
		"openmpt/xm/PanOff.data");
}
END_TEST
