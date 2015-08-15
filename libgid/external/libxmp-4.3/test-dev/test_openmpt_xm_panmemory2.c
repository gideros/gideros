#include "test.h"

/*
 A more thorough check than PanMemory.xm. Both channels should be panned
 identically and the module should thus stay silent.
*/

TEST(test_openmpt_xm_panmemory2)
{
	compare_mixer_data(
		"openmpt/xm/PanMemory2.xm",
		"openmpt/xm/PanMemory2.data");
}
END_TEST
