#include "test.h"

/*
 Portamento with funny sample maps, in compatible Gxx mode. With compatible
 Gxx, portamento between different samples should keep playing the old sample.
*/

TEST(test_openmpt_it_portainsnumcompat)
{
	compare_mixer_data(
		"openmpt/it/PortaInsNumCompat.it",
		"openmpt/it/PortaInsNumCompat.data");
}
END_TEST
