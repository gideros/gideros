#ifndef _TEXTINPUTDIALOGBINDER_H_
#define _TEXTINPUTDIALOGBINDER_H_

#include "binder.h"

class TextInputDialogBinder
{
public:
    TextInputDialogBinder(lua_State* L);

private:
    static int create(lua_State *L);
    static int destruct(lua_State *L);

    static int show(lua_State *L);
    static int hide(lua_State *L);

    static int setText(lua_State *L);
    static int getText(lua_State *L);
    static int setInputType(lua_State *L);
    static int getInputType(lua_State *L);
    static int setSecureInput(lua_State *L);
    static int isSecureInput(lua_State *L);
};

#endif
