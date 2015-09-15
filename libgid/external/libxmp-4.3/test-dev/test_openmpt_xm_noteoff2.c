#include "test.h"

/*
 Various combinations of note-off notes and other things. One of my earliest
 tests...
*/

TEST(test_openmpt_xm_noteoff2)
{
	compare_mixer_data(
		"openmpt/xm/NoteOff2.xm",
		"openmpt/xm/NoteOff2.data");
}
END_TEST
