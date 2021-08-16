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
	struct CameraInfo
	{
		CameraInfo() : angle(0) {}
		std::vector<int> previewSizes;
		std::vector<int> pictureSizes;
		int angle;
		std::vector<int> flashModes;
	};
	void init();
	std::vector<CameraDesc> availableDevices();
	void start(Orientation orientation,int *camwidth,int *camheight,const char *device,int *picWidth,int *picHeight);
	void stop();
	void deinit();
    bool isAvailable();
    bool setFlash(int mode);
    bool takePicture();
    CameraInfo queyCamera(const char *device, Orientation orientation);
    void callback_s(int type, void *event, void *udata);
}

#endif
