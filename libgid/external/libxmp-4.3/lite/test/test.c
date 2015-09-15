#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"
#include "xmp.h"

static inline int is_big_endian() {
	unsigned short w = 0x00ff;
	return (*(char *)&w == 0x00);
}

/* Convert little-endian 16 bit samples to big-endian */
static void convert_endian(unsigned char *p, int l)
{
	unsigned char b;
	int i;

	for (i = 0; i < l; i++) {
		b = p[0];
		p[0] = p[1];
		p[1] = b;
		p += 2;
	}
}

static int compare_md5(unsigned char *d, char *digest)
{
	int i;

	/*for (i = 0; i < 16 ; i++)
		printf("%02x", d[i]);
	printf("\n");*/

	for (i = 0; i < 16 && *digest; i++, digest += 2) {
		char hex[3];
		hex[0] = digest[0];
		hex[1] = digest[1];
		hex[2] = 0;

		if (d[i] != strtoul(hex, NULL, 16))
			return -1;
	}

	return 0;
}

int main()
{
	int ret;
	xmp_context c;
	struct xmp_frame_info info;
	long time;
	unsigned char digest[16];
	MD5_CTX ctx;

	c = xmp_create_context();
	if (c == NULL)
		goto err;

	ret = xmp_load_module(c, "test.it");
	if (ret != 0) {
		printf("can't load module\n");
		goto err;
	}

	xmp_get_frame_info(c, &info);
	if (info.total_time != 4800) {
		printf("estimated replay time error\n");
		goto err;
	}

	xmp_start_player(c, 22050, 0);
	xmp_set_player(c, XMP_PLAYER_MIX, 100);
	xmp_set_player(c, XMP_PLAYER_INTERP, XMP_INTERP_SPLINE);

	printf("Testing ");
	fflush(stdout);
	time = 0;

	MD5Init(&ctx);

	while (1) {
		xmp_play_frame(c);
		xmp_get_frame_info(c, &info);
		if (info.loop_count > 0)
			break;

		time += info.frame_time;

		if (is_big_endian())
			convert_endian(info.buffer, info.buffer_size >> 1);

		MD5Update(&ctx, info.buffer, info.buffer_size);

		printf(".");
		fflush(stdout);
	}

	MD5Final(digest, &ctx);

	if (compare_md5(digest, "939179b8e87f97df4e6152d5185b59ac") < 0) {
		printf("rendering error\n");
		goto err;
	}

	if (time / 1000 != info.total_time) {
		printf("replay time error\n");
		goto err;
	}

	printf(" pass\n");

	exit(0);

    err:
	printf(" fail\n");
	exit(1);
}
