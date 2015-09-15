#include "test.h"

/*
 Impulse Tracker executes the portamento when switching to instrument two
 on the second row when compatible Gxx is disabled.
*/

TEST(test_openmpt_it_portasample)
{
	compare_mixer_data(
		"openmpt/it/PortaSample.it",
		"openmpt/it/PortaSample.data");
}
END_TEST
