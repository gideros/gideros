#include <gaudio.h>
#include <ggaudiomanager.h>
#include <mpg123.h>
#include "lua.h"
#include "gplugin.h"

static GGAudioLoader audio_loader(gaudio_Mp3Open, gaudio_Mp3Close, gaudio_Mp3Read, gaudio_Mp3Seek, gaudio_Mp3Tell);

static void g_initializePlugin(lua_State *L) {
    mpg123_init();

	gaudio_registerType("mp3", audio_loader);
}

static void g_deinitializePlugin(lua_State *L) {
	gaudio_unregisterType("mp3");
    mpg123_exit();
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("EP_Mp3", "1.0",EP_Mp3)
#else
REGISTER_PLUGIN_NAMED("EP_Mp3", "1.0", EP_Mp3)
#endif
