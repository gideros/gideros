#include "pluginmanager.h"

static GGPlugin *s_active = NULL;

void *GGPlugin::main(lua_State *L, int type)
{
    void *result;

    if (type == 0)
    {
        s_active = this;
        result = main2(L, type);
        s_active = NULL;
    }
    else if (type == 1)
    {
        result = main2(L, type);

        openUrl = NULL;
        enterFrame = NULL;
        suspend = NULL;
        resume = NULL;
        background = NULL;
        foreground = NULL;
    }
    else
    {
        result = main2(L, type);
    }

    return result;
}

PluginManager& PluginManager::instance()
{
	static PluginManager instance;
	return instance;
}

int PluginManager::registerPlugin(void*(*main)(lua_State*, int))
{
    GGPlugin plugin;
    plugin.main2 = main;
    plugins.push_back(plugin);
    return 0;
}

extern "C"
{
int g_registerPlugin(void*(*main)(lua_State*, int))
{
    return PluginManager::instance().registerPlugin(main);
}
}

extern "C"
{
void g_registerOpenUrlCallback(void(*openUrl)(lua_State*, const char *))
{
    s_active->openUrl = openUrl;
}

void g_registerEnterFrameCallback(void(*enterFrame)(lua_State*))
{
    s_active->enterFrame = enterFrame;
}

void g_registerSuspendCallback(void(*suspend)(lua_State*))
{
    s_active->suspend = suspend;
}

void g_registerResumeCallback(void(*resume)(lua_State*))
{
    s_active->resume = resume;
}

void g_registerForegroundCallback(void(*foreground)(lua_State*))
{
    s_active->foreground = foreground;
}

void g_registerBackgroundCallback(void(*background)(lua_State*))
{
    s_active->background = background;
}

}
