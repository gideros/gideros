#ifndef GIDEROSAPI_H
#define GIDEROSAPI_H

#ifdef __cplusplus
extern "C" {
#endif

	void gdr_initialize(int width, int height, bool player);
	void gdr_drawFirstFrame();
	void gdr_drawFrame();
	void gdr_exitGameLoop();
	void gdr_deinitialize();
	void gdr_suspend();
	void gdr_resume();
	void gdr_didReceiveMemoryWarning();
	void gdr_foreground();
	void gdr_background();
	void gdr_openProject(const char* project);
	bool gdr_isRunning();

#ifdef __cplusplus
}
#endif

#endif

