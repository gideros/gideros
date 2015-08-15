#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

/*
Case 1: New note

  Instrument -> None    Same    Valid   Inval
PT1.1           Play    Play    Play    Cut
PT1.3           Play    Play    Play    Cut
PT2.3           Switch  Play    Play    Cut     <=
PT3.15          Switch  Play    Play    Cut     <= "Standard"
PT3.61          Switch  Play    Play    Cut     <=
PT4b2           Switch  Play    Play    Cut     <=
MED             Switch  Play    Play    Cut     <=
FT2             Switch  Play    Play    Cut     <=
ST3             Switch  Play    Play    Switch
IT(s)           Switch  Play    Play    ?
IT(i)           Switch  Play    Play    Cont
DT32            Play    Play    Play    Cut

Play    = Play new note with new default volume
Switch  = Play new note with current volume
NewVol  = Don't play sample, set new default volume
OldVol  = Don't play sample, set old default volume
Cont    = Continue playing sample
Cut     = Stop playing sample

*/

TEST(test_new_note_invalid_ins_it)
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
	new_event(ctx, 0, 1, 0, 50, 3,  0, 0x00, 0, 0, 0);
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

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

	/* Row 1: invalid instrument with new note (IT5)
	 *
	 * When a new invalid instrument and a new note is set, IT
	 * keeps playing the current sample
	 */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "changed note");
	fail_unless(vi->ins  ==  0, "changed instrument");
	fail_unless(vi->vol  == 43 * 16, "changed volume");
	fail_unless(vi->pos0 !=  0, "reset sample");
	xmp_play_frame(opaque);
}
END_TEST
