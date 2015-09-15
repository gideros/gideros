#ifndef LIBXMP_VIRTUAL_H
#define LIBXMP_VIRTUAL_H

#include "common.h"

#define VIRT_ACTION_CUT		XMP_INST_NNA_CUT
#define VIRT_ACTION_CONT	XMP_INST_NNA_CONT
#define VIRT_ACTION_OFF		XMP_INST_NNA_OFF
#define VIRT_ACTION_FADE	XMP_INST_NNA_FADE

#define VIRT_ACTIVE		0x100
#define VIRT_INVALID		-1

int	virt_on			(struct context_data *, int);
void	virt_off		(struct context_data *);
int	virt_mute		(struct context_data *, int, int);
int	virt_setpatch		(struct context_data *, int, int, int, int,
				 int, int, int);
int	virt_cvt8bit		(void);
void	virt_setnote		(struct context_data *, int, int);
void	virt_setsmp		(struct context_data *, int, int);
void	virt_setnna		(struct context_data *, int, int);
void	virt_pastnote		(struct context_data *, int, int);
void	virt_setvol		(struct context_data *, int, int);
void	virt_voicepos		(struct context_data *, int, int);
int	virt_getvoicepos	(struct context_data *, int);
void	virt_setbend		(struct context_data *, int, int);
void	virt_setpan		(struct context_data *, int, int);
void	virt_seteffect		(struct context_data *, int, int, int);
int	virt_cstat		(struct context_data *, int);
int	virt_mapchannel		(struct context_data *, int);
void	virt_resetchannel	(struct context_data *, int);
void	virt_resetvoice		(struct context_data *, int, int);
void	virt_reset		(struct context_data *);
int	virt_getroot		(struct context_data *, int);

#endif /* LIBXMP_VIRTUAL_H */
