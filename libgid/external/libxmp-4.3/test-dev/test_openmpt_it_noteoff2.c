#include "test.h"

/*
 The sample changes on rows 4 and 20, but not on rows 8 and 24.

 Claudio's note: don't use compare_mixer_data, we have random pan here.
*/

TEST(test_openmpt_it_noteoff2)
{
	compare_mixer_data(
		"openmpt/it/noteoff2.it",
		"openmpt/it/noteoff2.data");
}
END_TEST
