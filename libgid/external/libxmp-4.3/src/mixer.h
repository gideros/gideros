#ifndef LIBXMP_MIXER_H
#define LIBXMP_MIXER_H

#define SMIX_C4NOTE	6864

#define SMIX_NUMVOC	128	/* default number of softmixer voices */
#define SMIX_SHIFT	16
#define SMIX_MASK	0xffff

#define FILTER_SHIFT	16

/* Anticlick ramps */
#define SLOW_ATTACK_SHIFT 4
#define SLOW_ATTACK	(1 << SLOW_ATTACK_SHIFT)
#define SLOW_RELEASE	16


struct mixer_voice {
	int chn;		/* channel number */
	int root;		/* */
	unsigned int age;	/* */
	int note;		/* */
#define PAN_SURROUND 0x8000
	int pan;		/* */
	int vol;		/* */
	int period;		/* current period */
	unsigned int pos;	/* position in sample */
	int pos0;		/* position in sample before mixing */
	int frac;		/* interpolation */
	int fidx;		/* function index */
	int ins;		/* instrument number */
	int smp;		/* sample number */
	int end;		/* loop end */
	int act;		/* nna info & status of voice */
	int sleft;		/* last left sample output, in 32bit */
	int sright;		/* last right sample output, in 32bit */
	void *sptr;		/* sample pointer */

	struct {
		int r1;		/* filter variables */
		int r2;
		int l1;
		int l2;
		int a0;
		int b0;
		int b1;
		int cutoff;
		int resonance;
	} filter;

	int attack;		/* ramp up anticlick */
	int sample_loop;	/* set if sample has looped */
};

int	mixer_on		(struct context_data *, int, int, int);
void	mixer_off		(struct context_data *);
void    mixer_setvol		(struct context_data *, int, int);
void    mixer_seteffect		(struct context_data *, int, int, int);
void    mixer_setpan		(struct context_data *, int, int);
int	mixer_numvoices		(struct context_data *, int);
void	mixer_softmixer		(struct context_data *);
void	mixer_reset		(struct context_data *);
void	mixer_setpatch		(struct context_data *, int, int);
void	mixer_voicepos		(struct context_data *, int, int, int);
int	mixer_getvoicepos	(struct context_data *, int);
void	mixer_setnote		(struct context_data *, int, int);
void	mixer_setbend		(struct context_data *, int, int);

#endif /* LIBXMP_MIXER_H */
