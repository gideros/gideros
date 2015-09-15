#include "test.h"

TEST(test_player_it_portamento_after_keyoff)
{
	compare_mixer_data(
		"data/portamento_after_keyoff.it",
		"data/portamento_after_keyoff.data");
}
END_TEST
