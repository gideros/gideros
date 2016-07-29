#ifndef CAMERA_PLUGIN_BINDER
#define CAMERA_PLUGIN_BINDER

#include "texturebase.h"
#include "application.h"

namespace cameraplugin {
	extern TextureBase *cameraTexture;
	void start(Orientation orientation,int *camwidth,int *camheight);
	void stop();
}

#endif
