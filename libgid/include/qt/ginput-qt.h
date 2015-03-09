#ifndef _GINPUT_QT_H_
#define _GINPUT_QT_H_

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

G_API void ginputp_mouseDown(int x, int y, int button);
G_API void ginputp_mouseMove(int x, int y);
G_API void ginputp_mouseUp(int x, int y, int button);
G_API void ginputp_touchesBegin(int x, int y, int id, int touches, int xs[], int ys[], int ids[]);
G_API void ginputp_touchesMove(int x, int y, int id, int touches, int xs[], int ys[], int ids[]);
G_API void ginputp_touchesEnd(int x, int y, int id, int touches, int xs[], int ys[], int ids[]);
G_API void ginputp_touchesCancel(int x, int y, int id, int touches, int xs[], int ys[], int ids[]);
G_API void ginputp_keyDown(int keyCode);
G_API void ginputp_keyUp(int keyCode);

#ifdef __cplusplus
}
#endif

#endif
