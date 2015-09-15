#include "test.h"

TEST(test_player_it_portamento_after_cut_fade_cg)
{
	compare_mixer_data(
		"data/portamento_after_cut_fade_cg.it",
		"data/portamento_after_cut_fade_cg.data");
}
END_TEST
