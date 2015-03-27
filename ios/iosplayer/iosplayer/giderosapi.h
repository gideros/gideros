#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H

#ifdef __cplusplus
extern "C" {
#endif

void gdr_initialize(UIView* view, int width, int height, bool player);
void gdr_drawFirstFrame();
void gdr_drawFrame();
void gdr_exitGameLoop();
void gdr_deinitialize();
void gdr_suspend();
void gdr_resume();
BOOL gdr_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);
void gdr_willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
void gdr_didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation);
NSUInteger gdr_supportedInterfaceOrientations();
void gdr_didReceiveMemoryWarning();
void gdr_handleOpenUrl(NSURL *url);
void gdr_foreground();
void gdr_background();
void gdr_openProject(NSString* project);
BOOL gdr_isRunning();
    
void gdr_touchesBegan(NSSet* touches, NSSet* allTouches);
void gdr_touchesMoved(NSSet* touches, NSSet* allTouches);
void gdr_touchesEnded(NSSet* touches, NSSet* allTouches);
void gdr_touchesCancelled(NSSet* touches, NSSet* allTouches);

#ifdef __cplusplus
}
#endif

#endif
