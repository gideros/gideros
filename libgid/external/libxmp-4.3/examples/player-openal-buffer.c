/* A simple frontend for libxmp */
/* This file is in public domain */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <xmp.h>

static ALCdevice *al_dev;
static ALCcontext *al_ctx;

/* The number of buffers should be two or more, and the buffer
 * size should be a multiple of the frame size (by default, OpenAL's
 * largest frame size is 4, however extensions that can add more formats
 * may be larger). Slower systems may need more buffers/larger buffer
 * sizes. */
#define NUM_BUFFERS 6
#define BUFFER_SIZE 4096

ALuint source, buffers[NUM_BUFFERS];


static int openal_init()
{
	al_dev = alcOpenDevice(NULL);
	if (al_dev == NULL)
		return -1;

	al_ctx = alcCreateContext(al_dev, NULL);
	if (al_ctx == NULL)
		return -1;

	alcMakeContextCurrent(al_ctx);

	alGenBuffers(NUM_BUFFERS, buffers);
	alGenSources(1, &source);
	if (alGetError() != AL_NO_ERROR)
		return -1;

	return 0;
}

static void openal_deinit()
{
	alDeleteSources(1, &source);
	alDeleteBuffers(NUM_BUFFERS, buffers);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(al_ctx);
	alcCloseDevice(al_dev);
}

static void display_data(struct xmp_module_info *mi, struct xmp_frame_info *fi)
{
	printf("%3d/%3d %3d/%3d\r",
	       fi->pos, mi->mod->len, fi->row, fi->num_rows);

	fflush(stdout);
}


int main(int argc, char **argv)
{
	xmp_context ctx;
	struct xmp_module_info mi;
	struct xmp_frame_info fi;
	int row, i;
	char *b;

	if (openal_init() < 0) {
		fprintf(stderr, "%s: can't initialize sound\n", argv[0]);
		exit(1);
	}

	if ((b = malloc(BUFFER_SIZE)) == NULL) {
		fprintf(stderr, "%s: can't allocate buffer\n", argv[0]);
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
			ALuint fmt = AL_FORMAT_STEREO16;
			ALuint freq = 44100;

			/* Show module data */

			xmp_get_module_info(ctx, &mi);
			printf("%s (%s)\n", mi.mod->name, mi.mod->type);

			/* Play module */

			row = -1;

			/* Fill OpenAL buffers */
			for (i = 0; i < NUM_BUFFERS; i++) {
				xmp_play_buffer(ctx, b, BUFFER_SIZE, 0);
				alBufferData(buffers[i], fmt, b,
						BUFFER_SIZE, freq);
			}

			alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
			alSourcePlay(source);
			if (alGetError() != AL_NO_ERROR) {
				fprintf(stderr, "Failed miserably\n");
				exit(1);
			}

			while (xmp_play_buffer(ctx, b, BUFFER_SIZE, 0) == 0) {
				ALuint buffer;
				ALint val;

				alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
				while (val <= 0) {
					usleep(10000);
					alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
				}

				xmp_get_frame_info(ctx, &fi);

				alSourceUnqueueBuffers(source, 1, &buffer);
				alBufferData(buffer, fmt, b, BUFFER_SIZE, freq);
				alSourceQueueBuffers(source, 1, &buffer);
				if (alGetError() != AL_NO_ERROR)
					break;
				
				alGetSourcei(source, AL_SOURCE_STATE, &val);
				if (val != AL_PLAYING)
					alSourcePlay(source);
			
				if (fi.row != row) {
					display_data(&mi, &fi);
					row = fi.row;
				}
			}
			xmp_end_player(ctx);
		}

		xmp_release_module(ctx);
		printf("\n");
	}

	xmp_free_context(ctx);

	openal_deinit();

	return 0;
}
