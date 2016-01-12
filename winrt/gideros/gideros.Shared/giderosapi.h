#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H

using namespace Windows::UI::Core;

#ifdef __cplusplus
extern "C" {
#endif

	void gdr_initialize(bool useXaml, CoreWindow^ Window, Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel,
						int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath, const wchar_t* tempPath);
	void gdr_drawFirstFrame();
	void gdr_drawFrame(bool useXaml);
	void gdr_exitGameLoop();
	void gdr_deinitialize();
	void gdr_suspend();
	void gdr_resume();
	void gdr_didReceiveMemoryWarning();
	void gdr_foreground();
	void gdr_background();
	void gdr_openProject(const char* project);
	bool gdr_isRunning();
	void gdr_keyDown(int keyCode);
	void gdr_keyUp(int keyCode);
	void gdr_keyChar(const char *keyChar);
	void gdr_mouseDown(int x, int y, int button);
	void gdr_mouseMove(int x, int y);
	void gdr_mouseHover(int x, int y);
	void gdr_mouseUp(int x, int y, int button);
	void gdr_mouseWheel(int x, int y, int delta);
	void gdr_touchBegin(int x, int y, int id);
	void gdr_touchMove(int x, int y, int id);
	void gdr_touchEnd(int x, int y, int id);
	void gdr_touchCancel(int x, int y, int id);
	void gdr_resize(int width, int height);
	Windows::UI::Xaml::Controls::SwapChainPanel^ gdr_getRootView();
#ifdef __cplusplus
}
#endif

#endif

