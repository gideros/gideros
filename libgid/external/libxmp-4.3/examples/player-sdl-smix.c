/* A simple frontend for libxmp */
/* This file is in public domain */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <xmp.h>


static int playing;

static void fill_audio(void *udata, Uint8 *stream, int len)
{
	if (xmp_play_buffer((xmp_context)udata, stream, len, 0) < 0)
		playing = 0;
}

static int sdl_init(xmp_context ctx)
{
	SDL_AudioSpec a;

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "sdl: can't initialize: %s\n", SDL_GetError());
		return -1;
	}

	a.freq = 44100;
	a.format = AUDIO_S16;
	a.channels = 2;
	a.samples = 512;
	a.callback = fill_audio;
	a.userdata = ctx;

	if (SDL_OpenAudio(&a, NULL) < 0) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

static void sdl_deinit()
{
	SDL_CloseAudio();
}

static void play_instrument(xmp_context ctx, int ins)
{
	if (xmp_smix_play_instrument(ctx, ins - 1, 60, 64, 0) == 0) {
		printf("play instrument %d\n", ins);
	}
}

int main(int argc, char **argv)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	int i;

	ctx = xmp_create_context();

	if (sdl_init(ctx) < 0) {
		fprintf(stderr, "%s: can't initialize sound\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (xmp_load_module(ctx, argv[i]) < 0) {
			fprintf(stderr, "%s: error loading %s\n", argv[0],
				argv[i]);
			continue;
		}

		if (xmp_start_smix(ctx, 1, 0) == 0 &&
		    xmp_start_player(ctx, 44100, 0) == 0) {

			xmp_set_player(ctx, XMP_PLAYER_VOLUME, 20);
			xmp_set_player(ctx, XMP_PLAYER_SMIX_VOLUME, 100);

			/* Show module data */

			xmp_get_module_info(ctx, &mi);
			printf("%s (%s)\n", mi.mod->name, mi.mod->type);

			/* Play module */

			playing = 1;
			SDL_PauseAudio(0);

			while (playing) {
				int ins;
				printf("Instrument to play: ");
				scanf("%d", &ins);
				if (ins > 0 && ins <= mi.mod->ins) {
					play_instrument(ctx, ins);
				}
			}
			xmp_end_player(ctx);
		}

		xmp_release_module(ctx);
		printf("\n");
	}

	xmp_free_context(ctx);

	sdl_deinit();

	return 0;
}
