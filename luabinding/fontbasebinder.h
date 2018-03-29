#ifndef FONTBASEBINDER_H
#define FONTBASEBINDER_H

#include "binder.h"

class FontBaseBinder
{
public:
    FontBaseBinder(lua_State *L);

private:
    static int getBounds(lua_State *L);
    static int getAscender(lua_State *L);
    static int getLineHeight(lua_State *L);
    static int getAdvanceX(lua_State *L);
    static int layoutText(lua_State *L);
    static int getCharIndexAtOffset(lua_State *L);
};

#endif
