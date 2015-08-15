#include "test.h"

/*
 In sample mode, sample is retriggered even with tone portamento if the
 previous sample is finished. See UNATCOReturn_music.it sample 12 in
 pattern 62 (reported by Alexander Null)
*/

TEST(test_player_it_sample_porta)
{
	compare_mixer_data(
		"data/it_sample_porta.it",
		"data/it_sample_porta.data");
}
END_TEST
