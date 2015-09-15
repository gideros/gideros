#include "test.h"

/*
 Unlike fine volume slides in the effect column, fine volume slides in the
 volume column are only ever executed on the first tick — not on multiples of
 the first tick if there is a pattern delay. Thus, the left and right channel
 of this example should always have the same volume.
*/

TEST(test_openmpt_it_finevolcolslide)
{
	compare_mixer_data(
		"openmpt/it/FineVolColSlide.it",
		"openmpt/it/FineVolColSlide.data");
}
END_TEST
