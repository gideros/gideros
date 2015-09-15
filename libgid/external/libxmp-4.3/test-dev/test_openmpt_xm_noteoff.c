#include "test.h"

/*
 I think the essence of this test was that a note-off note does not stop
 the sample, it just mutes it...
*/

TEST(test_openmpt_xm_noteoff)
{
	compare_mixer_data(
		"openmpt/xm/NoteOff.xm",
		"openmpt/xm/NoteOff.data");
}
END_TEST
