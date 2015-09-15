#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

/*
Case 3: Tone portamento

  Instrument -> None    Same    Valid   Inval
PT1.1           Cont            NewVol?
PT1.3           Cont    NewVol  NewVol* Cut
PT2.3           Cont    NewVol  NewVol* Cut
PT3.15          Cont    NewVol  NewVol  Cut     <= "Standard"
PT3.61          Cont    NewVol  NewVol  Cut     <=
PT4b2           Cont    NewVol  NewVol  Cut     <=
MED             Cont    NewVol  NewVol  Cut     <=
FT2             Cont    OldVol  OldVol  OldVol
ST3             Cont    NewVol  NewVol  Cont
IT(s)           Cont    NewVol  NewVol  Cont
IT(i) @         Cont    NewVol  NewVol  Cont
DT32            Cont    NewVol  NewVol  Cut

*/

TEST(test_porta_same_ins_mod)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 22);
	set_instrument_volume(ctx, 1, 0, 33);
	new_event(ctx, 0, 0, 0, 60, 1, 44, 0x0f, 2, 0, 0);
	new_event(ctx, 0, 1, 0, 50, 1,  0, 0x03, 4, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	/* Row 0 */
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol  == 43 * 16, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);

	/* Row 1: same instrument with tone portamento (PT 3.15)
	 *
	 * When the same instrument as the current instrument is played
	 * with tone portamento, PT3.15 keeps playing the current sample but
	 * sets the volume to the new instrument's default volume.
	 */
	xmp_play_frame(opaque);
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->vol  == 22 * 16, "not instrument volume");
	fail_unless(vi->pos0 !=  0, "sample reset");
}
END_TEST
