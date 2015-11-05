#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H

#if TARGET_OS_TV==0
#undef TARGET_OS_TV
#endif

#ifdef __cplusplus
extern "C" {
#endif

void gdr_initialize(UIView* view, int width, int height, bool player);
void gdr_surfaceChanged(int width,int height);
void gdr_drawFirstFrame();
void gdr_drawFrame();
void gdr_exitGameLoop();
void gdr_deinitialize();
void gdr_suspend();
void gdr_resume();
#ifndef TARGET_OS_TV
BOOL gdr_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);
void gdr_willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
void gdr_didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation);
NSUInteger gdr_supportedInterfaceOrientations();
#endif
void gdr_didReceiveMemoryWarning();
void gdr_handleOpenUrl(NSURL *url);
void gdr_foreground();
void gdr_background();
void gdr_openProject(NSString* project);
BOOL gdr_isRunning();
#ifndef TARGET_OS_TV
void gdr_touchesBegan(NSSet* touches, NSSet* allTouches);
void gdr_touchesMoved(NSSet* touches, NSSet* allTouches);
void gdr_touchesEnded(NSSet* touches, NSSet* allTouches);
void gdr_touchesCancelled(NSSet* touches, NSSet* allTouches);
#endif
#ifdef __cplusplus
}
#endif

#endif
