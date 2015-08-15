#include "test.h"

/*
 Portamento with funny sample maps. Without compatible Gxx, portamento
 between different samples should play the new sample.
*/

TEST(test_openmpt_it_portainsnum)
{
	compare_mixer_data(
		"openmpt/it/PortaInsNum.it",
		"openmpt/it/PortaInsNum.data");
}
END_TEST
