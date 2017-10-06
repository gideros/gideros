#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H

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
#if !TARGET_OS_TV && !TARGET_OS_OSX
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
#if TARGET_OS_TV == 0
    void gdr_touchesBegan(NSSet* touches, NSSet* allTouches);
    void gdr_touchesMoved(NSSet* touches, NSSet* allTouches);
    void gdr_touchesEnded(NSSet* touches, NSSet* allTouches);
    void gdr_touchesCancelled(NSSet* touches, NSSet* allTouches);
#endif
#if TARGET_OS_OSX
    void gdr_mouseDown(int x, int y, int button,int mod);
    void gdr_mouseMove(int x, int y, int button,int mod);
    void gdr_mouseHover(int x, int y, int button,int mod);
    void gdr_mouseUp(int x, int y, int button,int mod);
    void gdr_mouseWheel(int x, int y, int button, int delta,int mod);
#endif
void gdr_keyDown(int keyCode, int repeat);
void gdr_keyUp(int keyCode, int repeat);
void gdr_keyChar(NSString *text);
BOOL gdr_keyboardVisible();
#ifdef __cplusplus
}
#endif

#endif
