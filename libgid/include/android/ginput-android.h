#ifndef _GINPUT_ANDROID_H_
#define _GINPUT_ANDROID_H_

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

G_API void ginputp_touchBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
G_API void ginputp_touchesMove(int size, int *id, int *x, int *y, float *pressure);
G_API void ginputp_touchEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
G_API void ginputp_touchesCancel(int size, int *id, int *x, int *y, float *pressure);
G_API g_bool ginputp_keyDown(int keyCode, int repeatCount);
G_API g_bool ginputp_keyUp(int keyCode, int repeatCount);
G_API void ginputp_keyChar(const char *keyChar);

#ifdef __cplusplus
}
#endif

#endif

