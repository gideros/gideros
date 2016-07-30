#ifndef CAMERA_PLUGIN_BINDER
#define CAMERA_PLUGIN_BINDER

#include "texturebase.h"
#include "application.h"

namespace cameraplugin {
	extern TextureBase *cameraTexture;
	void init();
	void start(Orientation orientation,int *camwidth,int *camheight);
	void stop();
	void deinit();
}

#endif
