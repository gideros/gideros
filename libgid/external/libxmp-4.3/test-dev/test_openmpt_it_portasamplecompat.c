#include "test.h"

/*
 Impulse Tracker executes the portamento and doesn't switch to the new
 sample on the second row when compatible Gxx is enabled.
*/

TEST(test_openmpt_it_portasamplecompat)
{
	compare_mixer_data(
		"openmpt/it/PortaSampleCompat.it",
		"openmpt/it/PortaSampleCompat.data");
}
END_TEST
