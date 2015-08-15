#include "test.h"

/*
 This test is an addendum to the previous test (note-limit.xm) because I
 forgot that you first have to check if there is an instrument change
 happening before calculating the “real” note. I always took the previous
 transpose value when doing this check, so when switching from one instrument
 (with a high transpose value) to another one, it was possible that valid
 notes would get rejected.
*/

TEST(test_openmpt_xm_notelimit2)
{
	compare_mixer_data(
		"openmpt/xm/NoteLimit2.xm",
		"openmpt/xm/NoteLimit2.data");
}
END_TEST
