#include "test.h"

/*
 If an envelope sustain loop happens to end on exactly the same tick as a
 note-off event occurs, the envelope is not yet released. It will be released
 whenever the loop end is being hit again.
*/

TEST(test_openmpt_it_envofflength)
{
	compare_mixer_data(
		"openmpt/it/EnvOffLength.it",
		"openmpt/it/EnvOffLength.data");
}
END_TEST
