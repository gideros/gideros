/* A simple frontend for libxmp using fixed-size buffers */
/* This file is in public domain */

#include <stdio.h>
#include <stdlib.h>
#include <xmp.h>
#include "sound.h"


#define BUFFER_SIZE 5000
static char buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	int i;

	if (sound_init(44100, 2) < 0) {
		fprintf(stderr, "%s: can't initialize sound\n", argv[0]);
		exit(1);
	}

	ctx = xmp_create_context();

	for (i = 1; i < argc; i++) {
		if (xmp_load_module(ctx, argv[i]) < 0) {
			fprintf(stderr, "%s: error loading %s\n", argv[0],
				argv[i]);
			continue;
		}

		if (xmp_start_player(ctx, 44100, 0) == 0) {

			/* Show module data */

			xmp_get_module_info(ctx, &mi);
			printf("%s (%s)\n", mi.mod->name, mi.mod->type);

			/* Play module */

			while (xmp_play_buffer(ctx, buffer, BUFFER_SIZE, 1) == 0) {
				sound_play(buffer, BUFFER_SIZE);
			}
			xmp_end_player(ctx);
		}

		xmp_release_module(ctx);
	}

	xmp_free_context(ctx);

	sound_deinit();

	return 0;
}
