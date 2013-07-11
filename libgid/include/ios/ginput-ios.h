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

#ifdef __cplusplus
}
#endif

#endif

