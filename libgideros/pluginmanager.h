#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include <lua.hpp>
#include "gexport.h"
#include "gplugin.h"

struct GIDEROS_API GGPlugin
{
    GGPlugin()
    {
        main2 = NULL;
        openUrl = NULL;
        enterFrame = NULL;
        suspend = NULL;
        resume = NULL;
        background = NULL;
        foreground = NULL;
    }

    void *main(lua_State *L, int type);

    void*(*main2)(lua_State*, int);
    void(*openUrl)(lua_State*, const char *);
    void(*enterFrame)(lua_State*);
    void(*suspend)(lua_State*);
    void(*resume)(lua_State*);
    void(*background)(lua_State*);
    void(*foreground)(lua_State*);
};

class GIDEROS_API PluginManager
{
public:
	static PluginManager& instance();
    int registerPlugin(void*(*main)(lua_State*, int));

    std::vector<GGPlugin> plugins;
};

#endif
