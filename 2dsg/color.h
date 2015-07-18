#ifndef COLOR_H
#define COLOR_H

void glPushColor();
void glPopColor();
void glMultColor(float r, float g, float b, float a);
void glSetColor(float r, float g, float b, float a);
void glGetColor(float *r, float *g, float *b, float *a);
int colorStackSize();

#endif
