#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H


#import <UIKit/UIKit.h>

@protocol EAGLViewProtocol
	- (void)setFramebuffer;
	- (BOOL)presentFramebuffer;
	- (void)enableRetinaDisplay:(BOOL)enable;
@end


void gdr_initialize(UIView* view, int width, int height, bool player);
void gdr_drawFrame();
void gdr_resize(int width, int height);
void gdr_exitGameLoop();
void gdr_deinitialize();
void gdr_suspend();
void gdr_resume();
BOOL gdr_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);
void gdr_willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
void gdr_didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation); 
void gdr_didReceiveMemoryWarning();

void gdr_touchesBegan(NSSet* touches, NSSet* allTouches);
void gdr_touchesMoved(NSSet* touches, NSSet* allTouches);
void gdr_touchesEnded(NSSet* touches, NSSet* allTouches);
void gdr_touchesCancelled(NSSet* touches, NSSet* allTouches);

void gdr_setAccelerometer(double x, double y, double z);

#endif
