#include "test.h"

/*
 Impulse Tracker internally uses actual frequency values, while Fasttracker 2
 still uses exponentially scaled fine Amiga periods. When doing fine slides,
 errors from using periods instead of frequency can add up very quickly, and
 thus the two channel's frequency in this test will converge noticeably.
*/

TEST(test_openmpt_it_linearslides)
{
	compare_mixer_data(
		"openmpt/it/LinearSlides.it",
		"openmpt/it/LinearSlides.data");
}
END_TEST
