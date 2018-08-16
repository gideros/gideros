#include "alertdialogbinder.h"
#include <eventdispatcher.h>
#include <gui.h>
#include "luautil.h"

static char key = ' ';

#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static lua_State *L = NULL;
class GGAlertDialog : public EventDispatcher
{
public:
    GGAlertDialog(lua_State */*L*/, const char *title, const char *message, const char *cancelButton, const char *button1, const char *button2)
    {
        id_ = gui_createAlertDialog(title, message, cancelButton, button1, button2, callback_s, this);
    }

    ~GGAlertDialog()
    {
        gui_delete(id_);
    }

    void show()
    {
        gui_show(id_);
    }

    void hide()
    {
        gui_hide(id_);
    }

    bool isVisible()
    {
        return gui_isVisible(id_) != 0;
    }

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGAlertDialog*>(udata)->callback(type, event);
    }

    void callback(int /*type*/, void *event)
    {
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key); // push AlertDialogs table
        luaL_rawgetptr(L, -1, this);                // push AlertDialog object

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "dispatchEvent");

        lua_pushvalue(L, -2); // create copy of AlertDialog object

        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);				// remove global "Event"

        lua_pushstring(L, "complete");
        lua_call(L, 1, 1); // call Event.new

        gui_AlertDialogCompleteEvent *event2 = (gui_AlertDialogCompleteEvent*)event;
        if (event2->buttonIndex >= 1)
        {
            lua_pushinteger(L, event2->buttonIndex);
            lua_setfield(L, -2, "buttonIndex");
        }
        lua_pushstring(L, event2->buttonText);
        lua_setfield(L, -2, "buttonText");

        lua_call(L, 2, 0); // call alertdialog:dispatchEvent(event)

        lua_pop(L, 1);	// pop AlertDialog object

        lua_pushnil(L);
        luaL_rawsetptr(L, -2, this);

        lua_pop(L, 1);  // pop AlertDialogs table
    }

private:
    g_id id_;
};

AlertDialogBinder::AlertDialogBinder(lua_State* L)
{
    ::L = L;
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"show", show},
        {"hide", hide},
        {NULL, NULL},
    };

    binder.createClass("AlertDialog", "EventDispatcher", create, destruct, functionList);

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key);
}

int AlertDialogBinder::create(lua_State *L)
{
    Binder binder(L);

    const char *title = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);
    const char *cancelButton = luaL_checkstring(L, 3);
    const char *button1 = lua_isnoneornil(L, 4) ? NULL : luaL_checkstring(L, 4);
    const char *button2 = lua_isnoneornil(L, 5) ? NULL : luaL_checkstring(L, 5);

    GGAlertDialog *alertDialog = new GGAlertDialog(L, title, message, cancelButton, button1, button2);

    binder.pushInstance("AlertDialog", alertDialog);

    return 1;
}

int AlertDialogBinder::destruct(lua_State *L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GGAlertDialog *alertDialog = static_cast<GGAlertDialog*>(ptr);
    alertDialog->unref();

    return 0;
}

int AlertDialogBinder::show(lua_State *L)
{
    Binder binder(L);

    GGAlertDialog *alertDialog = static_cast<GGAlertDialog*>(binder.getInstance("AlertDialog", 1));
    alertDialog->show();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key);
    lua_pushvalue(L, 1);
    luaL_rawsetptr(L, -2, alertDialog);
    lua_pop(L, 1);

    return 0;
}

int AlertDialogBinder::hide(lua_State *L)
{
    Binder binder(L);

    GGAlertDialog *alertDialog = static_cast<GGAlertDialog*>(binder.getInstance("AlertDialog", 1));
    alertDialog->hide();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, alertDialog);
    lua_pop(L, 1);

    return 0;
}
