#ifndef CAMERA_PLUGIN_BINDER
#define CAMERA_PLUGIN_BINDER

#include "grendertarget.h"
#include "application.h"
#include "luaapplication.h"
#include <string>
#include <vector>

namespace cameraplugin {
	extern GRenderTarget *cameraTexture;
	extern LuaApplication* application;
	struct CameraDesc {
		std::string name;
		std::string description;
		enum {
			POS_UNKNOWN,
			POS_FRONTFACING,
			POS_BACKFACING
		} pos;
	};
	void init();
	std::vector<CameraDesc> availableDevices();
	void start(Orientation orientation,int *camwidth,int *camheight,const char *device);
	void stop();
	void deinit();
    bool isAvailable();
}

#endif
