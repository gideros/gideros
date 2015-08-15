#include "test.h"

/*
 ProTracker 1/2 has a slightly buggy offset implementation which adds the
 offset in two different places (according to tracker_notes.txt coming with
 libxmp: "once before playing this instrument (as is expected), and once
 again after this instrument has been played"). The left and right channel
 of this module should sound identical. OpenMPT emulates this behaviour
 correctly in ProTracker mode. 
*/

TEST(test_openmpt_mod_ptoffset)
{
	compare_mixer_data(
		"openmpt/mod/ptoffset.mod",
		"openmpt/mod/ptoffset.data");
}
END_TEST
