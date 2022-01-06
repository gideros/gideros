#ifndef _ALERTBOXBINDER_H_
#define _ALERTBOXBINDER_H_

#include "binder.h"

class AlertDialogBinder
{
public:
    AlertDialogBinder(lua_State* L);

private:
    static int create(lua_State *L);
    static int destruct(void *p);

    static int show(lua_State *L);
    static int hide(lua_State *L);
};

#endif
