#ifndef XMP_EXTRAS_H
#define XMP_EXTRAS_H

void release_module_extras(struct context_data *);

int new_channel_extras(struct context_data *, struct channel_data *);
void release_channel_extras(struct context_data *, struct channel_data *);
void reset_channel_extras(struct context_data *, struct channel_data *);

void play_extras(struct context_data *, struct channel_data *, int);

int extras_get_volume(struct context_data *, struct channel_data *);
int extras_get_period(struct context_data *, struct channel_data *);
int extras_get_linear_bend(struct context_data *, struct channel_data *);
void extras_process_fx(struct context_data *, struct channel_data *, int, uint8, uint8, uint8, int);


/* FIXME */
void med_hold_hack(struct context_data *ctx, int, int, int);

#endif
