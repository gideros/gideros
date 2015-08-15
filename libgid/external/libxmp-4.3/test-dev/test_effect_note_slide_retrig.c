#include "test.h"
#include "../src/effects.h"
#include "../src/mixer.h"
#include "../src/virtual.h"
#include <math.h>

/*
Periodtable for Tuning 0, Normal
  C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
  C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
  C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113

Amiga limits: 907 to 108
*/

#define PERIOD ((int)round(1.0 * info.channel_info[0].period / 4096))

static int vals[] = {
	856, 856, 856, 808, 808, 808, 762,
	762, 762, 720, 720, 720, 679, 679,
	679, 679, 679, 641, 641, 641, 605,
	605, 605, 605, 539, 539, 539, 480,
	480, 480, 428, 428, 381, 381, 340,
	340, 286, 240, 202, 170, 143, 120,

	120, 120, 120, 127, 127, 127, 135,
	135, 135, 143, 143, 143, 151, 151,
	151, 151, 151, 160, 160, 160, 170,
	170, 170, 170, 191, 191, 191, 214,
	214, 214, 240, 240, 270, 270, 303,
	303, 360, 428, 509, 605, 720, 856
};

static int vals2[] = {
	1, 0, 0, 1, 0, 0, 1,
	0, 0, 1, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1,
	0, 0, 0, 1, 0, 0, 1,
	0, 0, 1, 0, 1, 0, 1,
	0, 1, 1, 1, 1, 1, 1,

	0, 0, 0, 1, 0, 0, 1,
	0, 0, 1, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1,
	0, 0, 0, 1, 0, 0, 1,
	0, 0, 1, 0, 1, 0, 1,
	0, 1, 1, 1, 1, 1, 1
};

TEST(test_effect_note_slide_retrig)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	struct xmp_frame_info info;
	int i, j, voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 49, 1, 0, FX_NSLIDE_R_UP, 0x31, FX_SPEED, 7);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_NSLIDE_R_UP, 0x00, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_NSLIDE_R_UP, 0x31, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_NSLIDE_R_UP, 0x32, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_NSLIDE_R_UP, 0x22, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_NSLIDE_R_UP, 0x13, 0, 0);

	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x31, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x00, 0, 0);
	new_event(ctx, 0, 8, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x31, 0, 0);
	new_event(ctx, 0, 9, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x32, 0, 0);
	new_event(ctx, 0, 10, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x22, 0, 0);
	new_event(ctx, 0, 11, 0, 0, 0, 0, FX_NSLIDE_R_DN, 0x13, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 12; i++) {
		for (j = 0; j < 7; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);

			voc = map_channel(p, 0);
       		 	fail_unless(voc >= 0, "virtual map");
       		 	vi = &p->virt.voice_array[voc];
		
			fail_unless(PERIOD == vals[i * 7 + j], "note slide error");
			if (vals2[i * 7 + j])
				fail_unless(vi->pos0 == 0, "sample position");
			else
				fail_unless(vi->pos0 != 0, "sample position");
		}
	}
}
END_TEST
