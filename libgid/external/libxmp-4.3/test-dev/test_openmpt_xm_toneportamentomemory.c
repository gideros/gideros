#include "test.h"

/*
 Tone portamento is actually the only volume column command with an effect
 memory (as it is shared with the other effect column). Another nice bug
 demonstrated in this module is the combination of both portamento commands
 (Mx and 3xx) in the same cell: The 3xx parameter is ignored completely, and
 the Mx parameter is doubled, i.e. M2 3FF is the same as M4 000.
*/

TEST(test_openmpt_xm_toneportamentomemory)
{
	compare_mixer_data(
		"openmpt/xm/TonePortamentoMemory.xm",
		"openmpt/xm/TonePortamentoMemory.data");
}
END_TEST
