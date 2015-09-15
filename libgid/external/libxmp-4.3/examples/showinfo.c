/* Display module info using libxmp */
/* This file is in public domain */

#include <stdio.h>
#include <stdlib.h>
#include <xmp.h>


static void display_info(struct xmp_module_info *mi)
{
	int i, j;
	struct xmp_module *mod = mi->mod;

	printf("Name: %s\n", mod->name);
	printf("Type: %s\n", mod->type);
	printf("Number of patterns: %d\n", mod->pat);
	printf("Number of tracks: %d\n", mod->trk);
	printf("Number of channels: %d\n", mod->chn);
	printf("Number of instruments: %d\n", mod->ins);
	printf("Number of samples: %d\n", mod->smp);
	printf("Initial speed: %d\n", mod->spd);
	printf("Initial BPM: %d\n", mod->bpm);
	printf("Length in patterns: %d\n", mod->len);

	printf("\n");

	printf("Instruments:\n");
	for (i = 0; i < mod->ins; i++) {
		struct xmp_instrument *ins = &mod->xxi[i];

		printf("%02x %-32.32s V:%02x R:%04x %c%c%c\n",
				i, ins->name, ins->vol, ins->rls,
				ins->aei.flg & XMP_ENVELOPE_ON ? 'A' : '-',
				ins->pei.flg & XMP_ENVELOPE_ON ? 'P' : '-',
				ins->fei.flg & XMP_ENVELOPE_ON ? 'F' : '-'); 

		for (j = 0; j < ins->nsm; j++) {
			struct xmp_subinstrument *sub = &ins->sub[j];
			printf("   %02x V:%02x GV:%02x P:%02x X:%+04d F:%+04d\n",
					j, sub->vol, sub->gvl, sub->pan,
					sub->xpo, sub->fin);
		}
	}

	printf("\n");

	printf("Samples:\n");
	for (i = 0; i < mod->smp; i++) {
		struct xmp_sample *smp = &mod->xxs[i];

		printf("%02x %-32.32s %05x %05x %05x %c%c%c%c%c%c",
				i, smp->name, smp->len, smp->lps, smp->lpe,
				smp->flg & XMP_SAMPLE_16BIT ? 'W' : '-',
				smp->flg & XMP_SAMPLE_LOOP ? 'L' : '-',
				smp->flg & XMP_SAMPLE_LOOP_BIDIR ? 'B' : '-',
				smp->flg & XMP_SAMPLE_LOOP_REVERSE ? 'R' : '-',
				smp->flg & XMP_SAMPLE_LOOP_FULL ? 'F' : '-',
				smp->flg & XMP_SAMPLE_SYNTH ? 'S' : '-');

		if (smp->len > 0 && smp->lpe >= smp->len) {
			printf(" LOOP ERROR");
		}

		printf("\n");
	}
}


int main(int argc, char **argv)
{
	static xmp_context ctx;
	static struct xmp_module_info mi;
	int i;

	ctx = xmp_create_context();

	for (i = 1; i < argc; i++) {
		if (xmp_load_module(ctx, argv[i]) < 0) {
			fprintf(stderr, "%s: error loading %s\n", argv[0],
				argv[i]);
			continue;
		}

		if (xmp_start_player(ctx, 44100, 0) == 0) {
			xmp_get_module_info(ctx, &mi);
			display_info(&mi);
			xmp_end_player(ctx);
		}

		xmp_release_module(ctx);
	}

	xmp_free_context(ctx);

	return 0;
}
