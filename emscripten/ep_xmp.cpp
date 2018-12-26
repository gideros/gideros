#include <gaudio.h>
#include <ggaudiomanager.h>
#include "lua.h"
#include "gplugin.h"

static GGAudioLoader audio_loader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);

static void g_initializePlugin(lua_State *L) {

	gaudio_registerType("mod", audio_loader);
	gaudio_registerType("xm", audio_loader);
	gaudio_registerType("it", audio_loader);
	gaudio_registerType("s3m", audio_loader);
}

static void g_deinitializePlugin(lua_State *L) {
	gaudio_unregisterType("mod");
	gaudio_unregisterType("xm");
	gaudio_unregisterType("it");
	gaudio_unregisterType("s3m");
}
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER)
REGISTER_PLUGIN_STATICNAMED_CPP("EP_Xmp", "1.0",EP_Xmp);
#else
REGISTER_PLUGIN_NAMED("EP_Xmp", "1.0", EP_Xmp);
#endif
