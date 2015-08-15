#include "test.h"

/*
 This module should remain completely silent.
*/

TEST(test_openmpt_xm_noteofffade)
{
	compare_mixer_data(
		"openmpt/xm/NoteOffFade.xm",
		"openmpt/xm/NoteOffFade.data");
}
END_TEST
