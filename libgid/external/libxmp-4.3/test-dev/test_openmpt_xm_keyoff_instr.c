#include "test.h"

/*
 Apparently, any note number in a pattern causes instruments to recall their
 original sample settings (volume, panning, ...) - no matter if there's a Note
 Off next to it or whatever. In the first pattern, the panning of the both
 channels should always be opposed.
*/

TEST(test_openmpt_xm_keyoff_instr)
{
	compare_mixer_data(
		"openmpt/xm/keyoff+instr.xm",
		"openmpt/xm/keyoff+instr.data");
}
END_TEST
