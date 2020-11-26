#include "ogl.h"
#include "glog.h"
#include <string.h>
#include "gtexture.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef WINSTORE
#include "dx11Shaders.h"
#elif defined(TARGET_OS_MAC)
extern ShaderEngine *createMetalShaderEngine(int sw,int sh);
#else
#include "gl2Shaders.h"
#endif
#include "screen.h"

void oglInitialize(unsigned int sw, unsigned int sh) {
	if (ShaderEngine::Engine)
		return;
#ifdef WINSTORE
	ShaderEngine::Engine = new dx11ShaderEngine(sw, sh);
#elif TARGET_OS_MAC
    ShaderEngine::Engine=createMetalShaderEngine(sw,sh);
#else
	ShaderEngine::Engine = new ogl2ShaderEngine(sw, sh);
#endif
	ShaderEngine::Engine->setVBOThreshold(10,10);
	gtexture_set_engine(ShaderEngine::Engine);
	gtexture_set_screenmanager(ScreenManager::manager);
}

void oglCleanup() {
	if (ShaderEngine::Engine) {
		gtexture_set_screenmanager(NULL);
		gtexture_set_engine(NULL);
		delete ShaderEngine::Engine;
		ShaderEngine::Engine = NULL;
	}
}
