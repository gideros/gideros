/* Creates a simple module */

#include "../src/common.h"
#include "../src/loaders/loader.h"

void load_prologue(struct context_data *);
void load_epilogue(struct context_data *);

void create_simple_module(struct context_data *ctx, int ins, int pat)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	int i;

	load_prologue(ctx);

	/* Create module */

	mod->len = 2;
	mod->pat = pat;
	mod->ins = ins;
	mod->chn = 4;
	mod->trk = mod->pat * mod->chn;
	mod->smp = mod->ins;
	mod->xxo[0] = 0;
	mod->xxo[1] = 1;

	pattern_init(mod);

	for (i = 0; i < mod->pat; i++) {
		pattern_tracks_alloc(mod, i, 64);
	}

	instrument_init(mod);

	for (i = 0; i < mod->ins; i++) {
		mod->xxi[i].nsm = 1;
		subinstrument_alloc(mod, i, 1);

		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].vol = 0x40;
		mod->xxi[i].sub[0].sid = i;

		mod->xxs[i].len = 10000;
		mod->xxs[i].lps = 0;
		mod->xxs[i].lpe = 10000;
		mod->xxs[i].flg = XMP_SAMPLE_LOOP;
		mod->xxs[i].data = calloc(1, 11000);
		mod->xxs[i].data += 4;
	}

	/* End of module creation */

	load_epilogue(ctx);
	prepare_scan(ctx);
	scan_sequences(ctx);

	ctx->state = XMP_STATE_LOADED;
}

void new_event(struct context_data *ctx, int pat, int row, int chn, int note, int ins, int vol, int fxt, int fxp, int f2t, int f2p)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct xmp_event *e;
	int track;

	track = mod->xxp[pat]->index[chn];
	e = &mod->xxt[track]->event[row];

	e->note = note;
	e->ins = ins;
	e->vol = vol;
	e->fxt = fxt;
	e->fxp = fxp;
	e->f2t = f2t;
	e->f2p = f2p;
}

void set_order(struct context_data *ctx, int pos, int pat)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxo[pos] = pat;
	mod->len = pos + 1;
}

void set_instrument_volume(struct context_data *ctx, int ins, int sub, int vol)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxi[ins].sub[sub].vol = vol;
}

void set_instrument_nna(struct context_data *ctx, int ins, int sub,
			int nna, int dct, int dca)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxi[ins].sub[sub].nna = nna;
	mod->xxi[ins].sub[sub].dct = dct;
	mod->xxi[ins].sub[sub].dca = dca;
}

void set_instrument_envelope(struct context_data *ctx, int ins, int node,
			     int x, int y)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxi[ins].aei.data[node * 2] = x;
	mod->xxi[ins].aei.data[node * 2 + 1] = y;

	mod->xxi[ins].aei.npt = node + 1;
	mod->xxi[ins].aei.flg |= XMP_ENVELOPE_ON;
}

void set_instrument_envelope_sus(struct context_data *ctx, int ins, int sus)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxi[ins].aei.sus = sus;
	mod->xxi[ins].aei.sue = sus;
	mod->xxi[ins].aei.flg |= XMP_ENVELOPE_SUS | XMP_ENVELOPE_SLOOP;
}

void set_instrument_fadeout(struct context_data *ctx, int ins, int fade)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	mod->xxi[ins].rls = fade;
}

void set_quirk(struct context_data *ctx, int quirk, int read_mode)
{
	struct module_data *m = &ctx->m;

	m->quirk |= quirk;
	m->read_event_type = read_mode;
}

void reset_quirk(struct context_data *ctx, int quirk)
{
	struct module_data *m = &ctx->m;

	m->quirk &= ~quirk;
}
