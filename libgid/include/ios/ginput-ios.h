#ifndef _GINPUT_IOS_H_
#define _GINPUT_IOS_H_

#include <gglobal.h>
#import <UIKit/UIKit.h>

#ifdef __cplusplus
extern "C" {
#endif

G_API void ginputp_touchesBegan(NSSet *touches, NSSet *allTouches, UIView *view);
G_API void ginputp_touchesMoved(NSSet *touches, NSSet *allTouches, UIView *view);
G_API void ginputp_touchesEnded(NSSet *touches, NSSet *allTouchesx, UIView *view);
G_API void ginputp_touchesCancelled(NSSet *touches, NSSet *allTouches, UIView *view);
G_API g_bool ginputp_keyDown(int keyCode, int repeatCount);
G_API g_bool ginputp_keyUp(int keyCode, int repeatCount);
G_API void ginputp_keyChar(const char *keyChar);

#ifdef __cplusplus
}
#endif

#endif

