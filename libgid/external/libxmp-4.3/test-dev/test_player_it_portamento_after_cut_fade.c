#include "test.h"

TEST(test_player_it_portamento_after_cut_fade)
{
	compare_mixer_data(
		"data/portamento_after_cut_fade.it",
		"data/portamento_after_cut_fade.data");
}
END_TEST
