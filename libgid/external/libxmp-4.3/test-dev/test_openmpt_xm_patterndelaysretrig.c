#include "test.h"

/*
 Only the very first tick of a row should be considered as the “first tick”,
 even if the row is repeated multiple times using the pattern delay (EEx)
 command (i.e. multiples of the song speed should not be considered as the
 first tick). This is shown in this test by using the extra-fine portamento
 commands, which are only executed on the first tick.
*/

TEST(test_openmpt_xm_patterndelaysretrig)
{
	compare_mixer_data(
		"openmpt/xm/PatternDelaysRetrig.xm",
		"openmpt/xm/PatternDelaysRetrig.data");
}
END_TEST
