#include "test.h"

/* When a tone portamento effect is performed, we expect it to always
 * reset the envelope if the previous envelope already ended. If it
 * still didn't end, reset envelope in XM (and IT compatible GXX mode)
 * but not in standard IT mode
 */
TEST(test_player_it_portamento_envelope_reset_cg)
{
	compare_mixer_data(
		"data/it_portamento_envelope_reset_cg.it",
		"data/it_portamento_envelope_reset_cg.data");
}
END_TEST
