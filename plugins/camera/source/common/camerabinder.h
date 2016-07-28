#ifndef CAMERA_PLUGIN_BINDER
#define CAMERA_PLUGIN_BINDER

#include "texturebase.h"

namespace cameraplugin {
	extern TextureBase *cameraTexture;
	void start();
	void stop();
}

#endif
