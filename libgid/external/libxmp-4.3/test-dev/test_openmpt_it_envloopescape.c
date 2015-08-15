#include "test.h"

/*
 I created EnvOffLength.it without realizing that it is essentially the same
 bug as this one.
*/

TEST(test_openmpt_it_envloopescape)
{
	compare_mixer_data(
		"openmpt/it/EnvLoopEscape.it",
		"openmpt/it/EnvLoopEscape.data");
}
END_TEST
