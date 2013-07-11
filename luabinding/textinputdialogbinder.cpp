#include "textinputdialogbinder.h"
#include <eventdispatcher.h>
#include <gui.h>
#include "luautil.h"
#include <gstatus.h>

static char key = ' ';

#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

#define TEXT "text"
#define NUMBER "number"
#define PHONE "phone"
#define EMAIL "email"
#define URL "url"

class GGTextInputDialog : public EventDispatcher
{
public:
    GGTextInputDialog(lua_State *L,
                      const char *title,
                      const char *message,
                      const char *text,
                      const char *cancelButton,
                      const char *button1,
                      const char *button2)
    {
        this->L = L;
        gid_ = gui_createTextInputDialog(title, message, text, cancelButton, button1, button2, callback_s, this);
    }

    ~GGTextInputDialog()
    {
        gui_delete(gid_);
    }

    void show()
    {
        gui_show(gid_);
    }

    void hide()
    {
        gui_hide(gid_);
    }

    bool isVisible()
    {
        return gui_isVisible(gid_);
    }

    void setText(const char* text)
    {
        return gui_setText(gid_, text);
    }

    const char *getText() const
    {
        return gui_getText(gid_);
    }

    void setInputType(int inputType)
    {
        gui_setInputType(gid_, inputType);
    }

    int getInputType() const
    {
        return gui_getInputType(gid_);
    }

    void setSecureInput(bool secureInput)
    {
        gui_setSecureInput(gid_, secureInput);
    }

    bool isSecureInput() const
    {
        return gui_isSecureInput(gid_);
    }

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGTextInputDialog*>(udata)->callback(type, event);
    }

    void callback(int type, void *event)
    {
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key); // push TextInputDialogs table
        luaL_rawgetptr(L, -1, this);                // push TextInputDialog object

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "dispatchEvent");

        lua_pushvalue(L, -2); // create copy of TextInputDialog object

        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);				// remove global "Event"

        lua_pushstring(L, "complete");
        lua_call(L, 1, 1); // call Event.new

        gui_TextInputDialogCompleteEvent *event2 = (gui_TextInputDialogCompleteEvent*)event;
        lua_pushstring(L, event2->text);
        lua_setfield(L, -2, "text");
        if (event2->buttonIndex >= 1)
        {
            lua_pushinteger(L, event2->buttonIndex);
            lua_setfield(L, -2, "buttonIndex");
        }
        lua_pushstring(L, event2->buttonText);
        lua_setfield(L, -2, "buttonText");

        lua_call(L, 2, 0); // call textinputdialog:dispatchEvent(event)

        lua_pop(L, 1);	// pop TextInputDialog object

        lua_pushnil(L);
        luaL_rawsetptr(L, -2, this);

        lua_pop(L, 1);  // pop TextInputDialogss table
    }

private:
    g_id gid_;
    lua_State *L;
};

TextInputDialogBinder::TextInputDialogBinder(lua_State* L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"show", show},
        {"hide", hide},
        {"setText", setText},
        {"getText", getText},
        {"setInputType", setInputType},
        {"getInputType", getInputType},
        {"setSecureInput", setSecureInput},
        {"isSecureInput", isSecureInput},
        {NULL, NULL},
    };

    binder.createClass("TextInputDialog", "EventDispatcher", create, destruct, functionList);

    lua_getglobal(L, "TextInputDialog");
    lua_pushstring(L, TEXT);
    lua_setfield(L, -2, "TEXT");
    lua_pushstring(L, NUMBER);
    lua_setfield(L, -2, "NUMBER");
    lua_pushstring(L, PHONE);
    lua_setfield(L, -2, "PHONE");
    lua_pushstring(L, EMAIL);
    lua_setfield(L, -2, "EMAIL");
    lua_pushstring(L, URL);
    lua_setfield(L, -2, "URL");
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key);
}

int TextInputDialogBinder::create(lua_State *L)
{
    Binder binder(L);

    const char *title = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);
    const char *text = luaL_checkstring(L, 3);
    const char *cancelButton = luaL_checkstring(L, 4);
    const char *button1 = lua_isnoneornil(L, 5) ? NULL : luaL_checkstring(L, 5);
    const char *button2 = lua_isnoneornil(L, 6) ? NULL : luaL_checkstring(L, 6);

    GGTextInputDialog *textInputDialog = new GGTextInputDialog(L, title, message, text, cancelButton, button1, button2);

    binder.pushInstance("TextInputDialog", textInputDialog);

    return 1;
}

int TextInputDialogBinder::destruct(lua_State *L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(ptr);
    textInputDialog->unref();

    return 0;
}

int TextInputDialogBinder::show(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    textInputDialog->show();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key);
    lua_pushvalue(L, 1);
    luaL_rawsetptr(L, -2, textInputDialog);
    lua_pop(L, 1);

    return 0;
}

int TextInputDialogBinder::hide(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    textInputDialog->hide();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, textInputDialog);
    lua_pop(L, 1);

    return 0;
}


int TextInputDialogBinder::setText(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    textInputDialog->setText(luaL_checkstring(L, 2));

    return 0;
}

int TextInputDialogBinder::getText(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    lua_pushstring(L, textInputDialog->getText());

    return 1;
}

int TextInputDialogBinder::setInputType(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    const char *inputTypeStr = luaL_checkstring(L, 2);

    int inputType;
    if (strcmp(inputTypeStr, TEXT) == 0)
    {
        inputType = GUI_TEXT_INPUT_DIALOG_TEXT;
    }
    else if (strcmp(inputTypeStr, NUMBER) == 0)
    {
        inputType = GUI_TEXT_INPUT_DIALOG_NUMBER;
    }
    else if (strcmp(inputTypeStr, PHONE) == 0)
    {
        inputType = GUI_TEXT_INPUT_DIALOG_PHONE;
    }
    else if (strcmp(inputTypeStr, EMAIL) == 0)
    {
        inputType = GUI_TEXT_INPUT_DIALOG_EMAIL;
    }
    else if (strcmp(inputTypeStr, URL) == 0)
    {
        inputType = GUI_TEXT_INPUT_DIALOG_URL;
    }
    else
    {
        GStatus status(2008, "inputType"); // Error #2008: Parameter '%s' must be one of the accepted values.
        return luaL_error(L, status.errorString());
    }

    textInputDialog->setInputType(inputType);

    return 0;
}

int TextInputDialogBinder::getInputType(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));

    switch (textInputDialog->getInputType())
    {
    case GUI_TEXT_INPUT_DIALOG_TEXT:
        lua_pushstring(L, TEXT);
        break;
    case GUI_TEXT_INPUT_DIALOG_NUMBER:
        lua_pushstring(L, NUMBER);
        break;
    case GUI_TEXT_INPUT_DIALOG_PHONE:
        lua_pushstring(L, PHONE);
        break;
    case GUI_TEXT_INPUT_DIALOG_EMAIL:
        lua_pushstring(L, EMAIL);
        break;
    case GUI_TEXT_INPUT_DIALOG_URL:
        lua_pushstring(L, URL);
        break;
    }

    return 1;
}

int TextInputDialogBinder::setSecureInput(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    textInputDialog->setSecureInput(lua_toboolean(L, 2));

    return 0;
}

int TextInputDialogBinder::isSecureInput(lua_State *L)
{
    Binder binder(L);

    GGTextInputDialog *textInputDialog = static_cast<GGTextInputDialog*>(binder.getInstance("TextInputDialog", 1));
    lua_pushboolean(L, textInputDialog->isSecureInput());

    return 1;
}
