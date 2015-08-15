#include "test.h"

/*
 A test focussing on finding the correct sample playback position when
 switching samples “on the fly” (using instrument numbers without notes
 next to them). The module should remain silent when being played.

 Obs: libxmp doesn't keep them silent due to minute phase differences, but
      it's switching instruments correctly so we'll consider it ok.
*/

TEST(test_openmpt_it_swaptest)
{
	compare_mixer_data(
		"openmpt/it/swaptest.it",
		"openmpt/it/swaptest.data");
}
END_TEST
