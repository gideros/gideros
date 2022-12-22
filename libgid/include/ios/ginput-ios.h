#ifndef _GINPUT_IOS_H_
#define _GINPUT_IOS_H_

#include <gglobal.h>
#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define UIView NSView
#else
#import <UIKit/UIKit.h>
#endif

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
G_API void ginputp_mouseDown(int x, int y, int button,int mod);
G_API void ginputp_mouseMove(int x, int y, int button,int mod);
G_API void ginputp_mouseHover(int x, int y, int button,int mod);
G_API void ginputp_mouseUp(int x, int y, int button,int mod);
G_API void ginputp_mouseWheel(int x, int y, int button, int delta,int mod);
G_API void ginputp_mouseEnter(int x, int y, int buttons, int mod);
G_API void ginputp_mouseLeave(int x, int y, int mod);

#ifdef __cplusplus
}
#endif

#endif

