// regex: (\s\*)+\b

#define _UNUSED(n)
//#define IS_BETA_BUILD

#ifdef IS_BETA_BUILD
#define PLUGIN_NAME "ImGui_beta"
#else
#define PLUGIN_NAME "ImGui"
#endif

#include "lua.hpp"
#include "luautil.h"

#include "gplugin.h"
#include "gfile.h"
#include "gstdio.h"
#include "ginput.h"
#include "binder.h"
#include "application.h"
#include "luaapplication.h"

#include "mouseevent.h"
#include "keyboardevent.h"
#include "touchevent.h"

#include "sprite.h"
#include "texturebase.h"
#include "bitmapdata.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui_src/imgui.h"
#include "imgui_src/imgui_internal.h"

#ifdef IS_BETA_BUILD
#include "imgui-node-editor/imgui_node_editor.h" // https://github.com/thedmd/imgui-node-editor
#define ED ax::NodeEditor
#endif

static lua_State* L;
static Application* application;
static SpriteProxy* imguiProxy;

static char keyWeak = ' ';
static bool autoUpdateCursor = false;
static bool instanceCreated = false;
static bool resetTouchPosOnEnd = false;
static std::map<int, const char*> giderosCursorMap;

static void resetStaticVars()
{
    instanceCreated = false;
    resetTouchPosOnEnd = false;
    autoUpdateCursor = false;
}

#define LUA_ASSERT(EXP, MSG) if (!(EXP)) { lua_pushstring(L, MSG); lua_error(L); }
#define LUA_ASSERTF(EXP, FMT, ...) if (!(EXP)) { lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L); }
#define LUA_THROW_ERROR(MSG) lua_pushstring(L, MSG); lua_error(L);
#define LUA_THROW_ERRORF(FMT, ...) lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L);
#define LUA_PRINTF(FMT, ...) lua_getglobal(L, "print"); lua_pushfstring(L, FMT, __VA_ARGS__); lua_call(L, 1, 0);
#define LUA_PRINT(MSG) lua_getglobal(L, "print"); lua_pushstring(L, MSG); lua_call(L, 1, 0);

namespace ImGui_impl
{
////////////////////////////////////////////////////////////////////////////////
///
/// DEBUG TOOL
///
////////////////////////////////////////////////////////////////////////////////

int DUMP_INDEX = 0;

void stackDump(lua_State* L, const char* prefix = "")
{
    int i = lua_gettop(L);
    lua_getglobal(L, "print");
    lua_pushfstring(L, "----------------      %d      ----------------\n>%s\n----------------  Stack Dump ----------------", DUMP_INDEX, prefix);
    lua_call(L, 1, 0);
    while (i)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "%d:'%s'", i, lua_tostring(L, i));
            lua_call(L, 1, 0);
        }
            break;
        case LUA_TBOOLEAN:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "%d: %s", i, lua_toboolean(L, i) ? "true" : "false");
            lua_call(L, 1, 0);
        }
            break;
        case LUA_TNUMBER:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "%d: %d", i, lua_tonumber(L, i));
            lua_call(L, 1, 0);
        }
            break;
        default:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "%d: %s", i, lua_typename(L, t));
            lua_call(L, 1, 0);
        }
            break;
        }
        i--;
    }
    lua_getglobal(L, "print");
    lua_pushstring(L, "------------ Stack Dump Finished ------------\n");
    lua_call(L, 1, 0);

    DUMP_INDEX++;
}

////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
////////////////////////////////////////////////////////////////////////////////

static void localToGlobal(SpriteProxy* proxy, float x, float y, float* tx, float* ty)
{
    const Sprite* curr = proxy;

    float z;
    while (curr) {
        curr->matrix().transformPoint(x, y, 0, &x, &y, &z);
        curr = curr->parent();
    }

    if (tx)
        *tx = x;

    if (ty)
        *ty = y;
}

static int convertGiderosMouseButton(const int button)
{
    LUA_ASSERTF(button >= 0, "Button index must be >= 0, but was: %d", button);
    switch (button)
    {
        case GINPUT_NO_BUTTON:
            return 4; // unused by ImGui itself
        case GINPUT_LEFT_BUTTON:
            return 0;
        case GINPUT_RIGHT_BUTTON:
            return 1;
        case GINPUT_MIDDLE_BUTTON:
            return 2;
        case 8:
            return 3;
        case 16:
            return 4;
        default:
            LUA_THROW_ERRORF("Incorrect button index. Expected 0, 1, 2, 4, 8 or 16, but got: %d", button);
            break;
    }
}

struct GTextureData
{
    void* texture;
    ImVec2 texture_size;
    ImVec2 uv0;
    ImVec2 uv1;
};

struct VColor {
    uint8_t r,g,b,a;
};

struct GColor {
    int hex; // 0xffffff
    double alpha; // [0..1]

    GColor()
    {
        hex = 0;
        alpha = 0;
    }

    GColor(int _hex, double _alpha = 1.0f)
    {
        hex = _hex;
        alpha = _alpha;
    }

    static ImVec4 toVec4(int hex, double alpha = 1.0f)
    {
        double s = 1.0f / 255.0f;
        return ImVec4(
                    ((hex >> IM_COL32_B_SHIFT) & 0xFF) * s,
                    ((hex >> IM_COL32_G_SHIFT) & 0xFF) * s,
                    ((hex >> IM_COL32_R_SHIFT) & 0xFF) * s,
                    alpha);
    }

    static ImVec4 toVec4(GColor color)
    {
        return GColor::toVec4(color.hex, color.alpha);
    }

    static GColor toHex(double _r, double _g, double _b, double _a = 1.0f)
    {
        int r = _r * 255;
        int g = _g * 255;
        int b = _b * 255;

        int hex = (r << IM_COL32_B_SHIFT) + (g << IM_COL32_G_SHIFT) + (b << IM_COL32_R_SHIFT);

        return GColor(hex, _a);
    }

    static GColor toHex(ImVec4 color)
    {
        return GColor::toHex(color.x, color.y, color.z, color.w);
    }

    static ImU32 toU32(double _r, double _g, double _b, double _a = 1.0f)
    {
        ImU32 r = _r * 255;
        ImU32 g = _g * 255;
        ImU32 b = _b * 255;
        ImU32 a = _a * 255;
        return ((a << IM_COL32_A_SHIFT) | (b << IM_COL32_B_SHIFT) | (g << IM_COL32_G_SHIFT) | (r << IM_COL32_R_SHIFT));
    }

    static ImU32 toU32(ImVec4 color)
    {
        return GColor::toU32(color.x, color.y, color.y, color.w);
    }

    static ImU32 toU32(int hex, double alpha = 1.0f)
    {
        alpha *= 255.0f;
        ImU32 ghex = (int)alpha | hex << 8;

        ImU32 out =
                (((ghex << IM_COL32_R_SHIFT) & 0xff000000) >> IM_COL32_A_SHIFT) |
                (((ghex << IM_COL32_G_SHIFT) & 0xff000000) >> IM_COL32_B_SHIFT) |
                (((ghex << IM_COL32_B_SHIFT) & 0xff000000) >> IM_COL32_G_SHIFT) |
                (((ghex << IM_COL32_A_SHIFT) & 0xff000000) >> IM_COL32_R_SHIFT);
        return out;
    }

    static ImU32 toU32(GColor color)
    {
        return GColor::toU32(color.hex, color.alpha);
    }


};

static int getKeyboardModifiers(lua_State *L)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, "getKeyboardModifiers");
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    int mod = lua_tointeger(L, -1);
    lua_remove(L, -1);
    lua_remove(L, -1);
    return mod;
}

static lua_Number getAppProperty(lua_State *L, const char* name)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, name);
    lua_pushvalue(L, -2);
    lua_call(L, 1, 1);
    lua_Number value = lua_tonumber(L, -1);
    lua_remove(L, -1);
    return value;
}

static void setApplicationCursor(lua_State* L, const char* name)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, "set");
    lua_pushvalue(L, -2);
    lua_pushstring(L, "cursor");
    lua_pushstring(L, name);
    lua_call(L, 3, 0);
    lua_pop(L, 2);
}

GTextureData getTexture(lua_State* L, int idx = 1)
{
    Binder binder(L);

    if (binder.isInstanceOf("TextureBase", idx))
    {
        GTextureData data;
        TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", idx));

        TextureData* gdata = textureBase->data;

        data.texture_size.x = (float)gdata->width;
        data.texture_size.y = (float)gdata->height;
        data.texture = (void*)gdata->gid;
        data.uv0.x = 0.0f;
        data.uv0.y = 0.0f;
        data.uv1.x = data.texture_size.x / (float)gdata->exwidth;
        data.uv1.y = data.texture_size.y / (float)gdata->exheight;
        return data;
    }
    else if (binder.isInstanceOf("TextureRegion", idx))
    {
        GTextureData data;
        BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", idx));

        TextureData* gdata = bitmapData->texture()->data;

        int x, y, w, h;
        bitmapData->getRegion(&x, &y, &w, &h, 0, 0, 0, 0);
        data.texture_size.x = (float)w;
        data.texture_size.y = (float)h;

        data.uv0.x = (float)x / (float)gdata->exwidth;
        data.uv0.y = (float)y / (float)gdata->exheight;
        data.uv1.x = (float)(x + w) / (float)gdata->exwidth;
        data.uv1.y = (float)(y + h) / (float)gdata->exheight;
        data.texture = (void*)gdata->gid;

        return data;
    }
    else
    {
        luaL_typerror(L, idx, "TextureBase or TextureRegion");
    }
}

static int luaL_optboolean(lua_State* L, int narg, int def)
{
    return lua_isboolean(L, narg) ? lua_toboolean(L, narg) : def;
}

static lua_Number getfield(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    lua_Number result = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return result;
}

static lua_Number getsubfield(lua_State* L, const char* field, const char* key)
{
    lua_pushstring(L, field);
    lua_gettable(L, -2);
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    lua_Number result = lua_tonumber(L, -1);
    lua_pop(L, 2);
    return result;
}

static void lua_setintfield(lua_State* L, int idx, int index)
{
    lua_pushinteger(L, index);
    lua_insert(L, -2);
    lua_settable(L,idx-(idx<0));
}

ImGuiID checkID(lua_State* L, int idx = 2)
{
    double id = luaL_checknumber(L, idx);
    LUA_ASSERT(id > 0, "ID must be > 0!");
    return (ImGuiID)id;
}

static bool* getPopen(lua_State* L, int idx, int top = 2)
{
    if (lua_gettop(L) > top && lua_type(L, idx) != LUA_TNIL)
    {
        bool flag = lua_toboolean(L, idx);
        return new bool(flag);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void bindEnums(lua_State* L)
{
#ifdef IS_BETA_BUILD
    lua_getglobal(L, "ImGuiNodeEditor");

    lua_pushinteger(L, (int)ED::PinKind::Input);                        lua_setfield(L, -2, "Input");
    lua_pushinteger(L, (int)ED::PinKind::Output);                       lua_setfield(L, -2, "Output");

    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_Bg);             lua_setfield(L, -2, "StyleColor_Bg");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_Grid);           lua_setfield(L, -2, "StyleColor_Grid");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_NodeBg);         lua_setfield(L, -2, "StyleColor_NodeBg");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_NodeBorder);     lua_setfield(L, -2, "StyleColor_NodeBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_HovNodeBorder);  lua_setfield(L, -2, "StyleColor_HovNodeBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_SelNodeBorder);  lua_setfield(L, -2, "StyleColor_SelNodeBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_NodeSelRect);    lua_setfield(L, -2, "StyleColor_NodeSelRect");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_NodeSelRectBorder);lua_setfield(L, -2, "StyleColor_NodeSelRectBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_HovLinkBorder);  lua_setfield(L, -2, "StyleColor_HovLinkBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_SelLinkBorder);  lua_setfield(L, -2, "StyleColor_SelLinkBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_LinkSelRect);    lua_setfield(L, -2, "StyleColor_LinkSelRect");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_LinkSelRectBorder);lua_setfield(L, -2, "StyleColor_LinkSelRectBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_PinRect);        lua_setfield(L, -2, "StyleColor_PinRect");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_PinRectBorder);  lua_setfield(L, -2, "StyleColor_PinRectBorder");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_Flow);           lua_setfield(L, -2, "StyleColor_Flow");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_FlowMarker);     lua_setfield(L, -2, "StyleColor_FlowMarker");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_GroupBg);        lua_setfield(L, -2, "StyleColor_GroupBg");
    lua_pushinteger(L, (int)ED::StyleColor::StyleColor_GroupBorder);    lua_setfield(L, -2, "StyleColor_GroupBorder");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_NodePadding);        lua_setfield(L, -2, "StyleVar_NodePadding");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_NodeRounding);       lua_setfield(L, -2, "StyleVar_NodeRounding");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_NodeBorderWidth);    lua_setfield(L, -2, "StyleVar_NodeBorderWidth");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_HoveredNodeBorderWidth);lua_setfield(L, -2, "StyleVar_HoveredNodeBorderWidth");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_SelectedNodeBorderWidth);lua_setfield(L, -2, "StyleVar_SelectedNodeBorderWidth");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinRounding);        lua_setfield(L, -2, "StyleVar_PinRounding");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinBorderWidth);     lua_setfield(L, -2, "StyleVar_PinBorderWidth");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_LinkStrength);       lua_setfield(L, -2, "StyleVar_LinkStrength");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_SourceDirection);    lua_setfield(L, -2, "StyleVar_SourceDirection");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_TargetDirection);    lua_setfield(L, -2, "StyleVar_TargetDirection");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_ScrollDuration);     lua_setfield(L, -2, "StyleVar_ScrollDuration");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_FlowMarkerDistance); lua_setfield(L, -2, "StyleVar_FlowMarkerDistance");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_FlowSpeed);          lua_setfield(L, -2, "StyleVar_FlowSpeed");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_FlowDuration);       lua_setfield(L, -2, "StyleVar_FlowDuration");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PivotAlignment);     lua_setfield(L, -2, "StyleVar_PivotAlignment");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PivotSize);          lua_setfield(L, -2, "StyleVar_PivotSize");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PivotScale);         lua_setfield(L, -2, "StyleVar_PivotScale");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinCorners);         lua_setfield(L, -2, "StyleVar_PinCorners");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinRadius);          lua_setfield(L, -2, "StyleVar_PinRadius");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinArrowSize);       lua_setfield(L, -2, "StyleVar_PinArrowSize");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_PinArrowWidth);      lua_setfield(L, -2, "StyleVar_PinArrowWidth");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_GroupRounding);      lua_setfield(L, -2, "StyleVar_GroupRounding");
    lua_pushinteger(L, (int)ED::StyleVar::StyleVar_GroupBorderWidth);   lua_setfield(L, -2, "StyleVar_GroupBorderWidth");

    lua_pop(L, 1);
#endif

    lua_getglobal(L, "ImGui");
    // BackendFlags
    lua_pushinteger(L, ImGuiBackendFlags_None);                         lua_setfield(L, -2, "BackendFlags_None");
    lua_pushinteger(L, ImGuiBackendFlags_HasGamepad);                   lua_setfield(L, -2, "BackendFlags_HasGamepad");
    lua_pushinteger(L, ImGuiBackendFlags_HasMouseCursors);              lua_setfield(L, -2, "BackendFlags_HasMouseCursors");
    lua_pushinteger(L, ImGuiBackendFlags_HasSetMousePos);               lua_setfield(L, -2, "BackendFlags_HasSetMousePos");
    lua_pushinteger(L, ImGuiBackendFlags_RendererHasVtxOffset);         lua_setfield(L, -2, "BackendFlags_RendererHasVtxOffset");

    // ImGuiFocusedFlags
    lua_pushinteger(L, ImGuiFocusedFlags_ChildWindows);                 lua_setfield(L, -2, "FocusedFlags_ChildWindows");
    lua_pushinteger(L, ImGuiFocusedFlags_AnyWindow);                    lua_setfield(L, -2, "FocusedFlags_AnyWindow");
    lua_pushinteger(L, ImGuiFocusedFlags_RootWindow);                   lua_setfield(L, -2, "FocusedFlags_RootWindow");
    lua_pushinteger(L, ImGuiFocusedFlags_RootAndChildWindows);          lua_setfield(L, -2, "FocusedFlags_RootAndChildWindows");
    lua_pushinteger(L, ImGuiFocusedFlags_None);                         lua_setfield(L, -2, "FocusedFlags_None");

    // ImGuiPopupFlags
    lua_pushinteger(L, ImGuiPopupFlags_NoOpenOverExistingPopup);        lua_setfield(L, -2, "PopupFlags_NoOpenOverExistingPopup");
    lua_pushinteger(L, ImGuiPopupFlags_MouseButtonLeft);                lua_setfield(L, -2, "PopupFlags_MouseButtonLeft");
    lua_pushinteger(L, ImGuiPopupFlags_MouseButtonMask_);               lua_setfield(L, -2, "PopupFlags_MouseButtonMask");
    lua_pushinteger(L, ImGuiPopupFlags_MouseButtonRight);               lua_setfield(L, -2, "PopupFlags_MouseButtonRight");
    lua_pushinteger(L, ImGuiPopupFlags_AnyPopupId);                     lua_setfield(L, -2, "PopupFlags_AnyPopupId");
    lua_pushinteger(L, ImGuiPopupFlags_MouseButtonDefault_);            lua_setfield(L, -2, "PopupFlags_MouseButtonDefault");
    lua_pushinteger(L, ImGuiPopupFlags_MouseButtonMiddle);              lua_setfield(L, -2, "PopupFlags_MouseButtonMiddle");
    lua_pushinteger(L, ImGuiPopupFlags_None);                           lua_setfield(L, -2, "PopupFlags_None");
    lua_pushinteger(L, ImGuiPopupFlags_AnyPopup);                       lua_setfield(L, -2, "PopupFlags_AnyPopup");
    lua_pushinteger(L, ImGuiPopupFlags_AnyPopupLevel);                  lua_setfield(L, -2, "PopupFlags_AnyPopupLevel");
    lua_pushinteger(L, ImGuiPopupFlags_NoOpenOverItems);                lua_setfield(L, -2, "PopupFlags_NoOpenOverItems");

    // ImGuiHoveredFlags
    lua_pushinteger(L, ImGuiHoveredFlags_None);                         lua_setfield(L, -2, "HoveredFlags_None");
    lua_pushinteger(L, ImGuiHoveredFlags_RootAndChildWindows);          lua_setfield(L, -2, "HoveredFlags_RootAndChildWindows");
    lua_pushinteger(L, ImGuiHoveredFlags_AllowWhenBlockedByPopup);      lua_setfield(L, -2, "HoveredFlags_AllowWhenBlockedByPopup");
    lua_pushinteger(L, ImGuiHoveredFlags_AllowWhenBlockedByActiveItem); lua_setfield(L, -2, "HoveredFlags_AllowWhenBlockedByActiveItem");
    lua_pushinteger(L, ImGuiHoveredFlags_ChildWindows);                 lua_setfield(L, -2, "HoveredFlags_ChildWindows");
    lua_pushinteger(L, ImGuiHoveredFlags_RectOnly);                     lua_setfield(L, -2, "HoveredFlags_RectOnly");
    lua_pushinteger(L, ImGuiHoveredFlags_AllowWhenDisabled);            lua_setfield(L, -2, "HoveredFlags_AllowWhenDisabled");
    lua_pushinteger(L, ImGuiHoveredFlags_AllowWhenOverlapped);          lua_setfield(L, -2, "HoveredFlags_AllowWhenOverlapped");
    lua_pushinteger(L, ImGuiHoveredFlags_AnyWindow);                    lua_setfield(L, -2, "HoveredFlags_AnyWindow");
    lua_pushinteger(L, ImGuiHoveredFlags_RootWindow);                   lua_setfield(L, -2, "HoveredFlags_RootWindow");

    // ImGuiInputTextFlags
    lua_pushinteger(L, ImGuiInputTextFlags_EnterReturnsTrue);           lua_setfield(L, -2, "InputTextFlags_EnterReturnsTrue");
    lua_pushinteger(L, ImGuiInputTextFlags_CallbackCompletion);         lua_setfield(L, -2, "InputTextFlags_CallbackCompletion");
    lua_pushinteger(L, ImGuiInputTextFlags_None);                       lua_setfield(L, -2, "InputTextFlags_None");
    lua_pushinteger(L, ImGuiInputTextFlags_CallbackResize);             lua_setfield(L, -2, "InputTextFlags_CallbackResize");
    lua_pushinteger(L, ImGuiInputTextFlags_ReadOnly);                   lua_setfield(L, -2, "InputTextFlags_ReadOnly");
    lua_pushinteger(L, ImGuiInputTextFlags_AutoSelectAll);              lua_setfield(L, -2, "InputTextFlags_AutoSelectAll");
    lua_pushinteger(L, ImGuiInputTextFlags_AllowTabInput);              lua_setfield(L, -2, "InputTextFlags_AllowTabInput");
    lua_pushinteger(L, ImGuiInputTextFlags_CharsScientific);            lua_setfield(L, -2, "InputTextFlags_CharsScientific");
    lua_pushinteger(L, ImGuiInputTextFlags_CallbackAlways);             lua_setfield(L, -2, "InputTextFlags_CallbackAlways");
    lua_pushinteger(L, ImGuiInputTextFlags_CharsDecimal);               lua_setfield(L, -2, "InputTextFlags_CharsDecimal");
    lua_pushinteger(L, ImGuiInputTextFlags_NoUndoRedo);                 lua_setfield(L, -2, "InputTextFlags_NoUndoRedo");
    lua_pushinteger(L, ImGuiInputTextFlags_CallbackHistory);            lua_setfield(L, -2, "InputTextFlags_CallbackHistory");
    lua_pushinteger(L, ImGuiInputTextFlags_CtrlEnterForNewLine);        lua_setfield(L, -2, "InputTextFlags_CtrlEnterForNewLine");
    lua_pushinteger(L, ImGuiInputTextFlags_CharsHexadecimal);           lua_setfield(L, -2, "InputTextFlags_CharsHexadecimal");
    lua_pushinteger(L, ImGuiInputTextFlags_CharsNoBlank);               lua_setfield(L, -2, "InputTextFlags_CharsNoBlank");
    lua_pushinteger(L, ImGuiInputTextFlags_Password);                   lua_setfield(L, -2, "InputTextFlags_Password");
    lua_pushinteger(L, ImGuiInputTextFlags_CallbackCharFilter);         lua_setfield(L, -2, "InputTextFlags_CallbackCharFilter");
    lua_pushinteger(L, ImGuiInputTextFlags_NoHorizontalScroll);         lua_setfield(L, -2, "InputTextFlags_NoHorizontalScroll");
    lua_pushinteger(L, ImGuiInputTextFlags_AlwaysInsertMode);           lua_setfield(L, -2, "InputTextFlags_AlwaysInsertMode");
    lua_pushinteger(L, ImGuiInputTextFlags_CharsUppercase);             lua_setfield(L, -2, "InputTextFlags_CharsUppercase");
    // Custom enum
    lua_pushinteger(L, ImGuiInputTextFlags_NoBackground);               lua_setfield(L, -2, "InputTextFlags_NoBackground");

    // ImGuiTabBarFlags
    lua_pushinteger(L, ImGuiTabBarFlags_AutoSelectNewTabs);             lua_setfield(L, -2, "TabBarFlags_AutoSelectNewTabs");
    lua_pushinteger(L, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);  lua_setfield(L, -2, "TabBarFlags_NoCloseWithMiddleMouseButton");
    lua_pushinteger(L, ImGuiTabBarFlags_TabListPopupButton);            lua_setfield(L, -2, "TabBarFlags_TabListPopupButton");
    lua_pushinteger(L, ImGuiTabBarFlags_NoTooltip);                     lua_setfield(L, -2, "TabBarFlags_NoTooltip");
    lua_pushinteger(L, ImGuiTabBarFlags_FittingPolicyMask_);            lua_setfield(L, -2, "TabBarFlags_FittingPolicyMask");
    lua_pushinteger(L, ImGuiTabBarFlags_Reorderable);                   lua_setfield(L, -2, "TabBarFlags_Reorderable");
    lua_pushinteger(L, ImGuiTabBarFlags_FittingPolicyDefault_);         lua_setfield(L, -2, "TabBarFlags_FittingPolicyDefault");
    lua_pushinteger(L, ImGuiTabBarFlags_FittingPolicyScroll);           lua_setfield(L, -2, "TabBarFlags_FittingPolicyScroll");
    lua_pushinteger(L, ImGuiTabBarFlags_FittingPolicyResizeDown);       lua_setfield(L, -2, "TabBarFlags_FittingPolicyResizeDown");
    lua_pushinteger(L, ImGuiTabBarFlags_None);                          lua_setfield(L, -2, "TabBarFlags_None");
    lua_pushinteger(L, ImGuiTabBarFlags_NoTabListScrollingButtons);     lua_setfield(L, -2, "TabBarFlags_NoTabListScrollingButtons");

    // ImGuiTreeNodeFlags
    lua_pushinteger(L, ImGuiTreeNodeFlags_Bullet);                      lua_setfield(L, -2, "TreeNodeFlags_Bullet");
    lua_pushinteger(L, ImGuiTreeNodeFlags_None);                        lua_setfield(L, -2, "TreeNodeFlags_None");
    lua_pushinteger(L, ImGuiTreeNodeFlags_CollapsingHeader);            lua_setfield(L, -2, "TreeNodeFlags_CollapsingHeader");
    lua_pushinteger(L, ImGuiTreeNodeFlags_NavLeftJumpsBackHere);        lua_setfield(L, -2, "TreeNodeFlags_NavLeftJumpsBackHere");
    lua_pushinteger(L, ImGuiTreeNodeFlags_Framed);                      lua_setfield(L, -2, "TreeNodeFlags_Framed");
    lua_pushinteger(L, ImGuiTreeNodeFlags_FramePadding);                lua_setfield(L, -2, "TreeNodeFlags_FramePadding");
    lua_pushinteger(L, ImGuiTreeNodeFlags_AllowItemOverlap);            lua_setfield(L, -2, "TreeNodeFlags_AllowItemOverlap");
    lua_pushinteger(L, ImGuiTreeNodeFlags_OpenOnArrow);                 lua_setfield(L, -2, "TreeNodeFlags_OpenOnArrow");
    lua_pushinteger(L, ImGuiTreeNodeFlags_SpanFullWidth);               lua_setfield(L, -2, "TreeNodeFlags_SpanFullWidth");
    lua_pushinteger(L, ImGuiTreeNodeFlags_NoAutoOpenOnLog);             lua_setfield(L, -2, "TreeNodeFlags_NoAutoOpenOnLog");
    lua_pushinteger(L, ImGuiTreeNodeFlags_Leaf);                        lua_setfield(L, -2, "TreeNodeFlags_Leaf");
    lua_pushinteger(L, ImGuiTreeNodeFlags_NoTreePushOnOpen);            lua_setfield(L, -2, "TreeNodeFlags_NoTreePushOnOpen");
    lua_pushinteger(L, ImGuiTreeNodeFlags_Selected);                    lua_setfield(L, -2, "TreeNodeFlags_Selected");
    lua_pushinteger(L, ImGuiTreeNodeFlags_SpanAvailWidth);              lua_setfield(L, -2, "TreeNodeFlags_SpanAvailWidth");
    lua_pushinteger(L, ImGuiTreeNodeFlags_OpenOnDoubleClick);           lua_setfield(L, -2, "TreeNodeFlags_OpenOnDoubleClick");
    lua_pushinteger(L, ImGuiTreeNodeFlags_DefaultOpen);                 lua_setfield(L, -2, "TreeNodeFlags_DefaultOpen");

    // ImGuiStyleVar
    lua_pushinteger(L, ImGuiStyleVar_GrabRounding);                     lua_setfield(L, -2, "StyleVar_GrabRounding");
    lua_pushinteger(L, ImGuiStyleVar_Alpha);                            lua_setfield(L, -2, "StyleVar_Alpha");
    lua_pushinteger(L, ImGuiStyleVar_WindowMinSize);                    lua_setfield(L, -2, "StyleVar_WindowMinSize");
    lua_pushinteger(L, ImGuiStyleVar_PopupBorderSize);                  lua_setfield(L, -2, "StyleVar_PopupBorderSize");
    lua_pushinteger(L, ImGuiStyleVar_WindowBorderSize);                 lua_setfield(L, -2, "StyleVar_WindowBorderSize");
    lua_pushinteger(L, ImGuiStyleVar_FrameBorderSize);                  lua_setfield(L, -2, "StyleVar_FrameBorderSize");
    lua_pushinteger(L, ImGuiStyleVar_ItemSpacing);                      lua_setfield(L, -2, "StyleVar_ItemSpacing");
    lua_pushinteger(L, ImGuiStyleVar_IndentSpacing);                    lua_setfield(L, -2, "StyleVar_IndentSpacing");
    lua_pushinteger(L, ImGuiStyleVar_FramePadding);                     lua_setfield(L, -2, "StyleVar_FramePadding");
    lua_pushinteger(L, ImGuiStyleVar_WindowPadding);                    lua_setfield(L, -2, "StyleVar_WindowPadding");
    lua_pushinteger(L, ImGuiStyleVar_ChildRounding);                    lua_setfield(L, -2, "StyleVar_ChildRounding");
    lua_pushinteger(L, ImGuiStyleVar_ItemInnerSpacing);                 lua_setfield(L, -2, "StyleVar_ItemInnerSpacing");
    lua_pushinteger(L, ImGuiStyleVar_WindowRounding);                   lua_setfield(L, -2, "StyleVar_WindowRounding");
    lua_pushinteger(L, ImGuiStyleVar_FrameRounding);                    lua_setfield(L, -2, "StyleVar_FrameRounding");
    lua_pushinteger(L, ImGuiStyleVar_TabRounding);                      lua_setfield(L, -2, "StyleVar_TabRounding");
    lua_pushinteger(L, ImGuiStyleVar_ChildBorderSize);                  lua_setfield(L, -2, "StyleVar_ChildBorderSize");
    lua_pushinteger(L, ImGuiStyleVar_GrabMinSize);                      lua_setfield(L, -2, "StyleVar_GrabMinSize");
    lua_pushinteger(L, ImGuiStyleVar_ScrollbarRounding);                lua_setfield(L, -2, "StyleVar_ScrollbarRounding");
    lua_pushinteger(L, ImGuiStyleVar_ScrollbarSize);                    lua_setfield(L, -2, "StyleVar_ScrollbarSize");
    lua_pushinteger(L, ImGuiStyleVar_WindowTitleAlign);                 lua_setfield(L, -2, "StyleVar_WindowTitleAlign");
    lua_pushinteger(L, ImGuiStyleVar_SelectableTextAlign);              lua_setfield(L, -2, "StyleVar_SelectableTextAlign");
    lua_pushinteger(L, ImGuiStyleVar_PopupRounding);                    lua_setfield(L, -2, "StyleVar_PopupRounding");
    lua_pushinteger(L, ImGuiStyleVar_ButtonTextAlign);                  lua_setfield(L, -2, "StyleVar_ButtonTextAlign");

    // ImGuiCol
    lua_pushinteger(L, ImGuiCol_PlotHistogram);                         lua_setfield(L, -2, "Col_PlotHistogram");
    lua_pushinteger(L, ImGuiCol_TitleBg);                               lua_setfield(L, -2, "Col_TitleBg");
    lua_pushinteger(L, ImGuiCol_Separator);                             lua_setfield(L, -2, "Col_Separator");
    lua_pushinteger(L, ImGuiCol_HeaderActive);                          lua_setfield(L, -2, "Col_HeaderActive");
    lua_pushinteger(L, ImGuiCol_HeaderHovered);                         lua_setfield(L, -2, "Col_HeaderHovered");
    lua_pushinteger(L, ImGuiCol_ButtonHovered);                         lua_setfield(L, -2, "Col_ButtonHovered");
    lua_pushinteger(L, ImGuiCol_NavWindowingHighlight);                 lua_setfield(L, -2, "Col_NavWindowingHighlight");
    lua_pushinteger(L, ImGuiCol_ScrollbarGrab);                         lua_setfield(L, -2, "Col_ScrollbarGrab");
    lua_pushinteger(L, ImGuiCol_FrameBg);                               lua_setfield(L, -2, "Col_FrameBg");
    lua_pushinteger(L, ImGuiCol_TextSelectedBg);                        lua_setfield(L, -2, "Col_TextSelectedBg");
    lua_pushinteger(L, ImGuiCol_ScrollbarGrabActive);                   lua_setfield(L, -2, "Col_ScrollbarGrabActive");
    //lua_pushinteger(L, ImGuiCol_ModalWindowDarkening);                  lua_setfield(L, -2, "Col_ModalWindowDarkening");
    lua_pushinteger(L, ImGuiCol_TitleBgCollapsed);                      lua_setfield(L, -2, "Col_TitleBgCollapsed");
    lua_pushinteger(L, ImGuiCol_ModalWindowDimBg);                      lua_setfield(L, -2, "Col_ModalWindowDimBg");
    lua_pushinteger(L, ImGuiCol_ResizeGripActive);                      lua_setfield(L, -2, "Col_ResizeGripActive");
    lua_pushinteger(L, ImGuiCol_SeparatorHovered);                      lua_setfield(L, -2, "Col_SeparatorHovered");
    lua_pushinteger(L, ImGuiCol_ScrollbarGrabHovered);                  lua_setfield(L, -2, "Col_ScrollbarGrabHovered");
    lua_pushinteger(L, ImGuiCol_TabUnfocused);                          lua_setfield(L, -2, "Col_TabUnfocused");
    lua_pushinteger(L, ImGuiCol_ScrollbarBg);                           lua_setfield(L, -2, "Col_ScrollbarBg");
    lua_pushinteger(L, ImGuiCol_ChildBg);                               lua_setfield(L, -2, "Col_ChildBg");
    lua_pushinteger(L, ImGuiCol_Header);                                lua_setfield(L, -2, "Col_Header");
    lua_pushinteger(L, ImGuiCol_NavWindowingDimBg);                     lua_setfield(L, -2, "Col_NavWindowingDimBg");
    lua_pushinteger(L, ImGuiCol_CheckMark);                             lua_setfield(L, -2, "Col_CheckMark");
    lua_pushinteger(L, ImGuiCol_Button);                                lua_setfield(L, -2, "Col_Button");
    lua_pushinteger(L, ImGuiCol_BorderShadow);                          lua_setfield(L, -2, "Col_BorderShadow");
    lua_pushinteger(L, ImGuiCol_DragDropTarget);                        lua_setfield(L, -2, "Col_DragDropTarget");
    lua_pushinteger(L, ImGuiCol_MenuBarBg);                             lua_setfield(L, -2, "Col_MenuBarBg");
    lua_pushinteger(L, ImGuiCol_TitleBgActive);                         lua_setfield(L, -2, "Col_TitleBgActive");
    lua_pushinteger(L, ImGuiCol_SeparatorActive);                       lua_setfield(L, -2, "Col_SeparatorActive");
    lua_pushinteger(L, ImGuiCol_Text);                                  lua_setfield(L, -2, "Col_Text");
    lua_pushinteger(L, ImGuiCol_PlotLinesHovered);                      lua_setfield(L, -2, "Col_PlotLinesHovered");
    lua_pushinteger(L, ImGuiCol_Border);                                lua_setfield(L, -2, "Col_Border");
    lua_pushinteger(L, ImGuiCol_TabUnfocusedActive);                    lua_setfield(L, -2, "Col_TabUnfocusedActive");
    lua_pushinteger(L, ImGuiCol_PlotLines);                             lua_setfield(L, -2, "Col_PlotLines");
    lua_pushinteger(L, ImGuiCol_PlotHistogramHovered);                  lua_setfield(L, -2, "Col_PlotHistogramHovered");
    lua_pushinteger(L, ImGuiCol_ResizeGripHovered);                     lua_setfield(L, -2, "Col_ResizeGripHovered");
    lua_pushinteger(L, ImGuiCol_Tab);                                   lua_setfield(L, -2, "Col_Tab");
    lua_pushinteger(L, ImGuiCol_TabHovered);                            lua_setfield(L, -2, "Col_TabHovered");
    lua_pushinteger(L, ImGuiCol_PopupBg);                               lua_setfield(L, -2, "Col_PopupBg");
    lua_pushinteger(L, ImGuiCol_TabActive);                             lua_setfield(L, -2, "Col_TabActive");
    lua_pushinteger(L, ImGuiCol_FrameBgActive);                         lua_setfield(L, -2, "Col_FrameBgActive");
    lua_pushinteger(L, ImGuiCol_ButtonActive);                          lua_setfield(L, -2, "Col_ButtonActive");
    lua_pushinteger(L, ImGuiCol_WindowBg);                              lua_setfield(L, -2, "Col_WindowBg");
    lua_pushinteger(L, ImGuiCol_SliderGrabActive);                      lua_setfield(L, -2, "Col_SliderGrabActive");
    lua_pushinteger(L, ImGuiCol_SliderGrab);                            lua_setfield(L, -2, "Col_SliderGrab");
    lua_pushinteger(L, ImGuiCol_NavHighlight);                          lua_setfield(L, -2, "Col_NavHighlight");
    lua_pushinteger(L, ImGuiCol_FrameBgHovered);                        lua_setfield(L, -2, "Col_FrameBgHovered");
    lua_pushinteger(L, ImGuiCol_TextDisabled);                          lua_setfield(L, -2, "Col_TextDisabled");
    lua_pushinteger(L, ImGuiCol_ResizeGrip);                            lua_setfield(L, -2, "Col_ResizeGrip");
#ifdef IMGUI_HAS_DOCK
    lua_pushinteger(L, ImGuiCol_DockingPreview);                        lua_setfield(L, -2, "Col_DockingPreview");
    lua_pushinteger(L, ImGuiCol_DockingEmptyBg);                        lua_setfield(L, -2, "Col_DockingEmptyBg");
#endif

    // ImGuiDataType
    lua_pushinteger(L, ImGuiDataType_U8);                               lua_setfield(L, -2, "DataType_U8");
    lua_pushinteger(L, ImGuiDataType_S64);                              lua_setfield(L, -2, "DataType_S64");
    lua_pushinteger(L, ImGuiDataType_Float);                            lua_setfield(L, -2, "DataType_Float");
    lua_pushinteger(L, ImGuiDataType_S16);                              lua_setfield(L, -2, "DataType_S16");
    lua_pushinteger(L, ImGuiDataType_U16);                              lua_setfield(L, -2, "DataType_U16");
    lua_pushinteger(L, ImGuiDataType_Double);                           lua_setfield(L, -2, "DataType_Double");
    lua_pushinteger(L, ImGuiDataType_S8);                               lua_setfield(L, -2, "DataType_S8");
    lua_pushinteger(L, ImGuiDataType_U32);                              lua_setfield(L, -2, "DataType_U32");
    lua_pushinteger(L, ImGuiDataType_S32);                              lua_setfield(L, -2, "DataType_S32");
    lua_pushinteger(L, ImGuiDataType_U64);                              lua_setfield(L, -2, "DataType_U64");

    // ImGuiDir
    lua_pushinteger(L, ImGuiDir_None);                                  lua_setfield(L, -2, "Dir_None");
    lua_pushinteger(L, ImGuiDir_Left);                                  lua_setfield(L, -2, "Dir_Left");
    lua_pushinteger(L, ImGuiDir_Up);                                    lua_setfield(L, -2, "Dir_Up");
    lua_pushinteger(L, ImGuiDir_Down);                                  lua_setfield(L, -2, "Dir_Down");
    lua_pushinteger(L, ImGuiDir_Right);                                 lua_setfield(L, -2, "Dir_Right");

    // ImGuiWindowFlags
    lua_pushinteger(L, ImGuiWindowFlags_NoScrollWithMouse);             lua_setfield(L, -2, "WindowFlags_NoScrollWithMouse");
    lua_pushinteger(L, ImGuiWindowFlags_None);                          lua_setfield(L, -2, "WindowFlags_None");
    lua_pushinteger(L, ImGuiWindowFlags_NoScrollbar);                   lua_setfield(L, -2, "WindowFlags_NoScrollbar");
    lua_pushinteger(L, ImGuiWindowFlags_HorizontalScrollbar);           lua_setfield(L, -2, "WindowFlags_HorizontalScrollbar");
    lua_pushinteger(L, ImGuiWindowFlags_NoFocusOnAppearing);            lua_setfield(L, -2, "WindowFlags_NoFocusOnAppearing");
    lua_pushinteger(L, ImGuiWindowFlags_NoBringToFrontOnFocus);         lua_setfield(L, -2, "WindowFlags_NoBringToFrontOnFocus");
    lua_pushinteger(L, ImGuiWindowFlags_NoDecoration);                  lua_setfield(L, -2, "WindowFlags_NoDecoration");
    lua_pushinteger(L, ImGuiWindowFlags_NoCollapse);                    lua_setfield(L, -2, "WindowFlags_NoCollapse");
    lua_pushinteger(L, ImGuiWindowFlags_NoTitleBar);                    lua_setfield(L, -2, "WindowFlags_NoTitleBar");
    lua_pushinteger(L, ImGuiWindowFlags_NoMove);                        lua_setfield(L, -2, "WindowFlags_NoMove");
    lua_pushinteger(L, ImGuiWindowFlags_NoInputs);                      lua_setfield(L, -2, "WindowFlags_NoInputs");
    lua_pushinteger(L, ImGuiWindowFlags_NoMouseInputs);                 lua_setfield(L, -2, "WindowFlags_NoMouseInputs");
    lua_pushinteger(L, ImGuiWindowFlags_NoSavedSettings);               lua_setfield(L, -2, "WindowFlags_NoSavedSettings");
    lua_pushinteger(L, ImGuiWindowFlags_NoNav);                         lua_setfield(L, -2, "WindowFlags_NoNav");
    lua_pushinteger(L, ImGuiWindowFlags_UnsavedDocument);               lua_setfield(L, -2, "WindowFlags_UnsavedDocument");
    lua_pushinteger(L, ImGuiWindowFlags_NoNavFocus);                    lua_setfield(L, -2, "WindowFlags_NoNavFocus");
    lua_pushinteger(L, ImGuiWindowFlags_AlwaysHorizontalScrollbar);     lua_setfield(L, -2, "WindowFlags_AlwaysHorizontalScrollbar");
    lua_pushinteger(L, ImGuiWindowFlags_AlwaysUseWindowPadding);        lua_setfield(L, -2, "WindowFlags_AlwaysUseWindowPadding");
    lua_pushinteger(L, ImGuiWindowFlags_NoNavInputs);                   lua_setfield(L, -2, "WindowFlags_NoNavInputs");
    lua_pushinteger(L, ImGuiWindowFlags_NoResize);                      lua_setfield(L, -2, "WindowFlags_NoResize");
    lua_pushinteger(L, ImGuiWindowFlags_AlwaysVerticalScrollbar);       lua_setfield(L, -2, "WindowFlags_AlwaysVerticalScrollbar");
    lua_pushinteger(L, ImGuiWindowFlags_MenuBar);                       lua_setfield(L, -2, "WindowFlags_MenuBar");
    lua_pushinteger(L, ImGuiWindowFlags_NoBackground);                  lua_setfield(L, -2, "WindowFlags_NoBackground");
    lua_pushinteger(L, ImGuiWindowFlags_AlwaysAutoResize);              lua_setfield(L, -2, "WindowFlags_AlwaysAutoResize");
#ifdef IMGUI_HAS_DOCK
    lua_pushinteger(L, ImGuiWindowFlags_NoDocking);                     lua_setfield(L, -2, "WindowFlags_NoDocking");
#endif
    //@MultiPain
    lua_pushinteger(L, ImGuiWindowFlags_FullScreen);                    lua_setfield(L, -2, "WindowFlags_FullScreen");


    // ImGuiTabItemFlags
    lua_pushinteger(L, ImGuiTabItemFlags_SetSelected);                  lua_setfield(L, -2, "TabItemFlags_SetSelected");
    lua_pushinteger(L, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton); lua_setfield(L, -2, "TabItemFlags_NoCloseWithMiddleMouseButton");
    lua_pushinteger(L, ImGuiTabItemFlags_NoTooltip);                    lua_setfield(L, -2, "TabItemFlags_NoTooltip");
    lua_pushinteger(L, ImGuiTabItemFlags_None);                         lua_setfield(L, -2, "TabItemFlags_None");
    lua_pushinteger(L, ImGuiTabItemFlags_NoPushId);                     lua_setfield(L, -2, "TabItemFlags_NoPushId");
    lua_pushinteger(L, ImGuiTabItemFlags_UnsavedDocument);              lua_setfield(L, -2, "TabItemFlags_UnsavedDocument");
    lua_pushinteger(L, ImGuiTabItemFlags_Leading);                      lua_setfield(L, -2, "TabItemFlags_Leading");   // 1.79
    lua_pushinteger(L, ImGuiTabItemFlags_Trailing);                     lua_setfield(L, -2, "TabItemFlags_Trailing");  // 1.79
    lua_pushinteger(L, ImGuiTabItemFlags_NoReorder);                    lua_setfield(L, -2, "TabItemFlags_NoReorder"); // 1.79

    // ImGuiComboFlags
    lua_pushinteger(L, ImGuiComboFlags_HeightSmall);                    lua_setfield(L, -2, "ComboFlags_HeightSmall");
    lua_pushinteger(L, ImGuiComboFlags_HeightLarge);                    lua_setfield(L, -2, "ComboFlags_HeightLarge");
    lua_pushinteger(L, ImGuiComboFlags_PopupAlignLeft);                 lua_setfield(L, -2, "ComboFlags_PopupAlignLeft");
    lua_pushinteger(L, ImGuiComboFlags_None);                           lua_setfield(L, -2, "ComboFlags_None");
    lua_pushinteger(L, ImGuiComboFlags_NoPreview);                      lua_setfield(L, -2, "ComboFlags_NoPreview");
    lua_pushinteger(L, ImGuiComboFlags_HeightRegular);                  lua_setfield(L, -2, "ComboFlags_HeightRegular");
    lua_pushinteger(L, ImGuiComboFlags_HeightMask_);                    lua_setfield(L, -2, "ComboFlags_HeightMask");
    lua_pushinteger(L, ImGuiComboFlags_NoArrowButton);                  lua_setfield(L, -2, "ComboFlags_NoArrowButton");
    lua_pushinteger(L, ImGuiComboFlags_HeightLargest);                  lua_setfield(L, -2, "ComboFlags_HeightLargest");

    // ImGuiCond
    lua_pushinteger(L, ImGuiCond_Appearing);                            lua_setfield(L, -2, "Cond_Appearing");
    lua_pushinteger(L, ImGuiCond_None);                                 lua_setfield(L, -2, "Cond_None");
    lua_pushinteger(L, ImGuiCond_Always);                               lua_setfield(L, -2, "Cond_Always");
    lua_pushinteger(L, ImGuiCond_FirstUseEver);                         lua_setfield(L, -2, "Cond_FirstUseEver");
    lua_pushinteger(L, ImGuiCond_Once);                                 lua_setfield(L, -2, "Cond_Once");

    // ImGuiSelectableFlags
    lua_pushinteger(L, ImGuiSelectableFlags_None);                      lua_setfield(L, -2, "SelectableFlags_None");
    lua_pushinteger(L, ImGuiSelectableFlags_SpanAllColumns);            lua_setfield(L, -2, "SelectableFlags_SpanAllColumns");
    lua_pushinteger(L, ImGuiSelectableFlags_AllowItemOverlap);          lua_setfield(L, -2, "SelectableFlags_AllowItemOverlap");
    lua_pushinteger(L, ImGuiSelectableFlags_DontClosePopups);           lua_setfield(L, -2, "SelectableFlags_DontClosePopups");
    lua_pushinteger(L, ImGuiSelectableFlags_AllowDoubleClick);          lua_setfield(L, -2, "SelectableFlags_AllowDoubleClick");
    lua_pushinteger(L, ImGuiSelectableFlags_Disabled);                  lua_setfield(L, -2, "SelectableFlags_Disabled");

    // ImGuiMouseCursor
    lua_pushinteger(L, ImGuiMouseCursor_None);                          lua_setfield(L, -2, "MouseCursor_None");
    lua_pushinteger(L, ImGuiMouseCursor_Arrow);                         lua_setfield(L, -2, "MouseCursor_Arrow");
    lua_pushinteger(L, ImGuiMouseCursor_TextInput);                     lua_setfield(L, -2, "MouseCursor_TextInput");
    lua_pushinteger(L, ImGuiMouseCursor_ResizeAll);                     lua_setfield(L, -2, "MouseCursor_ResizeAll");
    lua_pushinteger(L, ImGuiMouseCursor_ResizeNS);                      lua_setfield(L, -2, "MouseCursor_ResizeNS");
    lua_pushinteger(L, ImGuiMouseCursor_ResizeEW);                      lua_setfield(L, -2, "MouseCursor_ResizeEW");
    lua_pushinteger(L, ImGuiMouseCursor_ResizeNESW);                    lua_setfield(L, -2, "MouseCursor_ResizeNESW");
    lua_pushinteger(L, ImGuiMouseCursor_ResizeNWSE);                    lua_setfield(L, -2, "MouseCursor_ResizeNWSE");
    lua_pushinteger(L, ImGuiMouseCursor_Hand);                          lua_setfield(L, -2, "MouseCursor_Hand");
    lua_pushinteger(L, ImGuiMouseCursor_NotAllowed);                    lua_setfield(L, -2, "MouseCursor_NotAllowed");

    // ImGuiColorEditFlags
    lua_pushinteger(L, ImGuiColorEditFlags_AlphaPreview);               lua_setfield(L, -2, "ColorEditFlags_AlphaPreview");
    lua_pushinteger(L, ImGuiColorEditFlags_DisplayRGB);                 lua_setfield(L, -2, "ColorEditFlags_DisplayRGB");
    lua_pushinteger(L, ImGuiColorEditFlags_DisplayHex);                 lua_setfield(L, -2, "ColorEditFlags_DisplayHex");
    lua_pushinteger(L, ImGuiColorEditFlags_InputHSV);                   lua_setfield(L, -2, "ColorEditFlags_InputHSV");
    lua_pushinteger(L, ImGuiColorEditFlags_NoSidePreview);              lua_setfield(L, -2, "ColorEditFlags_NoSidePreview");
    lua_pushinteger(L, ImGuiColorEditFlags_Uint8);                      lua_setfield(L, -2, "ColorEditFlags_Uint8");
    //lua_pushinteger(L, ImGuiColorEditFlags_HEX);                        lua_setfield(L, -2, "ColorEditFlags_HEX");
    lua_pushinteger(L, ImGuiColorEditFlags_AlphaPreviewHalf);           lua_setfield(L, -2, "ColorEditFlags_AlphaPreviewHalf");
    lua_pushinteger(L, ImGuiColorEditFlags_Float);                      lua_setfield(L, -2, "ColorEditFlags_Float");
    lua_pushinteger(L, ImGuiColorEditFlags_PickerHueWheel);             lua_setfield(L, -2, "ColorEditFlags_PickerHueWheel");
    lua_pushinteger(L, ImGuiColorEditFlags__OptionsDefault);            lua_setfield(L, -2, "ColorEditFlags_OptionsDefault");
    lua_pushinteger(L, ImGuiColorEditFlags_InputRGB);                   lua_setfield(L, -2, "ColorEditFlags_InputRGB");
    lua_pushinteger(L, ImGuiColorEditFlags_HDR);                        lua_setfield(L, -2, "ColorEditFlags_HDR");
    lua_pushinteger(L, ImGuiColorEditFlags_NoPicker);                   lua_setfield(L, -2, "ColorEditFlags_NoPicker");
    //lua_pushinteger(L, ImGuiColorEditFlags_RGB);                        lua_setfield(L, -2, "ColorEditFlags_RGB");
    lua_pushinteger(L, ImGuiColorEditFlags_AlphaBar);                   lua_setfield(L, -2, "ColorEditFlags_AlphaBar");
    lua_pushinteger(L, ImGuiColorEditFlags_DisplayHSV);                 lua_setfield(L, -2, "ColorEditFlags_DisplayHSV");
    lua_pushinteger(L, ImGuiColorEditFlags_PickerHueBar);               lua_setfield(L, -2, "ColorEditFlags_PickerHueBar");
    //lua_pushinteger(L, ImGuiColorEditFlags_HSV);                        lua_setfield(L, -2, "ColorEditFlags_HSV");
    lua_pushinteger(L, ImGuiColorEditFlags_NoAlpha);                    lua_setfield(L, -2, "ColorEditFlags_NoAlpha");
    lua_pushinteger(L, ImGuiColorEditFlags_NoOptions);                  lua_setfield(L, -2, "ColorEditFlags_NoOptions");
    lua_pushinteger(L, ImGuiColorEditFlags_NoDragDrop);                 lua_setfield(L, -2, "ColorEditFlags_NoDragDrop");
    lua_pushinteger(L, ImGuiColorEditFlags_NoInputs);                   lua_setfield(L, -2, "ColorEditFlags_NoInputs");
    lua_pushinteger(L, ImGuiColorEditFlags_None);                       lua_setfield(L, -2, "ColorEditFlags_None");
    lua_pushinteger(L, ImGuiColorEditFlags_NoSmallPreview);             lua_setfield(L, -2, "ColorEditFlags_NoSmallPreview");
    lua_pushinteger(L, ImGuiColorEditFlags_NoBorder);                   lua_setfield(L, -2, "ColorEditFlags_NoBorder");
    lua_pushinteger(L, ImGuiColorEditFlags_NoLabel);                    lua_setfield(L, -2, "ColorEditFlags_NoLabel");
    lua_pushinteger(L, ImGuiColorEditFlags_NoTooltip);                  lua_setfield(L, -2, "ColorEditFlags_NoTooltip");

    // ImGuiDragDropFlags
    lua_pushinteger(L, ImGuiDragDropFlags_SourceNoPreviewTooltip);      lua_setfield(L, -2, "DragDropFlags_SourceNoPreviewTooltip");
    lua_pushinteger(L, ImGuiDragDropFlags_SourceAllowNullID);           lua_setfield(L, -2, "DragDropFlags_SourceAllowNullID");
    lua_pushinteger(L, ImGuiDragDropFlags_AcceptNoDrawDefaultRect);     lua_setfield(L, -2, "DragDropFlags_AcceptNoDrawDefaultRect");
    lua_pushinteger(L, ImGuiDragDropFlags_AcceptPeekOnly);              lua_setfield(L, -2, "DragDropFlags_AcceptPeekOnly");
    lua_pushinteger(L, ImGuiDragDropFlags_AcceptBeforeDelivery);        lua_setfield(L, -2, "DragDropFlags_AcceptBeforeDelivery");
    lua_pushinteger(L, ImGuiDragDropFlags_SourceNoHoldToOpenOthers);    lua_setfield(L, -2, "DragDropFlags_SourceNoHoldToOpenOthers");
    lua_pushinteger(L, ImGuiDragDropFlags_AcceptNoPreviewTooltip);      lua_setfield(L, -2, "DragDropFlags_AcceptNoPreviewTooltip");
    lua_pushinteger(L, ImGuiDragDropFlags_SourceAutoExpirePayload);     lua_setfield(L, -2, "DragDropFlags_SourceAutoExpirePayload");
    lua_pushinteger(L, ImGuiDragDropFlags_SourceExtern);                lua_setfield(L, -2, "DragDropFlags_SourceExtern");
    lua_pushinteger(L, ImGuiDragDropFlags_None);                        lua_setfield(L, -2, "DragDropFlags_None");
    lua_pushinteger(L, ImGuiDragDropFlags_SourceNoDisableHover);        lua_setfield(L, -2, "DragDropFlags_SourceNoDisableHover");

    // ImDrawCornerFlags
    lua_pushinteger(L, ImDrawCornerFlags_None);                         lua_setfield(L, -2, "CornerFlags_None");
    lua_pushinteger(L, ImDrawCornerFlags_TopLeft);                      lua_setfield(L, -2, "CornerFlags_TopLeft");
    lua_pushinteger(L, ImDrawCornerFlags_TopRight);                     lua_setfield(L, -2, "CornerFlags_TopRight");
    lua_pushinteger(L, ImDrawCornerFlags_BotLeft);                      lua_setfield(L, -2, "CornerFlags_BotLeft");
    lua_pushinteger(L, ImDrawCornerFlags_BotRight);                     lua_setfield(L, -2, "CornerFlags_BotRight");
    lua_pushinteger(L, ImDrawCornerFlags_Top);                          lua_setfield(L, -2, "CornerFlags_Top");
    lua_pushinteger(L, ImDrawCornerFlags_Bot);                          lua_setfield(L, -2, "CornerFlags_Bot");
    lua_pushinteger(L, ImDrawCornerFlags_Left);                         lua_setfield(L, -2, "CornerFlags_Left");
    lua_pushinteger(L, ImDrawCornerFlags_Right);                        lua_setfield(L, -2, "CornerFlags_Right");
    lua_pushinteger(L, ImDrawCornerFlags_All);                          lua_setfield(L, -2, "CornerFlags_All");

    // 1.78* NEW*
    //ImGuiSliderFlags
    lua_pushinteger(L, ImGuiSliderFlags_None);                          lua_setfield(L, -2, "SliderFlags_None");
    lua_pushinteger(L, ImGuiSliderFlags_AlwaysClamp);                   lua_setfield(L, -2, "SliderFlags_ClampOnInput"); // backward capability
    lua_pushinteger(L, ImGuiSliderFlags_AlwaysClamp);                   lua_setfield(L, -2, "SliderFlags_AlwaysClamp");
    lua_pushinteger(L, ImGuiSliderFlags_Logarithmic);                   lua_setfield(L, -2, "SliderFlags_Logarithmic");
    lua_pushinteger(L, ImGuiSliderFlags_NoRoundToFormat);               lua_setfield(L, -2, "SliderFlags_NoRoundToFormat");
    lua_pushinteger(L, ImGuiSliderFlags_NoInput);                       lua_setfield(L, -2, "SliderFlags_NoInput");

    // ImGuiConfigFlags
    lua_pushinteger(L, ImGuiConfigFlags_None);                          lua_setfield(L, -2, "ConfigFlags_None");
    lua_pushinteger(L, ImGuiConfigFlags_NavEnableKeyboard);             lua_setfield(L, -2, "ConfigFlags_NavEnableKeyboard");
    lua_pushinteger(L, ImGuiConfigFlags_NavEnableGamepad);              lua_setfield(L, -2, "ConfigFlags_NavEnableGamepad");
    lua_pushinteger(L, ImGuiConfigFlags_NavEnableSetMousePos);          lua_setfield(L, -2, "ConfigFlags_NavEnableSetMousePos");
    lua_pushinteger(L, ImGuiConfigFlags_NavNoCaptureKeyboard);          lua_setfield(L, -2, "ConfigFlags_NavNoCaptureKeyboard");
    lua_pushinteger(L, ImGuiConfigFlags_NoMouse);                       lua_setfield(L, -2, "ConfigFlags_NoMouse");
    lua_pushinteger(L, ImGuiConfigFlags_NoMouseCursorChange);           lua_setfield(L, -2, "ConfigFlags_NoMouseCursorChange");
    lua_pushinteger(L, ImGuiConfigFlags_IsSRGB);                        lua_setfield(L, -2, "ConfigFlags_IsSRGB");
    lua_pushinteger(L, ImGuiConfigFlags_IsTouchScreen);                 lua_setfield(L, -2, "ConfigFlags_IsTouchScreen");
#ifdef IMGUI_HAS_DOCK
    lua_pushinteger(L, ImGuiConfigFlags_DockingEnable);                 lua_setfield(L, -2, "ConfigFlags_DockingEnable");

    // ImGuiDockNodeFlags
    lua_pushinteger(L, ImGuiDockNodeFlags_None);                        lua_setfield(L, -2, "DockNodeFlags_None");
    lua_pushinteger(L, ImGuiDockNodeFlags_KeepAliveOnly);               lua_setfield(L, -2, "DockNodeFlags_KeepAliveOnly");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoDockingInCentralNode);      lua_setfield(L, -2, "DockNodeFlags_NoDockingInCentralNode");
    lua_pushinteger(L, ImGuiDockNodeFlags_PassthruCentralNode);         lua_setfield(L, -2, "DockNodeFlags_PassthruCentralNode");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoSplit);                     lua_setfield(L, -2, "DockNodeFlags_NoSplit");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoResize);                    lua_setfield(L, -2, "DockNodeFlags_NoResize");
    lua_pushinteger(L, ImGuiDockNodeFlags_AutoHideTabBar);              lua_setfield(L, -2, "DockNodeFlags_AutoHideTabBar");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoWindowMenuButton);          lua_setfield(L, -2, "DockNodeFlags_NoWindowMenuButton");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoCloseButton);               lua_setfield(L, -2, "DockNodeFlags_NoCloseButton");
#endif

    // @MultiPain
    // ImGuiGlyphRanges
    lua_pushinteger(L, ImGuiGlyphRanges_Default);                       lua_setfield(L, -2, "GlyphRanges_Default");
    lua_pushinteger(L, ImGuiGlyphRanges_Korean);                        lua_setfield(L, -2, "GlyphRanges_Korean");
    lua_pushinteger(L, ImGuiGlyphRanges_ChineseFull);                   lua_setfield(L, -2, "GlyphRanges_ChineseFull");
    lua_pushinteger(L, ImGuiGlyphRanges_ChineseSimplifiedCommon);       lua_setfield(L, -2, "GlyphRanges_ChineseSimplifiedCommon");
    lua_pushinteger(L, ImGuiGlyphRanges_Japanese);                      lua_setfield(L, -2, "GlyphRanges_Japanese");
    lua_pushinteger(L, ImGuiGlyphRanges_Cyrillic);                      lua_setfield(L, -2, "GlyphRanges_Cyrillic");
    lua_pushinteger(L, ImGuiGlyphRanges_Thai);                          lua_setfield(L, -2, "GlyphRanges_Thai");
    lua_pushinteger(L, ImGuiGlyphRanges_Vietnamese);                    lua_setfield(L, -2, "GlyphRanges_Vietnamese");

    // ImGuiItemFlags
    lua_pushinteger(L, ImGuiItemFlags_Disabled);                        lua_setfield(L, -2, "ItemFlags_Disabled");
    lua_pushinteger(L, ImGuiItemFlags_ButtonRepeat);                    lua_setfield(L, -2, "ItemFlags_ButtonRepeat");

    // ImGuiNavInput
    lua_pushinteger(L, ImGuiNavInput_FocusNext);                        lua_setfield(L, -2, "NavInput_FocusNext");
    lua_pushinteger(L, ImGuiNavInput_TweakFast);                        lua_setfield(L, -2, "NavInput_TweakFast");
    lua_pushinteger(L, ImGuiNavInput_Input);                            lua_setfield(L, -2, "NavInput_Input");
    lua_pushinteger(L, ImGuiNavInput_DpadRight);                        lua_setfield(L, -2, "NavInput_DpadRight");
    lua_pushinteger(L, ImGuiNavInput_FocusPrev);                        lua_setfield(L, -2, "NavInput_FocusPrev");
    lua_pushinteger(L, ImGuiNavInput_LStickDown);                       lua_setfield(L, -2, "NavInput_LStickDown");
    lua_pushinteger(L, ImGuiNavInput_LStickUp);                         lua_setfield(L, -2, "NavInput_LStickUp");
    lua_pushinteger(L, ImGuiNavInput_Activate);                         lua_setfield(L, -2, "NavInput_Activate");
    lua_pushinteger(L, ImGuiNavInput_LStickLeft);                       lua_setfield(L, -2, "NavInput_LStickLeft");
    lua_pushinteger(L, ImGuiNavInput_LStickRight);                      lua_setfield(L, -2, "NavInput_LStickRight");
    lua_pushinteger(L, ImGuiNavInput_DpadLeft);                         lua_setfield(L, -2, "NavInput_DpadLeft");
    lua_pushinteger(L, ImGuiNavInput_DpadDown);                         lua_setfield(L, -2, "NavInput_DpadDown");
    lua_pushinteger(L, ImGuiNavInput_TweakSlow);                        lua_setfield(L, -2, "NavInput_TweakSlow");
    lua_pushinteger(L, ImGuiNavInput_DpadUp);                           lua_setfield(L, -2, "NavInput_DpadUp");
    lua_pushinteger(L, ImGuiNavInput_Menu);                             lua_setfield(L, -2, "NavInput_Menu");
    lua_pushinteger(L, ImGuiNavInput_Cancel);                           lua_setfield(L, -2, "NavInput_Cancel");

    lua_pop(L, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// NodeContext
///
/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef IS_BETA_BUILD

class NodeEditor
{
public:
    ED::EditorContext* ctx;
    NodeEditor(ED::Config* config = nullptr)
    {
        ctx = ED::CreateEditor(config);
    }
    ~NodeEditor()
    {
        ED::DestroyEditor(ctx);
    }
};

#endif


/////////////////////////////////////////////////////////////////////////////////////////////
///
/// GidImGui
///
/////////////////////////////////////////////////////////////////////////////////////////////
class EventListener;

class GidImGui
{
public:
    GidImGui(LuaApplication* application, lua_State* L,
             bool addMouseListeners, bool addKeyboardListeners, bool addTouchListeners);
    ~GidImGui();

    EventListener* eventListener;

    void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
    LuaApplication* application;
    VertexBuffer<Point2f> vertices;
    VertexBuffer<Point2f> texcoords;
    VertexBuffer<VColor> colors;
};

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// EVENT LISTENER
///
/////////////////////////////////////////////////////////////////////////////////////////////

class EventListener : public EventDispatcher
{
private:
    void keyUpOrDown(int keyCode, bool state)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[keyCode] = state;

        int mod = getKeyboardModifiers(L);

        if (!mod)
        {
            if (keyCode == GINPUT_KEY_SHIFT)
                io.KeyShift = state;
            if (keyCode == GINPUT_KEY_CTRL)
                io.KeyCtrl = state;
            if (keyCode == GINPUT_KEY_ALT)
                io.KeyAlt = state;
        }
        else
        {
            io.KeyAlt = mod & GINPUT_ALT_MODIFIER;
            io.KeyCtrl = mod & GINPUT_CTRL_MODIFIER;
            io.KeyShift = mod & GINPUT_SHIFT_MODIFIER;
            io.KeySuper = mod & GINPUT_META_MODIFIER;
        }
    }

    void mouseUpOrDown(float x, float y, int button, bool state)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[button] = state;
        io.MousePos = translateMousePos(x, y);
    }

    void scaleMouseCoords(float& x, float& y)
    {
        x = x * r_app_scale.x + app_bounds.x;
        y = y * r_app_scale.y + app_bounds.y;
    }

public:
    ImVec2 r_app_scale;
    ImVec2 app_bounds;

    EventListener()
    {
        applicationResize(nullptr);
    }

    ~EventListener() { }

    static ImVec2 translateMousePos(float x, float y)
    {
        std::stack<const Sprite*> stack;
        float z;

        const Sprite* curr = imguiProxy;
        while (curr)
        {
            stack.push(curr);
            curr = curr->parent();
        }

        while (!stack.empty())
        {
            stack.top()->matrix().inverseTransformPoint(x, y, 0, &x, &y, &z);
            stack.pop();
        }
        return ImVec2(x, y);
    }

    ///////////////////////////////////////////////////
    ///
    /// MOUSE
    ///
    ///////////////////////////////////////////////////

    void mouseDown(MouseEvent* event)
    {
        float x = (float)event->x;
        float y = (float)event->y;
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, convertGiderosMouseButton(event->button), true);
    }

    void mouseDown(float x, float y, int button)
    {
        mouseUpOrDown(x, y, convertGiderosMouseButton(button), true);
    }

    void mouseUp(MouseEvent* event)
    {
        float x = (float)event->x;
        float y = (float)event->y;
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, convertGiderosMouseButton(event->button), false);
    }

    void mouseUp(float x, float y, int button)
    {
        mouseUpOrDown(x, y, convertGiderosMouseButton(button), false);
    }

    void mouseMove(float x, float y, int button)
    {
        mouseUpOrDown(x, y, convertGiderosMouseButton(button), true);
    }

    void mouseHover(MouseEvent* event)
    {
        float x = (float)event->x;
        float y = (float)event->y;
        scaleMouseCoords(x, y);
        mouseHover(x, y);
    }

    void mouseHover(float x, float y)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = translateMousePos(x, y);
    }

    void mouseWheel(MouseEvent* event)
    {
        float x = (float)event->x;
        float y = (float)event->y;
        scaleMouseCoords(x, y);
        mouseWheel(x, y, event->wheel);
    }

    void mouseWheel(float x, float y, int wheel)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel += wheel < 0 ? -1.0f : 1.0f;
        io.MousePos = translateMousePos(x, y);
    }

    ///////////////////////////////////////////////////
    ///
    /// TOUCH
    ///
    ///////////////////////////////////////////////////

    void touchesBegin(TouchEvent* event)
    {
        float x = event->event->touch.x;
        float y = event->event->touch.y;
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, 0, true);
    }

    void touchesBegin(float x, float y)
    {
        mouseUpOrDown(x, y, 0, true);
    }

    void touchesEnd(TouchEvent* event)
    {
        float x;
        float y;
        if (resetTouchPosOnEnd)
        {
            x = FLT_MAX;
            y = FLT_MAX;
        }
        else
        {
            x = event->event->touch.x;
            y = event->event->touch.y;
        }
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, 0, false);
    }

    void touchesEnd(float x, float y)
    {
        mouseUpOrDown(x, y, 0, false);
    }

    void touchesMove(TouchEvent* event)
    {
        float x = event->event->touch.x;
        float y = event->event->touch.y;
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, 0, true);
    }

    void touchesMove(float x, float y)
    {
        mouseUpOrDown(x, y, 0, true);
    }

    void touchesCancel(TouchEvent* event)
    {
        float x;
        float y;
        if (resetTouchPosOnEnd)
        {
            x = FLT_MAX;
            y = FLT_MAX;
        }
        else
        {
            x = event->event->touch.x;
            y = event->event->touch.y;
        }
        scaleMouseCoords(x, y);
        mouseUpOrDown(x, y, 0, false);
    }

    void touchesCancel(float x, float y)
    {
        mouseUpOrDown(x, y, 0, false);
    }

    ///////////////////////////////////////////////////
    ///
    /// KEYBAORD
    ///
    ///////////////////////////////////////////////////

    void keyDown(KeyboardEvent* event)
    {
        keyDown(event->keyCode);
    }

    void keyDown(int keyCode)
    {
        keyUpOrDown(keyCode, true);
    }

    void keyUp(KeyboardEvent* event)
    {
        keyUpOrDown(event->keyCode, false);
    }

    void keyUp(int keyCode)
    {
        keyUpOrDown(keyCode, false);
    }

    void keyChar(KeyboardEvent* event)
    {
        keyChar(event->charCode);
    }

    void keyChar(std::string text)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(text.c_str());
    }

    void keyChar2(const char* text) // error when adding event listener to a proxy in GidImGui constructor
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(text);
    }

    void applicationResize(Event *)
    {
        lua_getglobal(L, "application");
        lua_getfield(L, -1, "getLogicalScaleX");
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        float sx = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "getLogicalScaleY");
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        float sy = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "getLogicalBounds");
        lua_pushvalue(L, -2);
        lua_call(L, 1, 4);
        app_bounds.x = luaL_checknumber(L, -4);
        app_bounds.y = luaL_checknumber(L, -3);
        lua_pop(L, 5);

        r_app_scale.x = 1.0f / sx;
        r_app_scale.y = 1.0f / sy;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// SPRITE PROXY
///
/////////////////////////////////////////////////////////////////////////////////////////////

static void _Draw(void* c, const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
    ((GidImGui* ) c)->doDraw(t, sx, sy, ex, ey);
}

static void _Destroy(void* c)
{
    delete ((GidImGui* ) c);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CONSREUCTOR / DESTRUCTOR / DRAW
///
/////////////////////////////////////////////////////////////////////////////////////////////

GidImGui::GidImGui(LuaApplication* application, lua_State* _UNUSED(L),
                   bool addMouseListeners = true, bool addKeyboardListeners = true, bool addTouchListeners = false)
{
    this->application = application;
    imguiProxy = gtexture_get_spritefactory()->createProxy(application->getApplication(), this, _Draw, _Destroy);
    eventListener = new EventListener();

    if (addMouseListeners)
    {
        imguiProxy->addEventListener(MouseEvent::MOUSE_DOWN,     eventListener, &EventListener::mouseDown);
        imguiProxy->addEventListener(MouseEvent::MOUSE_UP,       eventListener, &EventListener::mouseUp);
        imguiProxy->addEventListener(MouseEvent::MOUSE_MOVE,     eventListener, &EventListener::mouseDown);
        imguiProxy->addEventListener(MouseEvent::MOUSE_HOVER,    eventListener, &EventListener::mouseHover);
        imguiProxy->addEventListener(MouseEvent::MOUSE_WHEEL,    eventListener, &EventListener::mouseWheel);
    }

    if (addTouchListeners)
    {
        imguiProxy->addEventListener(TouchEvent::TOUCHES_BEGIN,  eventListener, &EventListener::touchesBegin);
        imguiProxy->addEventListener(TouchEvent::TOUCHES_END,    eventListener, &EventListener::touchesEnd);
        imguiProxy->addEventListener(TouchEvent::TOUCHES_MOVE,   eventListener, &EventListener::touchesMove);
        imguiProxy->addEventListener(TouchEvent::TOUCHES_CANCEL, eventListener, &EventListener::touchesCancel);
    }

    if (addKeyboardListeners)
    {
        imguiProxy->addEventListener(KeyboardEvent::KEY_DOWN,    eventListener, &EventListener::keyDown);
        imguiProxy->addEventListener(KeyboardEvent::KEY_UP,      eventListener, &EventListener::keyUp);
        imguiProxy->addEventListener(KeyboardEvent::KEY_CHAR,    eventListener, &EventListener::keyChar);
    }

    imguiProxy->addEventListener(Event::APPLICATION_RESIZE,  eventListener, &EventListener::applicationResize);
}

GidImGui::~GidImGui()
{
    imguiProxy->removeEventListeners();
    delete eventListener;
    delete imguiProxy;
}

void GidImGui::doDraw(const CurrentTransform&, float _UNUSED(sx), float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey))
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data) return;

    ShaderEngine* engine=gtexture_get_engine();
    ShaderProgram* shp=engine->getDefault(ShaderEngine::STDP_TEXTURECOLOR);
    ImVec2 pos = draw_data->DisplayPos;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // vertex buffer generated by Dear ImGui
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // index buffer generated by Dear ImGui

        size_t vtx_size=cmd_list->VtxBuffer.Size;

        vertices.resize(vtx_size);
        texcoords.resize(vtx_size);
        colors.resize(vtx_size);

        for (size_t i=0;i<vtx_size;i++)
        {
            vertices[i].x = vtx_buffer[i].pos.x;
            vertices[i].y = vtx_buffer[i].pos.y;
            texcoords[i].x = vtx_buffer[i].uv.x;
            texcoords[i].y = vtx_buffer[i].uv.y;

            uint32_t c = vtx_buffer[i].col;

            uint32_t r = (c >> IM_COL32_R_SHIFT) & 0xFF;
            uint32_t g = (c >> IM_COL32_G_SHIFT) & 0xFF;
            uint32_t b = (c >> IM_COL32_B_SHIFT) & 0xFF;
            uint32_t a = (c >> IM_COL32_A_SHIFT) & 0xFF;

            colors[i].r = (r * a) >> 8;
            colors[i].g = (g * a) >> 8;
            colors[i].b = (b * a) >> 8;
            colors[i].a = a;
        }
        vertices.Update();
        texcoords.Update();
        colors.Update();
        shp->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT,2, &vertices[0], vtx_size, true, NULL);
        shp->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT,2, &texcoords[0], vtx_size, true, NULL);
        shp->setData(ShaderProgram::DataColor, ShaderProgram::DUBYTE,4, &colors[0], vtx_size, true, NULL);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                g_id textureId = (g_id)pcmd->TextureId;

                engine->bindTexture(0, gtexture_getInternalTexture(textureId));

                engine->pushClip(
                            (int)(pcmd->ClipRect.x - pos.x),
                            (int)(pcmd->ClipRect.y - pos.y),
                            (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                            (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)
                            );
                shp->drawElements(ShaderProgram::Triangles, pcmd->ElemCount,ShaderProgram::DUSHORT, idx_buffer, true, NULL);
                engine->popClip();

            }
            idx_buffer += pcmd->ElemCount;
        }

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

GidImGui* getImgui(lua_State* L)
{
    Binder binder(L);
    SpriteProxy* sprite = static_cast<SpriteProxy*>(binder.getInstance("ImGui", 1));
    return (GidImGui*)sprite->getContext();
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ImGui create / destroy
///
/////////////////////////////////////////////////////////////////////////////////////////////

int initImGui(lua_State* L)
{
    LUA_ASSERT(!instanceCreated, "ImGui instance already exists! Please, consider using single ImGui object OR delete previous instance first!");

    instanceCreated = true;
    autoUpdateCursor = false;

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    ::application = application->getApplication();

    // init ImGui itself
    ImGui::CreateContext();

    // Setup style theme
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size
    io.DisplaySize.x = getAppProperty(L, "getContentWidth");
    io.DisplaySize.y = getAppProperty(L, "getContentHeight");

    io.BackendPlatformName = "Gideros Studio";
    io.BackendRendererName = "Gideros Studio";

    // Keyboard map
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = GINPUT_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = GINPUT_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = GINPUT_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = GINPUT_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = GINPUT_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = GINPUT_KEY_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]    = GINPUT_KEY_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]        = GINPUT_KEY_HOME;
    io.KeyMap[ImGuiKey_End]         = GINPUT_KEY_END;
    io.KeyMap[ImGuiKey_Delete]      = GINPUT_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = GINPUT_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]       = GINPUT_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = GINPUT_KEY_ESC;
    io.KeyMap[ImGuiKey_Insert]      = GINPUT_KEY_INSERT;
    io.KeyMap[ImGuiKey_A]           = GINPUT_KEY_A;
    io.KeyMap[ImGuiKey_C]           = GINPUT_KEY_C;
    io.KeyMap[ImGuiKey_V]           = GINPUT_KEY_V;
    io.KeyMap[ImGuiKey_X]           = GINPUT_KEY_X;
    io.KeyMap[ImGuiKey_Y]           = GINPUT_KEY_Y;
    io.KeyMap[ImGuiKey_Z]           = GINPUT_KEY_Z;

    // Create font atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_id texture = gtexture_create(width, height, GTEXTURE_RGBA, GTEXTURE_UNSIGNED_BYTE, GTEXTURE_CLAMP, GTEXTURE_LINEAR, pixels, NULL, 0);
    io.Fonts->TexID = (void*)texture;

    Binder binder(L);
    GidImGui* imgui = new GidImGui(application, L, luaL_optboolean(L, 1, 1), luaL_optboolean(L, 2, 1), luaL_optboolean(L, 3, 0));
    //GidImGuiPtr = imgui;
    binder.pushInstance("ImGui", imguiProxy);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, imgui);
    lua_pop(L, 1);

    return 1;
}

int destroyImGui(lua_State* L)
{
    resetStaticVars();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->ClearTexData();

    if (io.MouseDrawCursor)
        setApplicationCursor(L, "arrow");

    ImGui::DestroyContext();

    //imguiProxy->removeEventListeners();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// BINDINGS.
///
/////////////////////////////////////////////////////////////////////////////////////////////

/// MOUSE INPUTS

int MouseHover(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");

    imgui->eventListener->mouseHover(event_x, event_y);

    return 0;
}

int MouseMove(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");
    int button = getfield(L, "button");

    imgui->eventListener->mouseMove(event_x, event_y, button);

    return 0;
}

int MouseDown(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");
    int button = getfield(L, "button");

    imgui->eventListener->mouseDown(event_x, event_y, button);

    return 0;
}

int MouseUp(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");
    int button = getfield(L, "button");

    imgui->eventListener->mouseUp(event_x, event_y, button);

    return 0;
}

int MouseWheel(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");
    int wheel = getfield(L, "wheel");

    imgui->eventListener->mouseWheel(event_x, event_y, wheel);

    return 0;
}

/// TOUCH INPUT


int TouchCancel(lua_State* L)
{
    GidImGui* imgui = getImgui(L);
    float x = getsubfield(L, "touch", "x");
    float y = getsubfield(L, "touch", "y");
    imgui->eventListener->touchesCancel(x, y);
    return 0;
}

int TouchMove(lua_State* L)
{
    GidImGui* imgui = getImgui(L);
    float x = getsubfield(L, "touch", "x");
    float y = getsubfield(L, "touch", "y");
    imgui->eventListener->touchesMove(x, y);
    return 0;
}

int TouchBegin(lua_State* L)
{
    GidImGui* imgui = getImgui(L);
    float x = getsubfield(L, "touch", "x");
    float y = getsubfield(L, "touch", "y");
    imgui->eventListener->touchesBegin(x, y);
    return 0;
}

int TouchEnd(lua_State* L)
{
    GidImGui* imgui = getImgui(L);
    float x = getsubfield(L, "touch", "x");
    float y = getsubfield(L, "touch", "y");
    imgui->eventListener->touchesEnd(x, y);
    return 0;
}

/// KEYBOARD INPUTS

int KeyUp(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    int keyCode = getfield(L, "keyCode");

    imgui->eventListener->keyUp(keyCode);

    return 0;
}

int KeyDown(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    int keyCode = getfield(L, "keyCode");

    imgui->eventListener->keyDown(keyCode);

    return 0;
}

int KeyChar(lua_State* L)
{
    GidImGui* imgui = getImgui(L);

    lua_pushstring(L, "text");
    lua_gettable(L, -2);
    const char* text = lua_tostring(L, -1);
    lua_pop(L, 1);

    imgui->eventListener->keyChar2(text);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// DRAWING STUFF
///
/////////////////////////////////////////////////////////////////////////////////////////////

int NewFrame(lua_State* L)
{
    double deltaTime = getfield(L, "deltaTime");

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;
    ImGui::NewFrame();

    return 0;
}

int Render(lua_State* L)
{
    if (autoUpdateCursor)
    {
        ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
        const char* cursorName = giderosCursorMap[cursor];
        setApplicationCursor(L, cursorName);
    }
    ImGui::Render();
    return 0;
}

int EndFrame(lua_State* _UNUSED(L))
{
    ImGui::EndFrame();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Windows
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Begin(lua_State* L)
{
    const char* name = luaL_checkstring(L, 2);
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);

    bool* p_open = getPopen(L, 3);

    bool draw_flag = ImGui::Begin(name, p_open, flags);

    int ret = 1;
    if (p_open != NULL)
    {
        lua_pushboolean(L, *p_open);
        delete p_open;
        ret++;
    }

    lua_pushboolean(L, draw_flag);
    return ret;
}

int End(lua_State* _UNUSED(L))
{
    ImGui::End();
    return 0;
}

// @MultiPain
int BeginFullScreenWindow(lua_State* L)
{
    const char* name = luaL_checkstring(L, 2);
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);
    flags |= ImGuiWindowFlags_FullScreen;

    bool* p_open = getPopen(L, 3);

    ImGuiIO& IO = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(IO.DisplaySize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    bool draw_flag = ImGui::Begin(name, p_open, flags);

    ImGui::PopStyleVar(2);

    int ret = 1;
    if (p_open != NULL)
    {
        lua_pushboolean(L, *p_open);
        delete p_open;
        ret++;
    }

    lua_pushboolean(L, draw_flag);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Child Windows
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginChild(lua_State* L)
{
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, 0.0f), luaL_optnumber(L, 4, 0.0f));
    bool border = luaL_optboolean(L, 5, 0);
    ImGuiWindowFlags flags = luaL_optinteger(L, 6, 0);
    bool result;

    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char* str_id = luaL_checkstring(L, 2);
        result = ImGui::BeginChild(str_id, size, border, flags);
    }
    else
    {
        ImGuiID id = checkID(L);
        result = ImGui::BeginChild(id, size, border, flags);
    }

    lua_pushboolean(L, result);
    return 1;
}

int EndChild(lua_State* _UNUSED(L))
{
    ImGui::EndChild();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Windows Utilities
///
/////////////////////////////////////////////////////////////////////////////////////////////

int IsWindowAppearing(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowAppearing());
    return 1;
}

int IsWindowCollapsed(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowCollapsed());
    return 1;
}

int IsWindowFocused(lua_State* L)
{
    ImGuiFocusedFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowFocused(flag));
    return 1;
}

int IsWindowHovered(lua_State* L)
{
    ImGuiHoveredFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowHovered(flag));
    return 1;
}

int GetWindowPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetWindowPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int GetWindowSize(lua_State* L)
{
    ImVec2 size = ImGui::GetWindowSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return  2;
}

int GetWindowWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowWidth());
    return  1;
}

int GetWindowHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowHeight());
    return  1;
}

// @MultiPain
int GetWindowBounds(lua_State* L)
{
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    ImVec2 pos = ImGui::GetWindowPos();
    vMin += pos;
    vMax += pos;

    //GidImGui* imgui = getImgui(L);
    float x1, y1, x2, y2;

    localToGlobal(imguiProxy, vMin.x, vMin.y, &x1, &y1);
    localToGlobal(imguiProxy, vMax.x, vMax.y, &x2, &y2);

    lua_pushnumber(L, x1);
    lua_pushnumber(L, y1);
    lua_pushnumber(L, x2);
    lua_pushnumber(L, y2);

    return 4;
}

int SetNextWindowPos(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    ImVec2 pivot = ImVec2(luaL_optnumber(L, 5, 0), luaL_optnumber(L, 6, 0));

    ImGui::SetNextWindowPos(pos, cond, pivot);

    return 0;
}

int SetNextWindowSize(lua_State* L)
{
    double w = luaL_checknumber(L, 2);
    double h = luaL_checknumber(L, 3);
    const ImVec2& size = ImVec2(w, h);
    ImGuiCond cond = luaL_optinteger(L, 4, 0);

    ImGui::SetNextWindowSize(size, cond);

    return 0;
}

static void NextWindowSizeConstraintCallback(ImGuiSizeCallbackData* data)
{
    lua_State* L = (lua_State*)data->UserData;

    luaL_checktype(L, 5, LUA_TFUNCTION);
    lua_pushvalue(L, 5);
    lua_pushnumber(L, data->Pos.x);
    lua_pushnumber(L, data->Pos.y);
    lua_pushnumber(L, data->CurrentSize.x);
    lua_pushnumber(L, data->CurrentSize.y);
    lua_pushnumber(L, data->DesiredSize.x);
    lua_pushnumber(L, data->DesiredSize.y);
    lua_call(L, 6, 2);
    data->DesiredSize = ImVec2(luaL_checknumber(L, -2), luaL_checknumber(L, -1));
    lua_pop(L, 1);
}

int SetNextWindowSizeConstraints(lua_State* L)
{
    ImVec2 size_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 size_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImGui::SetNextWindowSizeConstraints(size_min, size_max, NextWindowSizeConstraintCallback, (void *)L);
    return 0;
}

int SetNextWindowContentSize(lua_State* L)
{
    double w = luaL_checknumber(L, 2);
    double h = luaL_checknumber(L, 3);
    const ImVec2& size = ImVec2(w, h);

    ImGui::SetNextWindowContentSize(size);

    return 0;
}

int SetNextWindowCollapsed(lua_State* L)
{
    bool collapsed = lua_toboolean(L, 2) > 0;
    ImGuiCond cond = luaL_optinteger(L, 3, 0);

    ImGui::SetNextWindowCollapsed(collapsed, cond);
    return 0;
}

int SetNextWindowFocus(lua_State* _UNUSED(L))
{
    ImGui::SetNextWindowFocus();
    return 0;
}

int SetNextWindowBgAlpha(lua_State* L)
{
    double alpha = luaL_checknumber(L, 2);
    ImGui::SetNextWindowBgAlpha(alpha);
    return 0;
}

int SetWindowPos(lua_State* L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char* name = luaL_checkstring(L, 2);
        double x = luaL_checknumber(L, 3);
        double y = luaL_checknumber(L, 4);
        const ImVec2& pos = ImVec2(x, y);
        ImGuiCond cond = luaL_optinteger(L, 5, 0);

        ImGui::SetWindowPos(name, pos, cond);
    }
    else
    {
        double x = luaL_checknumber(L, 2);
        double y = luaL_checknumber(L, 3);
        const ImVec2& pos = ImVec2(x, y);
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowPos(pos, cond);
    }

    return  0;
}

int SetWindowSize(lua_State* L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char* name = luaL_checkstring(L, 2);
        double w = luaL_checknumber(L, 3);
        double h = luaL_checknumber(L, 4);
        const ImVec2& size = ImVec2(w, h);
        ImGuiCond cond = luaL_optinteger(L, 5, 0);

        ImGui::SetWindowSize(name, size, cond);
    }
    else
    {
        double w = luaL_checknumber(L, 2);
        double h = luaL_checknumber(L, 3);
        const ImVec2& size = ImVec2(w, h);
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowSize(size, cond);
    }

    return 0;
}

int SetWindowCollapsed(lua_State* L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char* name = luaL_checkstring(L, 2);
        bool collapsed = lua_toboolean(L, 3);
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowCollapsed(name, collapsed, cond);
    }
    else
    {
        bool collapsed = lua_toboolean(L, 2);
        ImGuiCond cond = luaL_optinteger(L, 3, 0);

        ImGui::SetWindowCollapsed(collapsed, cond);
    }

    return 0;
}

int SetWindowFocus(lua_State* L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char* name = luaL_checkstring(L, 2);
        ImGui::SetWindowFocus(name);
    }
    else
        ImGui::SetWindowFocus();

    return 0;
}

int SetWindowFontScale(lua_State* L)
{
    double scale = luaL_checknumber(L, 2);
    ImGui::SetWindowFontScale(scale);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Content region
/// Those functions are bound to be redesigned soon (they are confusing, incomplete and return values in local window coordinates which increases confusion)
///
/////////////////////////////////////////////////////////////////////////////////////////////
int GetContentRegionMax(lua_State* L)
{
    ImVec2 max = ImGui::GetContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int GetContentRegionAvail(lua_State* L)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    lua_pushnumber(L, avail.x);
    lua_pushnumber(L, avail.y);
    return 2;
}

int GetWindowContentRegionMin(lua_State* L)
{
    ImVec2 min = ImGui::GetWindowContentRegionMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int GetWindowContentRegionMax(lua_State* L)
{
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int GetWindowContentRegionWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowContentRegionWidth());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Windows Scrolling
///
/////////////////////////////////////////////////////////////////////////////////////////////
int GetScrollX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollX());
    return 1;
}

int GetScrollY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollY());
    return 1;
}

int GetScrollMaxX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxX());
    return 1;
}

int GetScrollMaxY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxY());
    return 1;
}

int SetScrollX(lua_State* L)
{
    double scroll_x = luaL_checknumber(L, 2);
    ImGui::SetScrollX(scroll_x);
    return 0;
}

int SetScrollY(lua_State* L)
{
    double scroll_y = luaL_checknumber(L, 2);
    ImGui::SetScrollY(scroll_y);
    return 0;
}

int SetScrollHereX(lua_State* L)
{
    double center_x_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereX(center_x_ratio);
    return 0;
}

int SetScrollHereY(lua_State* L)
{
    double center_y_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereY(center_y_ratio);
    return 0;
}

int SetScrollFromPosX(lua_State* L)
{
    double local_x = luaL_checknumber(L, 2);
    double center_x_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosX(local_x, center_x_ratio);
    return 0;
}

int SetScrollFromPosY(lua_State* L)
{
    double local_y = luaL_checknumber(L, 2);
    double center_y_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosY(local_y, center_y_ratio);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Parameters stacks (shared)
///
/////////////////////////////////////////////////////////////////////////////////////////////

int PushStyleColor(lua_State* L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);
    int hex = luaL_checkinteger(L, 3);
    float alpha = luaL_optnumber(L, 4, 1.0f);

    ImGui::PushStyleColor(idx, GColor::toU32(hex, alpha));

    return 0;
}

int PopStyleColor(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleColor(count);
    return 0;
}

int PushStyleVar(lua_State* L)
{
    ImGuiStyleVar idx = luaL_checkinteger(L, 2);

    if (lua_gettop(L) > 3)
    {
        double vx = luaL_checknumber(L, 3);
        double vy = luaL_checknumber(L, 4);
        ImGui::PushStyleVar(idx, ImVec2(vx, vy));
    }
    else
    {
        double val = luaL_checknumber(L, 3);
        ImGui::PushStyleVar(idx, val);
    }

    return 0;
}

int PopStyleVar(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleVar(count);
    return 0;
}

int GetFontSize(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFontSize());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Parameters stacks (current window)
///
/////////////////////////////////////////////////////////////////////////////////////////////

int PushItemWidth(lua_State* L)
{
    double item_width = luaL_checknumber(L, 2);
    ImGui::PushItemWidth(item_width);
    return 0;
}

int PopItemWidth(lua_State* _UNUSED(L))
{
    ImGui::PopItemWidth();
    return 0;
}

int PushItemFlag(lua_State* L)
{
    int option = luaL_checkinteger(L, 2);
    ImGui::PushItemFlag(option, lua_toboolean(L, 3));
    return 0;
}

int PopItemFlag(lua_State* _UNUSED(L))
{
    ImGui::PopItemFlag();
    return 0;
}

int SetNextItemWidth(lua_State* L)
{
    double item_width = luaL_checknumber(L, 2);
    ImGui::SetNextItemWidth(item_width);
    return 0;
}

int CalcItemWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::CalcItemWidth());
    return 1;
}

int PushTextWrapPos(lua_State* L)
{
    double wrap_local_pos_x = luaL_optnumber(L, 2, 0.0f);
    ImGui::PushTextWrapPos(wrap_local_pos_x);
    return 0;
}

int PopTextWrapPos(lua_State* _UNUSED(L))
{
    ImGui::PopTextWrapPos();
    return 0;
}

int PushAllowKeyboardFocus(lua_State* L)
{
    bool allow_keyboard_focus = lua_toboolean(L, 2);
    ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
    return 0;
}

int PopAllowKeyboardFocus(lua_State* _UNUSED(L))
{
    ImGui::PopAllowKeyboardFocus();
    return 0;
}

int PushButtonRepeat(lua_State* L)
{
    bool repeat = lua_toboolean(L, 2);
    ImGui::PushButtonRepeat(repeat);
    return 0;
}

int PopButtonRepeat(lua_State* _UNUSED(L))
{
    ImGui::PopButtonRepeat();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Cursor / Layout
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Separator(lua_State* _UNUSED(L))
{
    ImGui::Separator();
    return 0;
}

int SameLine(lua_State* L)
{
    double offset_from_start_x = luaL_optnumber(L, 2, 0.0f);
    double spacing = luaL_optnumber(L, 3, -1.0f);
    ImGui::SameLine(offset_from_start_x, spacing);
    return 0;
}

int NewLine(lua_State* _UNUSED(L))
{
    ImGui::NewLine();
    return 0;
}

int Spacing(lua_State* _UNUSED(L))
{
    ImGui::Spacing();
    return 0;
}

int Dummy(lua_State* L)
{
    double w = luaL_checknumber(L, 2);
    double h = luaL_checknumber(L, 3);

    ImGui::Dummy(ImVec2(w, h));
    return 0;
}

int Indent(lua_State* L)
{
    double indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Indent(indent_w);
    return 0;
}

int Unindent(lua_State* L)
{
    double indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Unindent(indent_w);
    return 0;
}

int BeginGroup(lua_State* _UNUSED(L))
{
    ImGui::BeginGroup();
    return 0;
}

int EndGroup(lua_State* _UNUSED(L))
{
    ImGui::EndGroup();
    return 0;
}

int GetCursorPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int GetCursorPosX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetCursorPosX());
    return 1;
}

int GetCursorPosY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetCursorPosY());
    return 1;
}

int SetCursorPos(lua_State* L)
{
    ImVec2 local_pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImGui::SetCursorPos(local_pos);
    return 0;
}

int SetCursorPosX(lua_State* L)
{
    double local_x = luaL_checknumber(L, 2);
    ImGui::SetCursorPosX(local_x);
    return 0;
}

int SetCursorPosY(lua_State* L)
{
    double local_y = luaL_checknumber(L, 2);
    ImGui::SetCursorPosY(local_y);
    return 0;
}

int GetCursorStartPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorStartPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int GetCursorScreenPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int SetCursorScreenPos(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImGui::SetCursorScreenPos(pos);
    return 0;
}

int AlignTextToFramePadding(lua_State* _UNUSED(L))
{
    ImGui::AlignTextToFramePadding();
    return 0;
}

int GetTextLineHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeight());
    return 1;
}

int GetTextLineHeightWithSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeightWithSpacing());
    return 1;
}

int GetFrameHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFrameHeight());
    return 1;
}

int GetFrameHeightWithSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFrameHeightWithSpacing());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ID
///
/////////////////////////////////////////////////////////////////////////////////////////////

int PushID(lua_State* L)
{
    if (lua_gettop(L) == 2)
    {
        const int arg_type = lua_type(L, 2);
        switch(arg_type)
        {
        case(LUA_TNIL):
            {
                LUA_THROW_ERROR("bad argument #2 to 'pushID' (string/number/table/function expected, got nil)");
            }
            break;
        case(LUA_TSTRING):
            ImGui::PushID(luaL_checkstring(L, 2));
            break;
        case(LUA_TNUMBER):
            ImGui::PushID(luaL_checknumber(L, 2));
            break;
        default:
            ImGui::PushID(lua_topointer(L, 2));
            break;
        }
    }
    else
    {
        LUA_THROW_ERROR("bar argument #2 to 'pushID'");
    }
    return 0;
}

int PopID(lua_State* _UNUSED(L))
{
    ImGui::PopID();
    return 0;
}

int GetID(lua_State* L)
{
    switch(lua_type(L, 2))
    {
        case LUA_TSTRING:
        {
            const char* str_id = luaL_checkstring(L, 2);
            ImGuiID id = ImGui::GetID(str_id);
            lua_pushnumber(L, (double)id);
        }
        default:
        {
            ImGuiID id = ImGui::GetID(lua_topointer(L, 2));
            lua_pushnumber(L, (double)id);
        }
    }

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Text
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Text(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    //ImGui::Text("%s", text);
    ImGui::TextUnformatted(text); // Must be faster
    return 0;
}

int TextColored(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextColored(GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)), "%s", text);
    return 0;
}

int TextDisabled(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextDisabled("%s", text);
    return 0;
}

int TextWrapped(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextWrapped("%s", text);
    return 0;
}

int LabelText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImGui::LabelText(label, "%s", text);
    return 0;
}

int BulletText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::BulletText("%s", text);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Main
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Button(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const ImVec2& size = ImVec2(luaL_optnumber(L, 3, 0.0f), luaL_optnumber(L, 4, 0.0f));
    lua_pushboolean(L, ImGui::Button(label, size));
    return 1;
}

int SmallButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    lua_pushboolean(L, ImGui::SmallButton(label));
    return 1;
}

int InvisibleButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    const ImVec2& size = ImVec2(luaL_optnumber(L, 3, 0.0f), luaL_optnumber(L, 4, 0.0f));
    lua_pushboolean(L, ImGui::InvisibleButton(str_id, size));
    return 1;
}

int ArrowButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiDir dir = luaL_checknumber(L, 3);
    lua_pushboolean(L, ImGui::ArrowButton(str_id, dir));
    return 1;
}

int Image(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));

    ImGui::Image(data.texture, size, data.uv0, data.uv1, tint, border);
    return 0;
}

int ImageFilled(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 0.0f));

    ImGui::ImageFilled(data.texture, size, data.uv0, data.uv1, bg_col, tint, border);
    return 0;
}

int ImageButton(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    int frame_padding = luaL_optinteger(L, 5, -1);
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 8, 0xffffff), luaL_optnumber(L, 9, 0.0f));

    lua_pushboolean(L, ImGui::ImageButton(data.texture, size, data.uv0, data.uv1, frame_padding, bg_col, tint));
    return 1;
}

int ImageButtonWithText(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    int frame_padding = luaL_optinteger(L, 6, -1);
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 tint_col = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 1.0f));

    lua_pushboolean(L, ImGui::ImageButtonWithText(data.texture, label, size, data.uv0, data.uv1, frame_padding, bg_col, tint_col));
    return 1;
}

int ScaledImage(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    const ImVec2& anchor = ImVec2(luaL_optnumber(L,  9, 0.5f), luaL_optnumber(L, 10, 0.5f));

    ImGui::ScaledImage(data.texture, size, data.texture_size, anchor, data.uv0, data.uv1, tint, border);
    return 0;
}

int ScaledImageFilled(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 0.0f));
    const ImVec2& anchor = ImVec2(luaL_optnumber(L, 11, 0.5f), luaL_optnumber(L, 12, 0.5f));

    ImGui::ScaledImageFilled(data.texture, size, data.texture_size, anchor, data.uv0, data.uv1, bg_col, tint, border);
    return 0;
}

int ScaledImageButton(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    int frame_padding = luaL_optinteger(L, 5, -1);
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 8, 0xffffff), luaL_optnumber(L, 9, 0.0f));
    const ImVec2& anchor = ImVec2(luaL_optnumber(L, 10, 0.5f), luaL_optnumber(L, 11, 0.5f));

    lua_pushboolean(L, ImGui::ScaledImageButton(data.texture, size, data.texture_size, anchor, data.uv0, data.uv1, frame_padding, bg_col, tint));
    return 1;
}

int ScaledImageButtonWithText(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    int frame_padding = luaL_optinteger(L, 6, -1);
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 tint_col = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 1.0f));
    const ImVec2& anchor = ImVec2(luaL_optnumber(L, 13, 0.5f), luaL_optnumber(L, 14, 0.5f));

    lua_pushboolean(L, ImGui::ScaledImageButtonWithText(data.texture, label, data.texture_size, anchor, size, data.uv0, data.uv1, frame_padding, bg_col, tint_col));
    return 1;
}

int Checkbox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool v = lua_toboolean2(L, 3) > 0;
    bool result = ImGui::Checkbox(label, &v);
    lua_pushboolean(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int CheckboxFlags(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    double flags = luaL_optnumber(L, 3, 0.0);
    double flags_value = luaL_optnumber(L, 4, 0.0);

    lua_pushboolean(L, ImGui::CheckboxFlags(label, (unsigned int*)&flags, (unsigned int)flags_value));
    lua_pushnumber(L, flags);
    return 2;
}

int RadioButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    if (lua_gettop(L) == 4)
    {
        int v = luaL_checkinteger(L, 3);
        int v_button = luaL_checkinteger(L, 4);
        lua_pushboolean(L, ImGui::RadioButton(label, &v, v_button));
        lua_pushinteger(L, v);
        return 2;
    }
    else
    {
        bool active = lua_toboolean2(L, 3) > 0;
        lua_pushboolean(L, ImGui::RadioButton(label, active));
        return 1;
    }
}

int ProgressBar(lua_State* L)
{
    double fraction = luaL_checknumber(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, -1.0f), luaL_optnumber(L, 4, 0.0f));
    const char* overlay = luaL_optstring(L, 5, "");
    ImGui::ProgressBar(fraction, size, overlay);
    return  0;
}

int Bullet(lua_State* _UNUSED(L))
{
    ImGui::Bullet();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Combo Box
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginCombo(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* preview_value = luaL_checkstring(L, 3);
    ImGuiComboFlags flags = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::BeginCombo(label, preview_value, flags));
    return 1;
}

int EndCombo(lua_State* _UNUSED(L))
{
    ImGui::EndCombo();
    return 0;
}

int Combo(lua_State* L)
{

    const char* label = luaL_checkstring(L, 2);
    int item_current = luaL_checkinteger(L, 3);
    if (item_current < 0)
    {
        lua_pushnumber(L, -1);
        lua_pushboolean(L, false);
        return 2;
    }

    int maxItems = luaL_optinteger(L, 5, -1);
    bool result = false;
    const int arg_type = lua_type(L, 4);

    switch (arg_type)
    {
        case LUA_TTABLE:
        {
            int len = luaL_getn(L, 4);
            if (!len)
            {
                lua_pushnumber(L, -1);
                return 1;
            }

            const char** items = new const char*[len];
            lua_pushvalue(L, 4);
            for (int i = 0; i < len; i++)
            {
                lua_rawgeti(L, 4, i + 1);
                const char* str = lua_tostring(L, -1);
                items[i] = str;
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            result = ImGui::Combo(label, &item_current, items, len, maxItems);

            delete[] items;
        } break;
        case LUA_TSTRING:
        {
            const char* items = luaL_checkstring(L, 4);

            result = ImGui::Combo(label, &item_current, items, maxItems);
        } break;
        default:
        {
            LUA_THROW_ERRORF("bad argument #3 to 'combo' (table/string expected, got %s)", lua_typename(L, 4));
            return 0;
        }
    }

    lua_pushinteger(L, item_current);
    lua_pushboolean(L, result);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Drags
///
/////////////////////////////////////////////////////////////////////////////////////////////

int DragFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    float v = luaL_checknumber(L, 3);
    float v_speed = luaL_optnumber(L, 4, 1.0f);
    float v_min = luaL_optnumber(L, 5, 0.0f);
    float v_max = luaL_optnumber(L, 6, 0.0f);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::DragFloat(label, &v, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);

    return 2;
}

int DragFloat2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec2f[4];
    vec2f[0] = luaL_checkinteger(L, 3);
    vec2f[1] = luaL_checkinteger(L, 4);
    float v_speed = luaL_optnumber(L, 5, 1.0f);
    float v_min = luaL_optnumber(L, 6, 0.0f);
    float v_max = luaL_optnumber(L, 7, 0.0f);
    const char* format = luaL_optstring(L, 8, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);
    bool result = ImGui::DragFloat2(label, vec2f, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 3;
}

int DragFloat3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec3f[4];
    vec3f[0] = luaL_checkinteger(L, 3);
    vec3f[1] = luaL_checkinteger(L, 4);
    vec3f[2] = luaL_checkinteger(L, 5);
    float v_speed = luaL_optnumber(L, 6, 1.0f);
    float v_min = luaL_optnumber(L, 7, 0.0f);
    float v_max = luaL_optnumber(L, 8, 0.0f);
    const char* format = luaL_optstring(L, 9, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);
    bool result = ImGui::DragFloat3(label, vec3f, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec3f[0]);
    lua_pushnumber(L, vec3f[1]);
    lua_pushnumber(L, vec3f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int DragFloat4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec4f[4];
    vec4f[0] = luaL_checkinteger(L, 3);
    vec4f[1] = luaL_checkinteger(L, 4);
    vec4f[2] = luaL_checkinteger(L, 5);
    vec4f[2] = luaL_checkinteger(L, 6);

    float v_speed = luaL_optnumber(L, 7, 1.0f);
    float v_min = luaL_optnumber(L, 8, 0.0f);
    float v_max = luaL_optnumber(L, 9, 0.0f);
    const char* format = luaL_optstring(L, 10, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, 0);
    bool result = ImGui::DragFloat4(label, vec4f, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec4f[0]);
    lua_pushnumber(L, vec4f[1]);
    lua_pushnumber(L, vec4f[2]);
    lua_pushnumber(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int DragFloatRange2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    float v_current_min = luaL_checknumber(L, 3);
    float v_current_max = luaL_checknumber(L, 4);
    float v_speed = luaL_optnumber(L, 5, 1.0f);
    float v_min = luaL_optnumber(L, 6, 0.0f);
    float v_max = luaL_optnumber(L, 7, 0.0f);
    const char* format = luaL_optstring(L, 8, "%.3f");
    const char* format_max = luaL_optstring(L, 9, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::DragFloatRange2(label, &v_current_min, &v_current_max, v_speed, v_min, v_max, format, format_max, sliderFlag);

    lua_pushnumber(L, v_current_min);
    lua_pushnumber(L, v_current_max);
    lua_pushboolean(L, result);
    return 3;
}

int DragInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    int v = luaL_checkinteger(L, 3);
    double v_speed = luaL_optnumber(L, 4, 1.0f);
    int v_min = luaL_optinteger(L, 5, 0);
    int v_max = luaL_optinteger(L, 6, 0);
    const char* format = luaL_optstring(L, 7, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::DragInt(label, &v, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int DragInt2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[2];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);

    double v_speed = luaL_optnumber(L, 5, 1.0f);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::DragInt(label, vec2i, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int DragInt3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 3);
    vec3i[1] = luaL_checkinteger(L, 4);
    vec3i[2] = luaL_checkinteger(L, 5);

    double v_speed = luaL_optnumber(L, 5, 1.0f);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::DragInt(label, vec3i, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int DragInt4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 3);
    vec4i[1] = luaL_checkinteger(L, 4);
    vec4i[2] = luaL_checkinteger(L, 5);
    vec4i[2] = luaL_checkinteger(L, 6);

    double v_speed = luaL_optnumber(L, 7, 1.0f);
    int v_min = luaL_optinteger(L, 8, 0);
    int v_max = luaL_optinteger(L, 9, 0);
    const char* format = luaL_optstring(L, 10, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, 0);

    bool result = ImGui::DragInt(label, vec4i, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int DragIntRange2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    int v_current_min = luaL_checkinteger(L, 3);
    int v_current_max = luaL_checkinteger(L, 4);
    double v_speed = luaL_optnumber(L, 5, 1.0f);
    int v_min = luaL_optinteger(L, 6, 0.0f);
    int v_max = luaL_optinteger(L, 7, 0.0f);
    const char* format = luaL_optstring(L, 8, "%d");
    const char* format_max = luaL_optstring(L, 9, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::DragIntRange2(label, &v_current_min, &v_current_max, v_speed, v_min, v_max, format, format_max, sliderFlag);

    lua_pushinteger(L, v_current_min);
    lua_pushinteger(L, v_current_max);
    lua_pushboolean(L, result);
    return 3;
}

int DragScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiDataType data_type = luaL_checkinteger(L, 3);
    double value = luaL_checknumber(L, 4);
    double v_speed = luaL_checknumber(L, 5);
    double v_min = luaL_optnumber(L, 6, NULL);
    double v_max = luaL_optnumber(L, 7, NULL);
    const char* format = luaL_optstring(L, 8, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::DragScalar(label, data_type, (void *)&value, v_speed, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Sliders
///
/////////////////////////////////////////////////////////////////////////////////////////////

int SliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    float v = luaL_checknumber(L, 3);
    float v_min = luaL_checknumber(L, 4);
    float v_max = luaL_checknumber(L, 5);
    const char* format = luaL_optstring(L, 6, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 7, 0);

    bool result = ImGui::SliderFloat(label, &v, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int SliderFloat2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec2f[3];
    vec2f[0] = luaL_checknumber(L, 3);
    vec2f[1] = luaL_checknumber(L, 4);
    float v_min = luaL_checknumber(L, 5);
    float v_max = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::SliderFloat2(label, vec2f, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 3;
}

int SliderFloat3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec3f[3];
    vec3f[0] = luaL_checknumber(L, 3);
    vec3f[1] = luaL_checknumber(L, 4);
    vec3f[2] = luaL_checknumber(L, 5);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::SliderFloat3(label, vec3f, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3f[0]);
    lua_pushinteger(L, vec3f[1]);
    lua_pushinteger(L, vec3f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int SliderFloat4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec4f[4];
    vec4f[0] = luaL_checknumber(L, 3);
    vec4f[1] = luaL_checknumber(L, 4);
    vec4f[2] = luaL_checknumber(L, 5);
    vec4f[3] = luaL_checknumber(L, 6);
    int v_min = luaL_optinteger(L, 7, 0);
    int v_max = luaL_optinteger(L, 8, 0);
    const char* format = luaL_optstring(L, 9, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::SliderFloat4(label, vec4f, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushinteger(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int SliderAngle(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    float v_rad = luaL_checknumber(L, 3);
    float v_degrees_min = luaL_optnumber(L, 4, -360.0f);
    float v_degrees_max = luaL_optnumber(L, 5,  360.0f);
    const char* format = luaL_optstring(L, 6, "%.0f deg");

    bool result = ImGui::SliderAngle(label, &v_rad, v_degrees_min, v_degrees_max, format);

    lua_pushnumber(L, v_rad);
    lua_pushboolean(L, result);
    return 2;
}

int SliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    int v = luaL_checkinteger(L, 3);
    int v_min = luaL_optinteger(L, 4, 0);
    int v_max = luaL_optinteger(L, 5, 0);
    const char* format = luaL_optstring(L, 6, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 7, 0);

    bool result = ImGui::SliderInt(label, &v, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int SliderInt2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[4];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);
    int v_min = luaL_optinteger(L, 5, 0);
    int v_max = luaL_optinteger(L, 6, 0);
    const char* format = luaL_optstring(L, 7, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::SliderInt2(label, vec2i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int SliderInt3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 3);
    vec3i[1] = luaL_checkinteger(L, 4);
    vec3i[2] = luaL_checkinteger(L, 5);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::SliderInt3(label, vec3i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int SliderInt4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 3);
    vec4i[1] = luaL_checkinteger(L, 4);
    vec4i[2] = luaL_checkinteger(L, 5);
    vec4i[3] = luaL_checkinteger(L, 6);

    int v_min = luaL_optinteger(L, 7, 0);
    int v_max = luaL_optinteger(L, 8, 0);
    const char* format = luaL_optstring(L, 9, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::SliderInt4(label, vec4i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int SliderScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiDataType data_type = luaL_checkinteger(L, 3);
    double value = luaL_checknumber(L, 4);
    double v_min = luaL_checknumber(L, 5);
    double v_max = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::SliderScalar(label, data_type, (void *)&value, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

int VSliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    float v = luaL_checknumber(L, 5);
    float v_min = luaL_checknumber(L, 6);
    float v_max = luaL_checknumber(L, 7);
    const char* format = luaL_optstring(L, 8, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::VSliderFloat(label, size, &v, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int VSliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    int v = luaL_checkinteger(L, 5);
    int v_min = luaL_checkinteger(L, 6);
    int v_max = luaL_checkinteger(L, 7);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::VSliderInt(label, size, &v, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int VSliderScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiDataType data_type = luaL_checkinteger(L, 5);
    double value = luaL_checknumber(L, 6);
    double v_min = luaL_optnumber(L, 7, NULL);
    double v_max = luaL_optnumber(L, 8, NULL);
    const char* format = luaL_optstring(L, 9, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::VSliderScalar(label, size, data_type, (void *)&value, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Custom filled sliders
///
/////////////////////////////////////////////////////////////////////////////////////////////

int FilledSliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    float v = luaL_checknumber(L, 4);
    float v_min = luaL_checknumber(L, 5);
    float v_max = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::FilledSliderFloat(label, mirror, &v, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int FilledSliderFloat2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static float vec2f[3];
    vec2f[0] = luaL_checknumber(L, 4);
    vec2f[1] = luaL_checknumber(L, 5);
    float v_min = luaL_checknumber(L, 6);
    float v_max = luaL_checknumber(L, 7);
    const char* format = luaL_optstring(L, 8, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::FilledSliderFloat(label, mirror, vec2f, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 2;
}

int FilledSliderFloat3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static float vec4f[3];
    vec4f[0] = luaL_checknumber(L, 4);
    vec4f[1] = luaL_checknumber(L, 5);
    vec4f[2] = luaL_checknumber(L, 6);
    int v_min = luaL_optinteger(L, 7, 0);
    int v_max = luaL_optinteger(L, 8, 0);
    const char* format = luaL_optstring(L, 9, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, NULL);

    bool result = ImGui::FilledSliderFloat3(label, mirror, vec4f, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int FilledSliderFloat4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static float vec4f[4];
    vec4f[0] = luaL_checknumber(L, 4);
    vec4f[1] = luaL_checknumber(L, 5);
    vec4f[2] = luaL_checknumber(L, 6);
    vec4f[3] = luaL_checknumber(L, 7);
    int v_min = luaL_optinteger(L, 8, 0);
    int v_max = luaL_optinteger(L, 9, 0);
    const char* format = luaL_optstring(L, 10, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, NULL);

    bool result = ImGui::FilledSliderFloat3(label, mirror, vec4f, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushinteger(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int FilledSliderAngle(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    float v_rad = luaL_checknumber(L, 4);
    float v_degrees_min = luaL_optnumber(L, 5, -360.0f);
    float v_degrees_max = luaL_optnumber(L, 6,  360.0f);
    const char* format = luaL_optstring(L, 7, "%.0f deg");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, NULL);

    bool result = ImGui::FilledSliderAngle(label, mirror, &v_rad, v_degrees_min, v_degrees_max, format, sliderFlag);

    lua_pushnumber(L, v_rad);
    lua_pushboolean(L, result);
    return 2;
}

int FilledSliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    int v = luaL_checkinteger(L, 4);
    int v_min = luaL_optinteger(L, 5, 0);
    int v_max = luaL_optinteger(L, 6, 0);
    const char* format = luaL_optstring(L, 7, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, NULL);

    bool result = ImGui::FilledSliderInt(label, mirror, &v, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int FilledSliderInt2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static int vec2i[4];
    vec2i[0] = luaL_checkinteger(L, 4);
    vec2i[1] = luaL_checkinteger(L, 5);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::FilledSliderInt2(label, mirror,  vec2i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int FilledSliderInt3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 4);
    vec3i[1] = luaL_checkinteger(L, 5);
    vec3i[2] = luaL_checkinteger(L, 6);
    int v_min = luaL_optinteger(L, 7, 0);
    int v_max = luaL_optinteger(L, 8, 0);
    const char* format = luaL_optstring(L, 9, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::FilledSliderInt3(label, mirror, vec3i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int FilledSliderInt4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 4);
    vec4i[1] = luaL_checkinteger(L, 5);
    vec4i[2] = luaL_checkinteger(L, 6);
    vec4i[3] = luaL_checkinteger(L, 7);

    int v_min = luaL_optinteger(L, 8, 0);
    int v_max = luaL_optinteger(L, 9, 0);
    const char* format = luaL_optstring(L, 10, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, 0);

    bool result = ImGui::FilledSliderInt4(label, mirror, vec4i, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int FilledSliderScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    ImGuiDataType data_type = luaL_checkinteger(L, 4);
    double value = luaL_checknumber(L, 5);
    double v_min = luaL_optnumber(L, 6, NULL);
    double v_max = luaL_optnumber(L, 7, NULL);
    const char* format = luaL_optstring(L, 8, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, NULL);

    bool result = ImGui::FilledSliderScalar(label, mirror, data_type, (void *)&value, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

int VFilledSliderFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    const ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    float v = luaL_checknumber(L, 6);
    float v_min = luaL_checknumber(L, 7);
    float v_max = luaL_checknumber(L, 8);
    const char* format = luaL_optstring(L, 9, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, 0);

    bool result = ImGui::VFilledSliderFloat(label, mirror, size, &v, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int VFilledSliderInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    const ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    int v = luaL_checkinteger(L, 6);
    int v_min = luaL_checkinteger(L, 7);
    int v_max = luaL_checkinteger(L, 8);
    const char* format = luaL_optstring(L, 9, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, NULL);

    bool result = ImGui::VFilledSliderInt(label, mirror, size, &v, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int VFilledSliderScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    const ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImGuiDataType data_type = luaL_checkinteger(L, 6);
    double value = luaL_checknumber(L, 7);
    double v_min = luaL_optnumber(L, 8, NULL);
    double v_max = luaL_optnumber(L, 9, NULL);
    const char* format = luaL_optstring(L, 10, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, NULL);

    bool result = ImGui::VFilledSliderScalar(label, mirror, size, data_type, (void *)&value, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

// TODO: callbacks?
/*
    static int TextInputCallback(ImGuiInputTextCallbackData* data)
    {
        //lua_State* L = (lua_State *)data->UserData;
        //lua_pushstring(L, data->Buf);
        //lua_error(L);
        return 0;
    }
    */

// Widgets: Input with Keyboard
int InputText(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    int buffer_size = luaL_checkinteger(L, 4);
    char* buffer = new char[buffer_size];
    sprintf(buffer, "%s", text);

    ImGuiInputTextFlags flags = luaL_optinteger(L, 5, 0);

    bool result = ImGui::InputText(label, buffer, buffer_size, flags);

    lua_pushstring(L, &(*buffer));
    lua_pushboolean(L, result);
    delete[] buffer;
    return 2;
}

int InputTextMultiline(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    int buffer_size = luaL_checkinteger(L, 4);

    char* buffer = new char[buffer_size];
    sprintf(buffer, "%s", text);

    ImVec2 size = ImVec2(luaL_optnumber(L, 5, 0.0f), luaL_optnumber(L, 6, 0.0f));
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputTextMultiline(label, buffer, buffer_size, size, flags);
    lua_pushstring(L, &(*buffer));
    lua_pushboolean(L, result);
    delete[] buffer;
    return 2;

}

int InputTextWithHint(lua_State* L)
{

    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    const char* hint = luaL_checkstring(L, 4);
    size_t buf_size = luaL_checkinteger(L, 5);
    char* buffer = new char[buf_size];
    sprintf(buffer, "%s", text);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputTextWithHint(label, hint, buffer, buf_size, flags);
    lua_pushstring(L, &(*buffer));
    lua_pushboolean(L, result);
    delete[] buffer;
    return 2;
}

int InputFloat(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    float value = luaL_checknumber(L, 3);
    float step = luaL_optnumber(L, 4, 0.0f);
    float step_fast = luaL_optnumber(L, 5, 0.0f);
    const char* format = luaL_optstring(L, 6, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputFloat(label, &value, step, step_fast, format, flags);
    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

int InputFloat2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec2f[2];
    vec2f[0] = luaL_checknumber(L, 3);
    vec2f[1] = luaL_checknumber(L, 4);
    const char* format = luaL_optstring(L, 5, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputFloat2(label, vec2f, format, flags);
    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 3;
}

int InputFloat3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec3f[3];
    vec3f[0] = luaL_checknumber(L, 3);
    vec3f[1] = luaL_checknumber(L, 4);
    vec3f[2] = luaL_checknumber(L, 5);
    const char* format = luaL_optstring(L, 6, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputFloat3(label, vec3f, format, flags);
    lua_pushnumber(L, vec3f[0]);
    lua_pushnumber(L, vec3f[1]);
    lua_pushnumber(L, vec3f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int InputFloat4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec4f[4];
    vec4f[0] = luaL_checknumber(L, 3);
    vec4f[1] = luaL_checknumber(L, 4);
    vec4f[2] = luaL_checknumber(L, 5);
    vec4f[3] = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 8, 0);

    bool result = ImGui::InputFloat4(label, vec4f, format, flags);
    lua_pushnumber(L, vec4f[0]);
    lua_pushnumber(L, vec4f[1]);
    lua_pushnumber(L, vec4f[2]);
    lua_pushnumber(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int InputInt(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);
    int step = luaL_optinteger(L, 4, 1);
    int step_fast = luaL_optinteger(L, 5, 100);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputInt(label, &value, step, step_fast, flags);
    lua_pushinteger(L, value);
    lua_pushboolean(L, result);
    return 2;
}

int InputInt2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[2];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 5, 0);

    bool result = ImGui::InputInt2(label, vec2i, flags);
    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int InputInt3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 3);
    vec3i[1] = luaL_checkinteger(L, 4);
    vec3i[2] = luaL_checkinteger(L, 5);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputInt3(label, vec3i, flags);
    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int InputInt4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 3);
    vec4i[1] = luaL_checkinteger(L, 4);
    vec4i[2] = luaL_checkinteger(L, 5);
    vec4i[3] = luaL_checkinteger(L, 6);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputInt4(label, vec4i, flags);
    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int InputDouble(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);
    double step = luaL_optnumber(L, 4, 0.0);
    double step_fast = luaL_optnumber(L, 5, 0.0);
    const char* format = luaL_optstring(L, 6, "%.6f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputDouble(label, &value, step, step_fast, format, flags);
    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

int InputScalar(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiDataType data_type = luaL_checkinteger(L, 3);
    double value = luaL_checknumber(L, 4);
    double v_min = luaL_checknumber(L, 5);
    double v_max = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::InputScalar(label, data_type, (void *)&value, (void *)&v_min, (void *)&v_max, format, sliderFlag);

    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little colored preview square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
///
/////////////////////////////////////////////////////////////////////////////////////////////

int ColorEdit3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::ColorEdit3(label, (float*)&col, flags);

    GColor conv = GColor::toHex(col);

    lua_pushnumber(L, conv.hex);
    lua_pushboolean(L, result);
    return 2;
}

int ColorEdit4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);

    bool result = ImGui::ColorEdit4(label, (float*)&col, flags);

    GColor conv = GColor::toHex(col);
    lua_pushnumber(L, conv.hex);
    lua_pushnumber(L, conv.alpha);
    lua_pushboolean(L, result);
    return 3;
}

int ColorPicker3(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::ColorPicker3(label, (float*)&col, flags);

    GColor conv = GColor::toHex(col);
    lua_pushnumber(L, conv.hex);
    lua_pushboolean(L, result);
    return 2;
}

int ColorPicker4(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);
    ImVec4 refCol = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 1.0f));

    bool result = ImGui::ColorPicker4(label, (float*)&col, flags, (float*)&refCol);

    GColor conv1 = GColor::toHex(col);
    GColor conv2 = GColor::toHex(refCol);
    lua_pushnumber(L, conv1.hex);
    lua_pushnumber(L, conv1.alpha);
    lua_pushnumber(L, conv2.hex);
    lua_pushnumber(L, conv2.alpha);
    lua_pushboolean(L, result);
    return 5;
}

int ColorButton(lua_State* L)
{
    const char* desc_id = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);
    ImVec2 size = ImVec2(luaL_optnumber(L, 6, 0), luaL_optnumber(L, 7, 0));

    bool result = ImGui::ColorButton(desc_id, col, flags, size);

    GColor conv = GColor::toHex(col);
    lua_pushnumber(L, conv.hex);
    lua_pushnumber(L, conv.alpha);
    lua_pushboolean(L, result);
    return 3;
}

int SetColorEditOptions(lua_State* L)
{
    ImGuiColorEditFlags flags = luaL_checkinteger(L, 2);
    ImGui::SetColorEditOptions(flags);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Trees
///
/////////////////////////////////////////////////////////////////////////////////////////////

int TreeNode(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool result;

    if (lua_type(L, 3) == LUA_TNIL)
        result = ImGui::TreeNode(label);
    else
        result = ImGui::TreeNode(label, "%s", luaL_checkstring(L, 3));

    lua_pushboolean(L, result);
    return 1;
}

int TreeNodeEx(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiTreeNodeFlags flags = luaL_checkinteger(L, 3);

    bool result;
    if (lua_type(L, 4) == LUA_TNIL)
        result = ImGui::TreeNodeEx(label, flags);
    else
        result = ImGui::TreeNodeEx(label, flags, "%s", luaL_checkstring(L, 4));

    lua_pushboolean(L, result);
    return 1;
}

int TreePush(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGui::TreePush(str_id);
    return 0;
}

int TreePop(lua_State* _UNUSED(L))
{
    ImGui::TreePop();
    return 0;
}

int GetTreeNodeToLabelSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTreeNodeToLabelSpacing());
    return 1;
}

int CollapsingHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool* p_open = getPopen(L, 3);
    ImGuiTreeNodeFlags flags = luaL_optinteger(L, 4, 0);

    bool flag = ImGui::CollapsingHeader(label, p_open, flags);

    int ret = 1;
    if (p_open != NULL)
    {
        lua_pushboolean(L, *p_open);
        delete p_open;
        ret++;
    }

    lua_pushboolean(L, flag);
    return ret;
}

int SetNextItemOpen(lua_State* L)
{
    bool is_open = lua_toboolean(L, 2);
    ImGuiCond cond = luaL_optinteger(L, 3, 0);
    ImGui::SetNextItemOpen(is_open, cond);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Selectables
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Selectable(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool selected = lua_toboolean(L, 3) > 0;
    ImGuiSelectableFlags flags = luaL_optinteger(L, 4, 0);
    ImVec2 size = ImVec2(luaL_optnumber(L, 5, 0.0f), luaL_optnumber(L, 6, 0.0f));

    bool result = ImGui::Selectable(label, &selected, flags, size);

    lua_pushboolean(L, result);
    lua_pushboolean(L, selected);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: List Boxes
///
/////////////////////////////////////////////////////////////////////////////////////////////

int ListBox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int current_item = luaL_checkinteger(L, 3);
    luaL_checktype(L, 4, LUA_TTABLE);
    int maxItems = luaL_optinteger(L, 5, -1);

    int len = luaL_getn(L, 4);
    const char** items = new const char*[len];
    lua_pushvalue(L, 4);
    for (int i = 0; i < len; i++)
    {
        lua_rawgeti(L, 4, i+1);

        const char* str = lua_tostring(L,-1);
        items[i] = str;
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    bool result = ImGui::ListBox(label, &current_item, items, len, maxItems);

    lua_pushinteger(L, current_item);
    lua_pushboolean(L, result);
    delete[] items;
    return 2;
}

int ListBoxHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    if (lua_gettop(L) > 3)
    {
        ImVec2 size = ImVec2(luaL_optnumber(L, 3, 0.0f), luaL_optnumber(L, 4, 0.0f));
        lua_pushboolean(L, ImGui::ListBoxHeader(label, size));
    }
    else
    {
        int items_count = luaL_checkinteger(L, 3);
        lua_pushboolean(L, ImGui::ListBoxHeader(label, items_count));
    }
    return 1;
}

int ListBoxFooter(lua_State* _UNUSED(L))
{
    ImGui::ListBoxFooter();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Data Plotting
///
/////////////////////////////////////////////////////////////////////////////////////////////

int PlotLines(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    luaL_checktype(L, 3, LUA_TTABLE);
    size_t len = luaL_getn(L, 3);
    float* values = new float[len];
    lua_pushvalue(L, 3);
    for (unsigned int i = 0; i < len; i++)
    {
        lua_rawgeti(L, 3, i+1);

        float v = luaL_checknumber(L, -1);
        values[i] = v;
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    int values_offset = luaL_optinteger(L, 4, 0);
    const char* overlay_text = luaL_optstring(L, 5, NULL);
    float scale_min = luaL_optnumber(L, 6, FLT_MAX);
    float scale_max = luaL_optnumber(L, 7, FLT_MAX);
    ImVec2 graph_size = ImVec2(luaL_optnumber(L, 8, 0), luaL_optnumber(L, 9, 0));
    int stride = sizeof(float);

    ImGui::PlotLines(label, values, len, values_offset, overlay_text, scale_min, scale_max, graph_size, stride);
    delete[] values;
    return 0;
}

int PlotHistogram(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    luaL_checktype(L, 3, LUA_TTABLE);
    int len = luaL_getn(L, 3);
    float* values = new float[len];
    lua_pushvalue(L, 3);
    for (int i = 0; i < len; i++)
    {
        lua_rawgeti(L, 3, i+1);

        float v = luaL_checknumber(L, -1);
        values[i] = v;
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    int values_offset = luaL_optinteger(L, 4, 0);
    const char* overlay_text = luaL_optstring(L, 5, NULL);
    float scale_min = luaL_optnumber(L, 6, FLT_MAX);
    float scale_max = luaL_optnumber(L, 7, FLT_MAX);
    ImVec2 graph_size = ImVec2(luaL_optnumber(L, 8, 0), luaL_optnumber(L, 9, 0));
    int stride = sizeof(float);

    ImGui::PlotHistogram(label, values, len, values_offset, overlay_text, scale_min, scale_max, graph_size, stride);
    delete[] values;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Value() Helpers.
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Value(lua_State* L)
{
    const char* prefix = luaL_checkstring(L, 2);
    const int valueType = lua_type(L, 3);
    switch(valueType)
    {
        case LUA_TBOOLEAN:
        {
            ImGui::Value(prefix, lua_toboolean(L, 3));
            break;
        }
        case LUA_TNUMBER:
        {
            if (lua_gettop(L) > 3)
            {
                float n = luaL_checknumber(L, 3);
                ImGui::Value(prefix, n, luaL_optstring(L, 4, ""));
            }
            else
            {
                int n = luaL_checkinteger(L, 3);
                ImGui::Value(prefix, n);
            }
            break;
        }
        default:
        {
            LUA_THROW_ERROR("Type mismatch. 'Number' or 'Boolean' expected.");
            break;
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Widgets: Menus
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginMenuBar(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginMenuBar());
    return 1;
}

int EndMenuBar(lua_State* _UNUSED(L))
{
    ImGui::EndMenuBar();
    return 0;
}

int BeginMainMenuBar(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginMainMenuBar());
    return 1;
}

int EndMainMenuBar(lua_State* _UNUSED(L))
{
    ImGui::EndMainMenuBar();
    return 0;
}

int BeginMenu(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool enabled = luaL_optboolean(L, 3, 1);
    lua_pushboolean(L, ImGui::BeginMenu(label, enabled));
    return 1;
}

int EndMenu(lua_State* _UNUSED(L))
{
    ImGui::EndMenu();
    return 0;
}

int MenuItem(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* shortcut = luaL_optstring(L, 3, NULL);
    int selected = luaL_optboolean(L, 4, 0);
    int enabled = luaL_optboolean(L, 5, 1);

    lua_pushboolean(L, ImGui::MenuItem(label, shortcut, selected, enabled));

    return 1;
}

int MenuItemWithShortcut(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* shortcut = luaL_checkstring(L, 3);
    bool p_selected = luaL_optboolean(L, 4, 0);
    bool enabled = luaL_optboolean(L, 5, 1);

    bool result = ImGui::MenuItem(label, shortcut, &p_selected, enabled);

    lua_pushboolean(L, p_selected);
    lua_pushboolean(L, result);

    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Tooltips
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginTooltip(lua_State* _UNUSED(L))
{
    ImGui::BeginTooltip();
    return 0;
}

int EndTooltip(lua_State* _UNUSED(L))
{
    ImGui::EndTooltip();
    return 0;
}

int SetTooltip(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::SetTooltip("%s", text);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Popups, Modals
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginPopup(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiWindowFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushboolean(L, ImGui::BeginPopup(str_id, flags));
    return 1;
}

int BeginPopupModal(lua_State* L)
{
    const char* name = luaL_checkstring(L, 2);
    bool* p_open = getPopen(L, 3);
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);
    bool draw_flag = ImGui::BeginPopupModal(name, p_open, flags);
    delete p_open;
    lua_pushboolean(L, draw_flag);
    return 1;
}

int EndPopup(lua_State* _UNUSED(L))
{
    ImGui::EndPopup();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Popups: open/close functions
///
/////////////////////////////////////////////////////////////////////////////////////////////

int OpenPopup(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 0);
    ImGui::OpenPopup(str_id, popup_flags);
    return 0;
}

int OpenPopupContextItem(lua_State* L) // renamed in 1.79 (backward capability)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);
    ImGui::OpenPopupOnItemClick(str_id, popup_flags);
    return 0;
}

int OpenPopupOnItemClick(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);
    ImGui::OpenPopupOnItemClick(str_id, popup_flags);
    return 0;
}

int CloseCurrentPopup(lua_State* _UNUSED(L))
{
    ImGui::CloseCurrentPopup();
    return 0;
}

// Popups: open+begin combined functions helpers
int BeginPopupContextItem(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextItem(str_id, popup_flags));
    return 1;
}

int BeginPopupContextWindow(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextWindow(str_id, popup_flags));
    return 1;
}

int BeginPopupContextVoid(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextVoid(str_id, popup_flags));
    return 1;
}

// Popups: test function
int IsPopupOpen(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::IsPopupOpen(str_id, popup_flags));
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Columns
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Columns(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    const char* id = luaL_optstring(L, 3, NULL);
    bool border = luaL_optboolean(L, 4, 1);

    ImGui::Columns(count, id, border);

    return 0;
}

int NextColumn(lua_State* _UNUSED(L))
{
    ImGui::NextColumn();
    return 0;
}

int GetColumnIndex(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetColumnIndex());
    return 1;
}

int GetColumnWidth(lua_State* L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnWidth(column_index));
    return 1;
}

int SetColumnWidth(lua_State* L)
{
    int column_index = luaL_checkinteger(L, 2);
    double width = luaL_checknumber(L, 3);
    ImGui::SetColumnWidth(column_index, width);
    return 0;
}

int GetColumnOffset(lua_State* L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnOffset(column_index));
    return 1;
}

int SetColumnOffset(lua_State* L)
{
    int column_index = luaL_checkinteger(L, 2);
    double offset = luaL_checknumber(L, 3);
    ImGui::SetColumnOffset(column_index, offset);
    return 0;
}

int GetColumnsCount(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetColumnsCount());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Tab Bars, Tabs
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginTabBar(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiTabBarFlags flags = luaL_optinteger(L, 3, 0);

    lua_pushboolean(L, ImGui::BeginTabBar(str_id, flags));
    return 1;
}

int EndTabBar(lua_State* _UNUSED(L))
{
    ImGui::EndTabBar();
    return  0;
}

int BeginTabItem(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool* p_open = getPopen(L, 3);
    ImGuiTabItemFlags flags = luaL_optinteger(L, 4, 0);

    bool flag = ImGui::BeginTabItem(label, p_open, flags);

    int ret = 1;
    if (p_open != NULL)
    {
        lua_pushboolean(L, *p_open);
        delete p_open;
        ret++;
    }
    lua_pushboolean(L, flag);
    return ret;
}

int EndTabItem(lua_State* _UNUSED(L))
{
    ImGui::EndTabItem();
    return 0;
}

int TabItemButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiTabItemFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushboolean(L, ImGui::TabItemButton(label, flags));
    return 1;
}

int SetTabItemClosed(lua_State* L)
{
    const char* tab_or_docked_window_label = luaL_checkstring(L, 2);
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
    return 0;
}

#ifdef IS_BETA_BUILD

/// TODO list:
/// windows api?

int DockSpace(lua_State* L)
{
    ImGuiID id = checkID(L);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 5, 0);
    ImGui::DockSpace(id, size, flags);
    return 0;
}

int DockSpaceOverViewport(lua_State* L)
{
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushinteger(L, ImGui::DockSpaceOverViewport(NULL, flags));
    return 1;
}

int SetNextWindowDockID(lua_State* L)
{
    ImGuiID dock_id = checkID(L);

    ImGuiCond cond = luaL_optinteger(L, 3, 0);
    ImGui::SetNextWindowDockID(dock_id, cond);
    return 0;
}

int GetWindowDockID(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetWindowDockID());
    return 1;
}

int IsWindowDocked(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowDocked());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// DockBuilder [BETA API]
///
/////////////////////////////////////////////////////////////////////////////////////////////

int DockBuilderDockWindow(lua_State* L)
{
    const char* window_name = luaL_checkstring(L, 2);
    ImGuiID node_id = checkID(L, 3);
    ImGui::DockBuilderDockWindow(window_name, node_id);
    return 0;
}

int DockBuilderCheckNode(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(node_id);
    lua_pushboolean(L, node != nullptr);
    return 1;
}

int DockBuilderSetNodePos(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImVec2 pos = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGui::DockBuilderSetNodePos((ImGuiID)node_id, pos);
    return 0;
}

int DockBuilderSetNodeSize(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGui::DockBuilderSetNodeSize((ImGuiID)node_id, size);
    return 0;
}

int DockBuilderAddNode(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushnumber(L, ImGui::DockBuilderAddNode(node_id, flags));
    return 1;
}

int DockBuilderRemoveNode(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImGui::DockBuilderRemoveNode(node_id);
    return 0;
}

int DockBuilderRemoveNodeChildNodes(lua_State* L)
{
    ImGuiID node_id = checkID(L, 2);
    ImGui::DockBuilderRemoveNodeChildNodes(node_id);
    return 0;
}

int DockBuilderRemoveNodeDockedWindows(lua_State* L)
{
    lua_Number node_id = checkID(L);
    bool clear_settings_refs = lua_toboolean(L, 3);
    ImGui::DockBuilderRemoveNodeDockedWindows(node_id, clear_settings_refs);
    return 0;
}

int DockBuilderSplitNode(lua_State* L)
{
    ImGuiID id = checkID(L);
    ImGuiDir split_dir = luaL_checkinteger(L, 3);
    float size_ratio_for_node_at_dir = luaL_checknumber(L, 4);
    ImGuiID* out_id_at_dir;
    ImGuiID* out_id_at_opposite_dir;

    if (lua_isnil(L, 5))
        out_id_at_dir = NULL;
    else
    {
        ImGuiID id = checkID(L, 5);
        out_id_at_dir = &id;
    }

    if (lua_isnil(L, 6))
        out_id_at_opposite_dir = NULL;
    else
    {
        ImGuiID id = checkID(L, 6);
        out_id_at_opposite_dir = &id;
    }

    lua_pushinteger(L, ImGui::DockBuilderSplitNode(id, split_dir, size_ratio_for_node_at_dir, out_id_at_dir, out_id_at_opposite_dir));
    if (out_id_at_dir == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, (lua_Number)(*out_id_at_dir));

    if (out_id_at_opposite_dir == nullptr)
        lua_pushnil(L);
    else
        lua_pushnumber(L, (lua_Number)(*out_id_at_opposite_dir));
    return 3;
}

// NOT TESTED
int DockBuilderCopyNode(lua_State* L)
{
    ImGuiID src_node_id = checkID(L);
    ImGuiID dst_node_id = checkID(L, 3);
    ImVector<ImGuiID>* out_node_remap_pairs;

    ImGui::DockBuilderCopyNode(src_node_id, dst_node_id, out_node_remap_pairs);

    int count = out_node_remap_pairs->Size;

    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++)
    {
        lua_pushnumber(L, (*out_node_remap_pairs)[i]);
        lua_setintfield(L, -2, i + 1);
    }
    return 1;
}

int DockBuilderCopyWindowSettings(lua_State* L)
{
    const char* src_name = luaL_checkstring(L, 2);
    const char* dst_name = luaL_checkstring(L, 3);
    ImGui::DockBuilderCopyWindowSettings(src_name, dst_name);
    return 0;
}

int DockBuilderCopyDockSpace(lua_State* L)
{
    ImGuiID src_dockspace_id = checkID(L);
    ImGuiID dst_dockspace_id = checkID(L, 3);
    ImVector<const char*>* in_window_remap_pairs;
    ImGui::DockBuilderCopyDockSpace(src_dockspace_id, dst_dockspace_id, in_window_remap_pairs);
    return 0;
}

int DockBuilderFinish(lua_State* L)
{
    ImGuiID node_id = checkID(L);
    ImGui::DockBuilderFinish(node_id);
    return 0;
}

ImGuiDockNode* getDockNode(lua_State* L, int index = 1)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", index));
    return node;
}

int DockBuilderGetNode(lua_State* L)
{
    ImGuiID node_id = checkID(L, 2);
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(node_id);
    if (node == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    Binder binder(L);
    binder.pushInstance("ImGuiDockNode", node);
    return 1;
}

int DockBuilder_Node_GetID(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->ID);
    return 1;
}

int DockBuilder_Node_GetSharedFlags(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->SharedFlags);
    return 1;
}

int DockBuilder_Node_GetLocalFlags(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->LocalFlags);
    return 1;
}

int DockBuilder_Node_GetParentNode(lua_State* L)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", 1));
    if (node == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    binder.pushInstance("ImGuiDockNode", node->ParentNode);
    return 1;
}

int DockBuilder_Node_GetChildNodes(lua_State* L)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", 1));
    if (node->ChildNodes[0] == nullptr)
        lua_pushnil(L);
    else
        binder.pushInstance("ImGuiDockNode", node->ChildNodes[0]);

    if (node->ChildNodes[1] == nullptr)
        lua_pushnil(L);
    else
        binder.pushInstance("ImGuiDockNode", node->ChildNodes[1]);
    return 2;
}

/*
int DockBuilder_Node_GetWindows(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->Windows);
    return 1;
}
*/

ImGuiTabBar* getTabBar(lua_State* L, int idx = 1)
{
    Binder binder(L);
    return static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", idx));
}

int DockBuilder_Node_GetTabBar(lua_State* L)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", 1));

    if (node->TabBar == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }

    binder.pushInstance("ImGuiTabBar", node->TabBar);
    return 1;
}

int DockBuilder_Node_GetPos(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->Pos.x);
    lua_pushnumber(L, node->Pos.y);
    return 2;
}

int DockBuilder_Node_GetSize(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->Size.x);
    lua_pushnumber(L, node->Size.y);
    return 2;
}

int DockBuilder_Node_GetSizeRef(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->SizeRef.x);
    lua_pushnumber(L, node->SizeRef.y);
    return 2;
}

int DockBuilder_Node_GetSplitAxis(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->SplitAxis);
    return 1;
}

/*
int DockBuilder_Node_GetWindowClass(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->WindowClass);
    return 1;
}
*/

int DockBuilder_Node_GetState(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->State);
    return 1;
}

/*
int DockBuilder_Node_GetHostWindow(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->HostWindow);
    return 1;
}

int DockBuilder_Node_GetVisibleWindow(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->VisibleWindow);
    return 1;
}
*/

int DockBuilder_Node_GetCentralNode(lua_State* L)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", 1));
    if (node == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    binder.pushInstance("ImGuiDockNode", node->CentralNode);
    return 1;
}

int DockBuilder_Node_GetOnlyNodeWithWindows(lua_State* L)
{
    Binder binder(L);
    ImGuiDockNode* node = static_cast<ImGuiDockNode*>(binder.getInstance("ImGuiDockNode", 1));
    if (node == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    binder.pushInstance("ImGuiDockNode", node->OnlyNodeWithWindows);
    return 1;
}

int DockBuilder_Node_GetLastFrameAlive(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->LastFrameAlive);
    return 1;
}

int DockBuilder_Node_GetLastFrameActive(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->LastFrameActive);
    return 1;
}

int DockBuilder_Node_GetLastFrameFocused(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->LastFrameFocused);
    return 1;
}

int DockBuilder_Node_GetLastFocusedNodeId(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->LastFocusedNodeId);
    return 1;
}

int DockBuilder_Node_GetSelectedTabId(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->SelectedTabId);
    return 1;
}

int DockBuilder_Node_WantCloseTabId(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->WantCloseTabId);
    return 1;
}

int DockBuilder_Node_GetAuthorityForPos(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->AuthorityForPos);
    return 1;
}

int DockBuilder_Node_GetAuthorityForSize(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->AuthorityForSize);
    return 1;
}

int DockBuilder_Node_GetAuthorityForViewport(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->AuthorityForViewport);
    return 1;
}

int DockBuilder_Node_IsVisible(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsVisible);
    return 1;
}

int DockBuilder_Node_IsFocused(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsFocused);
    return 1;
}

int DockBuilder_Node_HasCloseButton(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->HasCloseButton);
    return 1;
}

int DockBuilder_Node_HasWindowMenuButton(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->HasWindowMenuButton);
    return 1;
}

int DockBuilder_Node_EnableCloseButton(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    node->EnableCloseButton = lua_toboolean(L, 2);
    return 0;
}

int DockBuilder_Node_IsCloseButtonEnable(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->EnableCloseButton);
    return 1;
}

int DockBuilder_Node_WantCloseAll(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->WantCloseAll);
    return 1;
}

int DockBuilder_Node_WantLockSizeOnce(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->WantLockSizeOnce);
    return 1;
}

int DockBuilder_Node_WantMouseMove(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->WantMouseMove);
    return 1;
}

int DockBuilder_Node_WantHiddenTabBarUpdate(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->WantHiddenTabBarUpdate);
    return 1;
}

int DockBuilder_Node_WantHiddenTabBarToggle(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->WantHiddenTabBarToggle);
    return 1;
}

int DockBuilder_Node_MarkedForPosSizeWrite(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->MarkedForPosSizeWrite);
    return 1;
}

int DockBuilder_Node_IsRootNode(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsRootNode());
    return 1;
}

int DockBuilder_Node_IsDockSpace(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsDockSpace());
    return 1;
}

int DockBuilder_Node_IsFloatingNode(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsFloatingNode());
    return 1;
}

int DockBuilder_Node_IsCentralNode(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsCentralNode());
    return 1;
}

int DockBuilder_Node_IsHiddenTabBar(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsHiddenTabBar());
    return 1;
}

int DockBuilder_Node_IsNoTabBar(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsNoTabBar());
    return 1;
}

int DockBuilder_Node_IsSplitNode(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsSplitNode());
    return 1;
}

int DockBuilder_Node_IsLeafNode(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsLeafNode());
    return 1;
}

int DockBuilder_Node_IsEmpty(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushboolean(L, node->IsEmpty());
    return 1;
}

int DockBuilder_Node_GetMergedFlags(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    lua_pushnumber(L, node->GetMergedFlags());
    return 1;
}

int DockBuilder_Node_Rect(lua_State* L)
{
    ImGuiDockNode* node = getDockNode(L);
    ImRect rect = node->Rect();
    lua_pushnumber(L, rect.Min.x);
    lua_pushnumber(L, rect.Min.y);
    lua_pushnumber(L, rect.Max.x);
    lua_pushnumber(L, rect.Max.y);
    return 4;
}

/// TabItem +

ImGuiTabItem* getTabItem(lua_State* L, int idx = 1)
{
    Binder binder(L);
    return static_cast<ImGuiTabItem*>(binder.getInstance("ImGuiTabItem", idx));
}

int TabItem_GetID(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->ID);
    return 1;
}

int TabItem_GetFlags(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->Flags);
    return 1;
}

int TabItem_GetLastFrameVisible(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->LastFrameVisible);
    return 1;
}

int TabItem_GetLastFrameSelected(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->LastFrameSelected);
    return 1;
}

int TabItem_GetOffset(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->Offset);
    return 1;
}

int TabItem_GetWidth(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->Width);
    return 1;
}

int TabItem_GetContentWidth(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->ContentWidth);
    return 1;
}

int TabItem_GetNameOffset(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->NameOffset);
    return 1;
}

int TabItem_GetBeginOrder(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->BeginOrder);
    return 1;
}

int TabItem_GetIndexDuringLayout(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushnumber(L, tabItem->IndexDuringLayout);
    return 1;
}

int TabItem_WantClose(lua_State* L)
{
    ImGuiTabItem* tabItem = getTabItem(L);
    lua_pushboolean(L, tabItem->WantClose);
    return 1;
}

/// TabItem -

/// TabBar +
int TabBar_GetTabs(lua_State* L)
{
    Binder binder(L);
    ImGuiTabBar* tabBar = static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", 1));
    int count = tabBar->Tabs.Size;
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; i++)
    {
        binder.pushInstance("ImGuiTabItem", &tabBar->Tabs[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int TabBar_GetTab(lua_State* L)
{
    Binder binder(L);
    ImGuiTabBar* tabBar = static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", 1));
    int count = tabBar->Tabs.Size;
    int index = luaL_checkinteger(L, 2) - 1;
    LUA_ASSERT(index >= 0 && index <= count, "Tab index is out of bounds.");
    binder.pushInstance("ImGuiTabItem", &tabBar->Tabs[index]);
    return 1;
}

int TabBar_GetTabCount(lua_State* L)
{
    Binder binder(L);
    ImGuiTabBar* tabBar = static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", 1));
    int count = tabBar->Tabs.Size;
    lua_pushnumber(L, count);
    return 1;
}

int TabBar_GetFlags(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushinteger(L, tabBar->Flags);
    return 1;
}

int TabBar_GetID(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ID);
    return 1;
}

int TabBar_GetSelectedTabId(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->SelectedTabId);
    return 1;
}

int TabBar_GetNextSelectedTabId(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->NextSelectedTabId);
    return 0;
}

int TabBar_GetVisibleTabId(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->VisibleTabId);
    return 0;
}

int TabBar_GetCurrFrameVisible(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->CurrFrameVisible);
    return 0;
}

int TabBar_GetPrevFrameVisible(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->PrevFrameVisible);
    return 0;
}

int TabBar_GetBarRect(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->BarRect.Min.x);
    lua_pushnumber(L, tabBar->BarRect.Min.y);
    lua_pushnumber(L, tabBar->BarRect.Max.x);
    lua_pushnumber(L, tabBar->BarRect.Max.y);
    return 4;
}

int TabBar_GetCurrTabsContentsHeight(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->CurrTabsContentsHeight);
    return 1;
}

int TabBar_GetPrevTabsContentsHeight(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->PrevTabsContentsHeight);
    return 1;
}

int TabBar_GetWidthAllTabs(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->WidthAllTabs);
    return 1;
}

int TabBar_GetWidthAllTabsIdeal(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->WidthAllTabsIdeal);
    return 1;
}

int TabBar_GetScrollingAnim(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingAnim);
    return 1;
}

int TabBar_GetScrollingTarget(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingTarget);
    return 1;
}

int TabBar_GetScrollingTargetDistToVisibility(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingTargetDistToVisibility);
    return 1;
}

int TabBar_GetScrollingSpeed(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingSpeed);
    return 1;
}

int TabBar_GetScrollingRectMinX(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingRectMinX);
    return 1;
}

int TabBar_GetScrollingRectMaxX(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ScrollingRectMaxX);
    return 1;
}

int TabBar_GetReorderRequestTabId(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ReorderRequestTabId);
    return 1;
}

int TabBar_GetReorderRequestDir(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ReorderRequestDir);
    return 1;
}

int TabBar_GetBeginCount(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->BeginCount);
    return 1;
}

int TabBar_WantLayout(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushboolean(L, tabBar->WantLayout);
    return 1;
}

int TabBar_VisibleTabWasSubmitted(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushboolean(L, tabBar->VisibleTabWasSubmitted);
    return 1;
}

int TabBar_TabsAddedNew(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushboolean(L, tabBar->TabsAddedNew);
    return 1;
}

int TabBar_GetTabsActiveCount(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->TabsActiveCount);
    return 1;
}

int TabBar_GetLastTabItemIdx(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->LastTabItemIdx);
    return 1;
}

int TabBar_GetItemSpacingY(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->ItemSpacingY);
    return 1;
}

int TabBar_GetFramePadding(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->FramePadding.x);
    lua_pushnumber(L, tabBar->FramePadding.y);
    return 2;
}

int TabBar_GetBackupCursorPos(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushnumber(L, tabBar->BackupCursorPos.x);
    lua_pushnumber(L, tabBar->BackupCursorPos.y);
    return 2;
}

int TabBar_GetTabsNames(lua_State* L)
{
    ImGuiTabBar* tabBar = getTabBar(L);
    lua_pushstring(L, tabBar->TabsNames.c_str());
    return 1;
}

int TabBar_GetTabOrder(lua_State* L)
{
    Binder binder(L);
    LUA_ASSERT(binder.isInstanceOf("ImGuiTabBar", 1), "bad argument #1! ImGuiTabBar expected");
    LUA_ASSERT(binder.isInstanceOf("ImGuiTabItem", 2), "bad argument #2! ImGuiTabItem expected");

    ImGuiTabBar* tabBar = static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", 1));
    LUA_ASSERT(tabBar != nullptr, "TabBar is nil!");

    ImGuiTabItem* tab = static_cast<ImGuiTabItem*>(binder.getInstance("ImGuiTabItem", 2));
    LUA_ASSERT(tab != nullptr, "TabItem is nil!");

    lua_pushnumber(L, tabBar->GetTabOrder(tab));
    return 1;
}

int TabBar_GetTabName(lua_State* L)
{

    Binder binder(L);
    LUA_ASSERT(binder.isInstanceOf("ImGuiTabBar", 1), "bad argument #1! ImGuiTabBar expected");
    LUA_ASSERT(binder.isInstanceOf("ImGuiTabItem", 2), "bad argument #2! ImGuiTabItem expected");

    ImGuiTabBar* tabBar = static_cast<ImGuiTabBar*>(binder.getInstance("ImGuiTabBar", 1));
    LUA_ASSERT(tabBar != nullptr && tabBar != NULL, "TabBar is nil!");

    ImGuiTabItem* tab = static_cast<ImGuiTabItem*>(binder.getInstance("ImGuiTabItem", 2));
    LUA_ASSERT(tab != nullptr && tab != NULL, "TabItem is nil!");

    lua_pushstring(L, tabBar->GetTabName(tab));
    return 1;
}

/// TabBar -

#endif // IS_BETA_BUILD

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Logging/Capture
///
/////////////////////////////////////////////////////////////////////////////////////////////

int LogToTTY(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);
    ImGui::LogToTTY(auto_open_depth);
    return 0;
}

int LogToFile(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);

    ImGuiIO& io = ImGui::GetIO();
    LUA_ASSERT(io.LogFilename != NULL, "Log to file is disabled! Use ImGui:setLogFilename(filename) first.");

    if (lua_gettop(L) < 2 || lua_isnil(L, 3))
        ImGui::LogToFile(auto_open_depth, NULL);
    else
    {
        const char* filename = luaL_checkstring(L, 3);
        ImGui::LogToFile(auto_open_depth, filename);
    }

    return 0;
}

int LogToClipboard(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);
    ImGui::LogToClipboard(auto_open_depth);
    return 0;
}

int LogFinish(lua_State* _UNUSED(L))
{
    ImGui::LogFinish();
    return 0;
}

int LogButtons(lua_State* _UNUSED(L))
{
    ImGui::LogButtons();
    return 0;
}

int LogText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::LogText("%s", text);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Drag and Drop
///
/////////////////////////////////////////////////////////////////////////////////////////////

int BeginDragDropSource(lua_State* L)
{
    ImGuiDragDropFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::BeginDragDropSource(flags));
    return 1;
}

int SetNumberDragDropPayload(lua_State* L)
{
    const char* type = luaL_checkstring(L, 2);
    double v = luaL_checknumber(L, 3);
    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::SetDragDropPayload(type, (const void*)&v, sizeof(double), cond));
    return 1;
}

int SetStringDragDropPayload(lua_State* L)
{
    const char* type = luaL_checkstring(L, 2);
    const char* str = luaL_checkstring(L, 3);

    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::SetDragDropPayload(type, str, strlen(str), cond));
    return 1;
}

int EndDragDropSource(lua_State* _UNUSED(L))
{
    ImGui::EndDragDropSource();
    return 0;
}

int BeginDragDropTarget(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginDragDropTarget());
    return 1;
}

int AcceptDragDropPayload(lua_State* L)
{
    const char* type = luaL_checkstring(L, 2);
    ImGuiDragDropFlags flags = luaL_optinteger(L, 3, 0);

    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type, flags);
    if (payload == nullptr)
    {
        lua_pushnil(L);
    }
    else
    {
        Binder binder(L);
        binder.pushInstance("ImGuiPayload", const_cast<ImGuiPayload*>(reinterpret_cast<const ImGuiPayload*>(payload)));
    }
    return 1;
}

int EndDragDropTarget(lua_State* _UNUSED(L))
{
    ImGui::EndDragDropTarget();
    return 0;
}

int GetDragDropPayload(lua_State* L)
{
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (payload == nullptr)
    {
        lua_pushnil(L);
    }
    else
    {
        Binder binder(L);
        binder.pushInstance("ImGuiPayload", const_cast<ImGuiPayload*>(reinterpret_cast<const ImGuiPayload*>(payload)));
    }

    return 1;
}

ImGuiPayload* getPayload(lua_State* L)
{
    Binder binder(L);
    return static_cast<ImGuiPayload*>(binder.getInstance("ImGuiPayload", 1));
}

int Payload_GetNumberData(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    double* v = (double*)(payload->Data);
    lua_pushnumber(L, *v);
    return 1;
}

int Payload_GetStringData(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    const char* str = static_cast<const char*>(payload->Data);
    lua_pushlstring(L, str, payload->DataSize);
    return 1;
}

int Payload_Clear(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    payload->Clear();
    return 0;
}

int Payload_GetDataSize(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    lua_pushinteger(L, payload->DataSize);
    return 1;
}

int Payload_IsDataType(lua_State* L)
{
    const char* datatype = luaL_checkstring(L, 2);

    ImGuiPayload* payload = getPayload(L);
    lua_pushboolean(L, payload->IsDataType(datatype));
    return 1;
}

int Payload_IsPreview(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    lua_pushboolean(L, payload->IsPreview());
    return 1;
}

int Payload_IsDelivery(lua_State* L)
{
    ImGuiPayload* payload = getPayload(L);
    lua_pushboolean(L, payload->IsDelivery());
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Clipping
///
/////////////////////////////////////////////////////////////////////////////////////////////

int PushClipRect(lua_State* L)
{
    const ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    const ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = lua_toboolean(L, 6) > 0;
    ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int PopClipRect(lua_State* _UNUSED(L))
{
    ImGui::PopClipRect();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Focus, Activation
///
/////////////////////////////////////////////////////////////////////////////////////////////

int SetItemDefaultFocus(lua_State* _UNUSED(L))
{
    ImGui::SetItemDefaultFocus();
    return 0;
}

int SetKeyboardFocusHere(lua_State* L)
{
    int offset = luaL_optinteger(L, 2, 0);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Item/Widgets Utilities
///
/////////////////////////////////////////////////////////////////////////////////////////////

int IsItemHovered(lua_State* L)
{
    ImGuiHoveredFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsItemHovered(flags));
    return 1;
}

int IsItemActive(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemActive());
    return 1;
}

int IsItemFocused(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemFocused());
    return 1;
}

int IsItemClicked(lua_State* L)
{
    ImGuiMouseButton mouse_button = convertGiderosMouseButton(luaL_optinteger(L, 2, 1));
    lua_pushboolean(L, ImGui::IsItemClicked(mouse_button));
    return 1;
}

int IsItemVisible(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemVisible());
    return 1;
}

int IsItemEdited(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemEdited());
    return 1;
}

int IsItemActivated(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemActivated());
    return 1;
}

int IsItemDeactivated(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivated());
    return 1;
}

int IsItemDeactivatedAfterEdit(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivatedAfterEdit());
    return 1;
}

int IsItemToggledOpen(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemToggledOpen());
    return 1;
}

int IsAnyItemHovered(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemHovered());
    return 1;
}

int IsAnyItemActive(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemActive());
    return 1;
}

int IsAnyItemFocused(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemFocused());
    return 1;
}

int GetItemRectMin(lua_State* L)
{
    ImVec2 min = ImGui::GetItemRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int GetItemRectMax(lua_State* L)
{
    ImVec2 max = ImGui::GetItemRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int GetItemRectSize(lua_State* L)
{
    ImVec2 size = ImGui::GetItemRectSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int SetItemAllowOverlap(lua_State* _UNUSED(L))
{
    ImGui::SetItemAllowOverlap();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Miscellaneous Utilities
///
/////////////////////////////////////////////////////////////////////////////////////////////

int IsRectVisible(lua_State* L)
{
    ImVec2 size = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    if (lua_gettop(L) > 3)
    {
        ImVec2 rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
        lua_pushboolean(L, ImGui::IsRectVisible(size, rect_max));
    }
    else
    {
        lua_pushboolean(L, ImGui::IsRectVisible(size));
    }
    return 1;
}

int GetTime(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTime());
    return 1;
}

int GetFrameCount(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetFrameCount());
    return 1;
}

int GetStyleColorName(lua_State* L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);
    lua_pushstring(L, ImGui::GetStyleColorName(idx));
    return 1;
}

int GetStyleColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    GColor color = GColor::toHex(ImGui::GetStyleColorVec4(idx));
    lua_pushinteger(L, color.hex);
    lua_pushnumber(L, color.alpha);
    return 2;
}

int CalcListClipping(lua_State* L)
{
    int items_count = luaL_checkinteger(L, 2);
    float items_height = luaL_checknumber(L, 3);
    int out_items_display_start = luaL_checkinteger(L, 4);
    int out_items_display_end = luaL_checkinteger(L, 5);

    ImGui::CalcListClipping(items_count, items_height, &out_items_display_start, &out_items_display_end);
    lua_pushinteger(L, out_items_display_start);
    lua_pushinteger(L, out_items_display_end);
    return 2;
}

int BeginChildFrame(lua_State* L)
{

    ImGuiID id = checkID(L);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiWindowFlags flags = luaL_optinteger(L, 5, 0);

    lua_pushboolean(L, ImGui::BeginChildFrame(id, size, flags));
    return 1;
}

int EndChildFrame(lua_State* _UNUSED(L))
{
    ImGui::EndChildFrame();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Text Utilities
///
/////////////////////////////////////////////////////////////////////////////////////////////

int CalcTextSize(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    bool hide_text_after_double_hash = luaL_optboolean(L, 3, 0);
    float wrap_width = luaL_optnumber(L, 4, -1.0);

    ImVec2 size = ImGui::CalcTextSize(text, NULL, hide_text_after_double_hash, wrap_width);

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);

    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Inputs Utilities: Keyboard
///
/////////////////////////////////////////////////////////////////////////////////////////////

int GetKeyIndex(lua_State* L)
{
    ImGuiKey imgui_key = luaL_checkinteger(L, 2);
    lua_pushinteger(L, ImGui::GetKeyIndex(imgui_key));
    return  1;
}

int IsKeyDown(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyDown(user_key_index));
    return  1;
}

int IsKeyPressed(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    bool repeat = luaL_optboolean(L, 3, 1);

    lua_pushboolean(L, ImGui::IsKeyPressed(user_key_index, repeat));
    return 1;
}

int IsKeyReleased(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyReleased(user_key_index));
    return 1;
}

int GetKeyPressedAmount(lua_State* L)
{
    int key_index = luaL_checkinteger(L, 2);
    double repeat_delay = luaL_checknumber(L, 3);
    double rate = luaL_checknumber(L, 4);
    lua_pushinteger(L, ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate));
    return 1;
}

int CaptureKeyboardFromApp(lua_State* L)
{
    bool want_capture_keyboard_value = luaL_optboolean(L, 2, 1);
    ImGui::CaptureKeyboardFromApp(want_capture_keyboard_value);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Inputs Utilities: Mouse
///
/////////////////////////////////////////////////////////////////////////////////////////////

int IsMouseDown(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDown(button));
    return 1;
}

int IsMouseClicked(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    bool repeat = luaL_optboolean(L, 3, 0);
    lua_pushboolean(L, ImGui::IsMouseClicked(button, repeat));
    return 1;
}

int IsMouseReleased(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseReleased(button));
    return 1;
}

int IsMouseDoubleClicked(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDoubleClicked(button));
    return 1;
}

int IsMouseHoveringRect(lua_State* L)
{
    ImVec2 r_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 r_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool clip = luaL_optboolean(L, 6, 1);
    lua_pushboolean(L, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
    return 1;
}

int IsMousePosValid(lua_State* L)
{
    ImVec2 mouse_pos = ImVec2(luaL_optnumber(L, 2, -FLT_MAX), luaL_optnumber(L, 3, -FLT_MAX));
    lua_pushboolean(L, ImGui::IsMousePosValid(&mouse_pos));
    return 1;
}

int IsAnyMouseDown(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyMouseDown());
    return 1;
}

int GetMousePos(lua_State* L)
{
    ImVec2 pos = ImGui::GetMousePos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int GetMousePosOnOpeningCurrentPopup(lua_State* L)
{
    ImVec2 pos = ImGui::GetMousePosOnOpeningCurrentPopup();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int IsMouseDragging(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, -1.0f);
    lua_pushboolean(L, ImGui::IsMouseDragging(button, lock_threshold));
    return 1;
}

int GetMouseDragDelta(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, -1.0f);
    ImVec2 pos = ImGui::GetMouseDragDelta(button, lock_threshold);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ResetMouseDragDelta(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(lua_tointeger(L, 2));
    ImGui::ResetMouseDragDelta(button);
    return 0;
}

int GetMouseCursor(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetMouseCursor());
    return 1;
}

int SetMouseCursor(lua_State* L)
{
    ImGuiMouseCursor cursor_type = luaL_checkinteger(L, 2);
    ImGui::SetMouseCursor(cursor_type);
    return 0;
}

int CaptureMouseFromApp(lua_State* L)
{
    bool want_capture_mouse_value = luaL_optboolean(L, 2, 1) > 0;
    ImGui::CaptureMouseFromApp(want_capture_mouse_value);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// STYLES
///
/////////////////////////////////////////////////////////////////////////////////////////////

int StyleDark(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsDark();
    return 0;
}

int StyleLight(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsLight();
    return 0;
}

int StyleClassic(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsClassic();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Color Utilities
///
/////////////////////////////////////////////////////////////////////////////////////////////

int ColorConvertHEXtoRGB(lua_State* L)
{
    GColor color = GColor(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    ImVec4 vec = GColor::toVec4(color);

    lua_pushnumber(L, vec.x);
    lua_pushnumber(L, vec.y);
    lua_pushnumber(L, vec.z);
    lua_pushnumber(L, vec.w);
    return 4;
}

int ColorConvertRGBtoHEX(lua_State* L)
{
    float r = luaL_checknumber(L, 2);
    float g = luaL_checknumber(L, 3);
    float b = luaL_checknumber(L, 4);

    GColor color = GColor::toHex(r, g, b, 1.0f);

    lua_pushinteger(L, color.hex);
    return 1;
}

int ColorConvertRGBtoHSV(lua_State* L)
{
    float r = luaL_checknumber(L, 2);
    float g = luaL_checknumber(L, 3);
    float b = luaL_checknumber(L, 4);

    float h = 0;
    float s = 0;
    float v = 0;

    ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);

    lua_pushnumber(L, h);
    lua_pushnumber(L, s);
    lua_pushnumber(L, v);

    return 3;
}

int ColorConvertHSVtoRGB(lua_State* L)
{
    float h = luaL_checknumber(L, 2);
    float s = luaL_checknumber(L, 3);
    float v = luaL_checknumber(L, 4);

    float r = 0;
    float g = 0;
    float b = 0;

    ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);

    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);

    return 3;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// DEMOS
///
/////////////////////////////////////////////////////////////////////////////////////////////

int ShowUserGuide(lua_State* _UNUSED(L))
{
    ImGui::ShowUserGuide();

    return 0;
}

int ShowDemoWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowDemoWindow();

    return 0;
}

int ShowAboutWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowAboutWindow();

    return 0;
}

int ShowStyleEditor(lua_State* _UNUSED(L))
{
    ImGui::ShowStyleEditor();

    return 0;
}

int ShowFontSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    ImGui::ShowFontSelector(label);

    return 0;
}

int ShowMetricsWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowMetricsWindow();

    return 0;
}

int ShowStyleSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    bool open = ImGui::ShowStyleSelector(label);

    lua_pushboolean(L, open);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Style class
///
/////////////////////////////////////////////////////////////////////////////////////////////

int GetStyle(lua_State* L)
{
    Binder binder(L);

    ImGuiStyle* style = &ImGui::GetStyle();
    binder.pushInstance("ImGuiStyle", style);
    return 1;
}

ImGuiStyle& getStyle(lua_State* L)
{
    Binder binder(L);
    ImGuiStyle &style = *(static_cast<ImGuiStyle*>(binder.getInstance("ImGuiStyle", 1)));
    return style;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// AUTO GENERATED STYLE METHODS ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

int Style_old_SetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    LUA_ASSERT(idx >= 0 && idx <= ImGuiCol_COUNT, "Color index is out of bounds.");

    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[idx] = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    return 0;
}

int Style_SetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    LUA_ASSERT(idx >= 0 && idx <= ImGuiCol_COUNT, "Color index is out of bounds.");

    ImGuiStyle &style = getStyle(L);
    style.Colors[idx] = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    return 0;
}

int Style_GetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    LUA_ASSERT(idx >= 0 && idx <= ImGuiCol_COUNT, "Color index is out of bounds.");

    ImGuiStyle &style = getStyle(L);
    GColor color = GColor::toHex(style.Colors[idx]);
    lua_pushinteger(L, color.hex);
    lua_pushnumber(L, color.alpha);
    return 2;
}

int Style_SetAlpha(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.Alpha = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetAlpha(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.Alpha);
    return 1;
}

int Style_SetWindowRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetWindowRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowRounding);
    return 1;
}

int Style_SetWindowBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetWindowBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowBorderSize);
    return 1;
}

int Style_SetChildRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ChildRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetChildRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ChildRounding);
    return 1;
}

int Style_SetChildBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ChildBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetChildBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ChildBorderSize);
    return 1;
}

int Style_SetPopupRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.PopupRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetPopupRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.PopupRounding);
    return 1;
}

int Style_SetPopupBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.PopupBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetPopupBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.PopupBorderSize);
    return 1;
}

int Style_SetFrameRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FrameRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetFrameRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FrameRounding);
    return 1;
}

int Style_SetFrameBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FrameBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetFrameBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FrameBorderSize);
    return 1;
}

int Style_SetIndentSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.IndentSpacing = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetIndentSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.IndentSpacing);
    return 1;
}

int Style_SetColumnsMinSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ColumnsMinSpacing = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetColumnsMinSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ColumnsMinSpacing);
    return 1;
}

int Style_SetScrollbarSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ScrollbarSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetScrollbarSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ScrollbarSize);
    return 1;
}

int Style_SetScrollbarRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ScrollbarRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetScrollbarRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ScrollbarRounding);
    return 1;
}

int Style_SetGrabMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.GrabMinSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetGrabMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.GrabMinSize);
    return 1;
}

int Style_SetGrabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.GrabRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetGrabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.GrabRounding);
    return 1;
}

int Style_SetLogSliderDeadzone(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.LogSliderDeadzone = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetLogSliderDeadzone(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.LogSliderDeadzone);
    return 1;
}

int Style_SetTabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabRounding = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetTabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabRounding);
    return 1;
}

int Style_SetTabBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetTabBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabBorderSize);
    return 1;
}

int Style_SetTabMinWidthForUnselectedCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    //style.TabMinWidthForUnselectedCloseButton = luaL_checknumber(L, 2); // renamed in 1.79 (backward capability)
    style.TabMinWidthForCloseButton = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetTabMinWidthForUnselectedCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    //lua_pushnumber(L, style.TabMinWidthForUnselectedCloseButton);
    lua_pushnumber(L, style.TabMinWidthForCloseButton);  // renamed in 1.79 (backward capability)
    return 1;
}

int Style_SetTabMinWidthForCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabMinWidthForCloseButton = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetTabMinWidthForCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabMinWidthForCloseButton);
    return 1;
}

int Style_SetMouseCursorScale(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.MouseCursorScale = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetMouseCursorScale(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.MouseCursorScale);
    return 1;
}

int Style_SetCurveTessellationTol(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.CurveTessellationTol = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetCurveTessellationTol(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.CurveTessellationTol);
    return 1;
}

int Style_SetCircleSegmentMaxError(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.CircleSegmentMaxError = luaL_checknumber(L, 2);
    return 0;
}

int Style_GetCircleSegmentMaxError(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.CircleSegmentMaxError);
    return 1;
}

int Style_SetWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowPadding.x);
    lua_pushnumber(L, style.WindowPadding.y);
    return 2;
}

int Style_SetWindowMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowMinSize = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetWindowMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowMinSize.x);
    lua_pushnumber(L, style.WindowMinSize.y);
    return 2;
}

int Style_SetWindowTitleAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowTitleAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetWindowTitleAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowTitleAlign.x);
    lua_pushnumber(L, style.WindowTitleAlign.y);
    return 2;
}

int Style_SetFramePadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FramePadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetFramePadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FramePadding.x);
    lua_pushnumber(L, style.FramePadding.y);
    return 2;
}

int Style_SetItemSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ItemSpacing = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetItemSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ItemSpacing.x);
    lua_pushnumber(L, style.ItemSpacing.y);
    return 2;
}

int Style_SetItemInnerSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ItemInnerSpacing = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetItemInnerSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ItemInnerSpacing.x);
    lua_pushnumber(L, style.ItemInnerSpacing.y);
    return 2;
}

int Style_SetTouchExtraPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TouchExtraPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetTouchExtraPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TouchExtraPadding.x);
    lua_pushnumber(L, style.TouchExtraPadding.y);
    return 2;
}

int Style_SetButtonTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ButtonTextAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetButtonTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ButtonTextAlign.x);
    lua_pushnumber(L, style.ButtonTextAlign.y);
    return 2;
}

int Style_SetSelectableTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.SelectableTextAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetSelectableTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.SelectableTextAlign.x);
    lua_pushnumber(L, style.SelectableTextAlign.y);
    return 2;
}

int Style_SetDisplayWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.DisplayWindowPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetDisplayWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.DisplayWindowPadding.x);
    lua_pushnumber(L, style.DisplayWindowPadding.y);
    return 2;
}

int Style_SetDisplaySafeAreaPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.DisplaySafeAreaPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int Style_GetDisplaySafeAreaPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.DisplaySafeAreaPadding.x);
    lua_pushnumber(L, style.DisplaySafeAreaPadding.y);
    return 2;
}

int Style_SetWindowMenuButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowMenuButtonPosition = luaL_checkinteger(L, 2);
    return 0;
}

int Style_GetWindowMenuButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushinteger(L, style.WindowMenuButtonPosition);
    return 1;
}

int Style_SetColorButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ColorButtonPosition = luaL_checkinteger(L, 2);
    return 0;
}

int Style_GetColorButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushinteger(L, style.ColorButtonPosition);
    return 1;
}

int Style_SetAntiAliasedLines(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedLines = lua_toboolean(L, 2) > 0;
    return 0;
}

int Style_GetAntiAliasedLines(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedLines);
    return 1;
}

int Style_SetAntiAliasedLinesUseTex(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedLinesUseTex = lua_toboolean(L, 2) > 0;
    return 0;
}

int Style_GetAntiAliasedLinesUseTex(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedLinesUseTex);
    return 1;
}

int Style_SetAntiAliasedFill(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedFill = lua_toboolean(L, 2) > 0;
    return 0;
}

int Style_GetAntiAliasedFill(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedFill);
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

ImFontAtlas* getFontAtlas(lua_State* L, int index = 1)
{
    Binder binder(L);
    return static_cast<ImFontAtlas*>(binder.getInstance("ImFontAtlas", index));
}

ImFont* getFont(lua_State* L, int index = 1)
{
    Binder binder(L);
    ImFont* font = static_cast<ImFont*>(binder.getInstance("ImFont", index));
    LUA_ASSERT(font, "Font is nil!");
    return font;
}

ImGuiIO& getIO(lua_State* L, int index = 1)
{
    Binder binder(L);
    ImGuiIO &io = *(static_cast<ImGuiIO*>(binder.getInstance("ImGuiIO", index)));
    return io;
}

int GetIO(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance("ImGuiIO", &ImGui::GetIO());
    return 1;
}

#ifdef IS_BETA_BUILD
int IO_GetConfigDockingNoSplit(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingNoSplit);
    return 0;
}

int IO_SetConfigDockingNoSplit(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingNoSplit = lua_toboolean(L, 2) > 0;
    return 0;
}

int IO_GetConfigDockingWithShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingWithShift);
    return 0;
}

int IO_SetConfigDockingWithShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingWithShift = lua_toboolean(L, 2) > 0;
    return 0;
}

int IO_GetConfigDockingAlwaysTabBar(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingAlwaysTabBar);
    return 0;
}

int IO_SetConfigDockingAlwaysTabBar(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingAlwaysTabBar = lua_toboolean(L, 2) > 0;
    return 0;
}

int IO_GetConfigDockingTransparentPayload(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingTransparentPayload);
    return 0;
}

int IO_SetConfigDockingTransparentPayload(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingTransparentPayload = lua_toboolean(L, 2) > 0;
    return 0;
}
#endif

int IO_SetFontDefault(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    ImFont* font = getFont(L, 2);
    if (font)
        io.FontDefault = font;
    return 0;
}

int IO_GetFonts(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    Binder binder(L);
    binder.pushInstance("ImFontAtlas", io.Fonts);
    return 1;
}

int IO_GetDeltaTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DeltaTime);
    return 1;
}

int IO_GetMouseWheel(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseWheel);
    return 1;
}

int IO_GetMouseWheelH(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseWheelH);
    return 1;
}

int IO_isMouseDown(lua_State* L)
{
    int button = convertGiderosMouseButton(luaL_checkinteger(L, 2));
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.MouseDown[button]);
    return  1;
}

int IO_isKeyCtrl(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyCtrl);
    return 1;
}

int IO_isKeyShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyShift);
    return 1;
}

int IO_isKeyAlt(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyAlt);
    return 1;
}

int IO_isKeySuper(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeySuper);
    return 1;
}

int IO_GetKeysDown(lua_State* L)
{
    int index = luaL_checkinteger(L, 2);
    LUA_ASSERT(index >= 0 && index <= 512, "KeyDown index is out of bounds!");
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeysDown[index]);
    return 1;
}

int IO_WantCaptureMouse(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantCaptureMouse);
    return 1;
}

int IO_WantCaptureKeyboard(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantCaptureKeyboard);
    return 1;
}

int IO_WantTextInput(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantTextInput);
    return 1;
}

int IO_WantSetMousePos(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantSetMousePos);
    return 1;
}

int IO_WantSaveIniSettings(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantSaveIniSettings);
    return 1;
}

int getNavButtonIndex(lua_State* L, int idx = 2)
{
    int index = luaL_checkinteger(L, idx);
    LUA_ASSERTF(index >= 0 && index <= ImGuiNavInput_COUNT - 5, "Nav input index is out of bounds! Must be [%d; %d]", 0, ImGuiNavInput_COUNT - 5);
    return index;
}

int IO_SetNavInput(lua_State* L)
{
    int index = getNavButtonIndex(L);
    float value = luaL_checknumber(L, 3);
    ImGuiIO& io = getIO(L);
    io.NavInputs[index] = value;
    return 0;
}

int IO_GetNavInput(lua_State* L)
{
    int index = getNavButtonIndex(L);

    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.NavInputs[index]);
    return 1;
}

int IO_IsNavActive(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.NavActive);
    return 1;
}

int IO_IsNavVisible(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.NavVisible);
    return 1;
}

int IO_SetNavInputsDownDuration(lua_State *L)
{
    ImGuiIO& io = getIO(L);
    int index = getNavButtonIndex(L);
    io.NavInputsDownDuration[index] = luaL_checknumber(L, 2);
    return 0;
}

int IO_GetNavInputsDownDuration(lua_State *L)
{
    ImGuiIO& io = getIO(L);
    int index = getNavButtonIndex(L);
    lua_pushboolean(L, io.NavInputsDownDuration[index]);
    return 1;
}

int IO_SetNavInputsDownDurationPrev(lua_State *L)
{
    ImGuiIO& io = getIO(L);
    int index = getNavButtonIndex(L);
    io.NavInputsDownDurationPrev[index] = luaL_checknumber(L, 2);
    return 0;
}

int IO_GetNavInputsDownDurationPrev(lua_State *L)
{
    ImGuiIO& io = getIO(L);
    int index = getNavButtonIndex(L);
    //io.NavActive
    //ImGuiKey_Nav
    lua_pushboolean(L, io.NavInputsDownDurationPrev[index]);
    return 1;
}

int IO_GetFramerate(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushnumber(L, io.Framerate);
    return 1;
}

int IO_GetMetricsRenderVertices(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderVertices);
    return 1;
}

int IO_GetMetricsRenderIndices(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderIndices);
    return 1;
}

int IO_GetMetricsRenderWindows(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderWindows);
    return 1;
}

int IO_GetMetricsActiveWindows(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsActiveWindows);
    return 1;
}

int IO_GetMetricsActiveAllocations(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsActiveAllocations);
    return 1;
}

int IO_GetMouseDelta(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDelta.x);
    lua_pushnumber(L, io.MouseDelta.y);
    return 2;
}

int IO_GetMouseDownSec(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int button = convertGiderosMouseButton(lua_tointeger(L, 2));

    lua_pushnumber(L, io.MouseDownDuration[button]);
    return 1;
}

int IO_SetDisplaySize(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.DisplaySize.x = luaL_checknumber(L, 2);
    io.DisplaySize.y = luaL_checknumber(L, 3);

    return 0;
}

int IO_GetDisplaySize(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DisplaySize.x);
    lua_pushnumber(L, io.DisplaySize.y);

    return 2;
}


int IO_GetConfigFlags(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushinteger(L, io.ConfigFlags);
    return 1;
}

int IO_SetConfigFlags(lua_State* L)
{
    ImGuiConfigFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.ConfigFlags = flags;
    return 0;
}

int IO_AddConfigFlags(lua_State* L)
{
    ImGuiConfigFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.ConfigFlags |= flags;
    return 0;
}

int IO_GetBackendFlags(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushinteger(L, io.BackendFlags);
    return 1;
}

int IO_SetBackendFlags(lua_State* L)
{
    ImGuiBackendFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.BackendFlags = flags;
    return 0;
}

int IO_AddBackendFlags(lua_State* L)
{
    ImGuiBackendFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.BackendFlags |= flags;
    return 0;
}

int IO_GetIniSavingRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.IniSavingRate);
    return 1;
}

int IO_SetIniSavingRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.IniSavingRate = luaL_optnumber(L, 2, 5.0f);
    return 1;
}

int IO_GetIniFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.IniFilename);
    return 1;
}

int IO_SetIniFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    if (lua_gettop(L) == 2 && lua_isnil(L, 2))
        io.IniFilename = NULL;
    else
        io.IniFilename = luaL_checkstring(L, 2);
    return 0;
}

int IO_GetLogFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.LogFilename);
    return 1;
}

int IO_SetLogFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    if (lua_gettop(L) == 2 && lua_isnil(L, 2))
        io.LogFilename = NULL;
    else
        io.LogFilename = luaL_checkstring(L, 2);
    return 0;
}

int IO_GetMouseDoubleClickTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDoubleClickTime);
    return 1;
}

int IO_SetMouseDoubleClickTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDoubleClickTime = luaL_optnumber(L, 2, 0.30f);
    return 0;
}

int IO_GetMouseDragThreshold(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDragThreshold);
    return 1;
}

int IO_SetMouseDragThreshold(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDragThreshold = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int IO_GetMouseDrawCursor(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.MouseDrawCursor);
    return 1;
}

int IO_SetMouseDrawCursor(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDrawCursor = lua_toboolean(L, 2) > 0;
    if (io.MouseDrawCursor)
    {
        bool hideSystemCursor = luaL_optboolean(L, 3, 1);
        if (hideSystemCursor)
            setApplicationCursor(L, "blank");
    }
    else
    {
        setApplicationCursor(L, "arrow");
    }
    return 0;
}

int IO_GetMouseDoubleClickMaxDist(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDoubleClickMaxDist);
    return 1;
}

int IO_SetMouseDoubleClickMaxDist(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDoubleClickMaxDist = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int IO_GetKeyMapValue(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int index = luaL_checkinteger(L, 2);
    LUA_ASSERT(index >= 0 && index <= ImGuiKey_COUNT, "KeyMap index is out of bounds!");
    lua_pushinteger(L, io.KeyMap[index]);
    return 1;
}

int IO_SetKeyMapValue(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int index = luaL_checkinteger(L, 2);
    LUA_ASSERT(index >= 0 && index <= ImGuiKey_COUNT, "KeyMap index is out of bounds!");

    io.KeyMap[index] = luaL_checkinteger(L, 3);
    return 0;
}

int IO_GetKeyRepeatDelay(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.KeyRepeatDelay);
    return 1;
}

int IO_SetKeyRepeatDelay(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.KeyRepeatDelay = luaL_optnumber(L, 2, 0.25f);
    return 0;
}

int IO_GetKeyRepeatRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.KeyRepeatRate);
    return 1;
}

int IO_SetKeyRepeatRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.KeyRepeatRate = luaL_optnumber(L, 2, 0.05f);
    return 0;
}

int IO_GetFontGlobalScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.FontGlobalScale);
    return 1;
}

int IO_SetFontGlobalScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.FontGlobalScale = luaL_optnumber(L, 2, 1.0f);
    return 0;
}

int IO_GetFontAllowUserScaling(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.FontAllowUserScaling);
    return 1;
}

int IO_SetFontAllowUserScaling(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.FontAllowUserScaling = lua_toboolean(L, 2) > 0;
    return 0;
}

int IO_GetDisplayFramebufferScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DisplayFramebufferScale.x);
    lua_pushnumber(L, io.DisplayFramebufferScale.y);
    return 2;
}

int IO_SetDisplayFramebufferScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    ImVec2 scale = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    io.DisplayFramebufferScale = scale;
    return 0;
}

int IO_GetConfigMacOSXBehaviors(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigMacOSXBehaviors);
    return 1;
}

int IO_SetConfigMacOSXBehaviors(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigMacOSXBehaviors = flag;
    return 0;
}

int IO_GetConfigInputTextCursorBlink(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int IO_SetConfigInputTextCursorBlink(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigInputTextCursorBlink = flag;
    return 0;
}

int IO_GetConfigWindowsResizeFromEdges(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int IO_SetConfigWindowsResizeFromEdges(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsResizeFromEdges = flag;
    return 0;
}

int IO_GetConfigWindowsMoveFromTitleBarOnly(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigWindowsMoveFromTitleBarOnly);
    return 1;
}

int IO_SetConfigWindowsMoveFromTitleBarOnly(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsMoveFromTitleBarOnly = flag;
    return 0;
}

int IO_GetConfigWindowsMemoryCompactTimer(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.ConfigWindowsMemoryCompactTimer);
    return 1;
}

int IO_SetConfigWindowsMemoryCompactTimer(lua_State* L)
{
    double t = luaL_optnumber(L, 2, -1.0f);

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsMemoryCompactTimer = t;
    return 0;
}

int IO_GetBackendPlatformName(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.BackendPlatformName);
    return 1;
}

int IO_GetBackendRendererName(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.BackendRendererName);
    return 1;
}

int IO_SetMouseDown(lua_State* L)
{
    int buttonIndex = luaL_checkinteger(L, 2);
    LUA_ASSERTF(buttonIndex >= 0 && buttonIndex <= ImGuiMouseButton_COUNT,
                "Button index is out of bounds. Must be: [0..%d], but was: %d", ImGuiMouseButton_COUNT, buttonIndex);
    bool state = lua_toboolean(L, 3);
    ImGuiIO& io = getIO(L);
    io.MouseDown[buttonIndex] = state;
}

int IO_SetMousePos(lua_State* L)
{
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    ImGuiIO& io = getIO(L);
    io.MousePos = EventListener::translateMousePos(x, y);
}

int IO_SetMouseWheel(lua_State* L)
{
    float wheel = luaL_checknumber(L, 2);
    ImGuiIO& io = getIO(L);
    io.MouseWheel = wheel;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

struct FontData
{
    void* data;
    size_t size;

    FontData(void* p_data, size_t p_size) :data(p_data),size(p_size) {}
};

FontData getFontData(lua_State* _UNUSED(L), const char* filename)
{
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);

    LUA_ASSERTF(data != nullptr, "Cant load '%s' font! File not found.", filename);

    return FontData(data, data_size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// FONTS API
///
/////////////////////////////////////////////////////////////////////////////////////////////

int Fonts_PushFont(lua_State* L)
{
    Binder binder(L);
    ImFont* font = static_cast<ImFont*>(binder.getInstance("ImFont", 2));
    LUA_ASSERT(font, "Font is nil");
    ImGui::PushFont(font);
    return 0;
}

int Fonts_PopFont(lua_State* _UNUSED(L))
{
    ImGui::PopFont();
    return 0;
}

const ImWchar* getRanges(ImFontAtlas* atlas, const int ranges)
{
    switch(ranges)
    {
    case ImGuiGlyphRanges_Default:
        return atlas->GetGlyphRangesDefault();
    case ImGuiGlyphRanges_Korean:
        return atlas->GetGlyphRangesKorean();
    case ImGuiGlyphRanges_ChineseFull:
        return atlas->GetGlyphRangesChineseFull();
    case ImGuiGlyphRanges_ChineseSimplifiedCommon:
        return atlas->GetGlyphRangesChineseSimplifiedCommon();
    case ImGuiGlyphRanges_Japanese:
        return atlas->GetGlyphRangesJapanese();
    case ImGuiGlyphRanges_Cyrillic:
        return atlas->GetGlyphRangesCyrillic();
    case ImGuiGlyphRanges_Thai:
        return atlas->GetGlyphRangesThai();
    case ImGuiGlyphRanges_Vietnamese:
        return atlas->GetGlyphRangesVietnamese();
    }
}

typedef void (*GidConfCallback)(ImFontGlyphRangesBuilder&, ImFontAtlas*, int);

void readConfTable(lua_State* L, const char* name, ImFontGlyphRangesBuilder &builder, ImFontAtlas* atlas, GidConfCallback f)
{
    lua_getfield(L, -1, name);
    luaL_checktype(L, 1, LUA_TTABLE);
    int len = luaL_getn(L, -1);

    if (!lua_isnil(L, -1) && len > 0)
    {
        for (int i = 0; i < len; i++)
        {
            lua_rawgeti(L, -1, i + 1);
            f(builder, atlas, luaL_checkinteger(L, -1));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

void addConfChars(ImFontGlyphRangesBuilder &builder, ImFontAtlas* atlas, int value)
{
    builder.AddChar(value);
}

void addConfRanges(ImFontGlyphRangesBuilder &builder, ImFontAtlas* atlas, int value)
{
    builder.AddRanges(getRanges(atlas, value));
}

// TODO
/*
    void addConfCustomRanges(ImFontGlyphRangesBuilder &builder, ImFontAtlas* atlas, int value)
    {

    }
    */
void loadFontConfig(lua_State* L, int index, ImFontConfig &config, ImFontAtlas* atlas)
{
    float GlyphExtraSpacingX = 0.0f;
    float GlyphExtraSpacingY = 0.0f;
    float GlyphOffsetX = 0.0f;
    float GlyphOffsetY = 0.0f;

    lua_getfield(L, index, "glyphExtraSpacingX");
    if (!lua_isnil(L, -1)) GlyphExtraSpacingX = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "glyphExtraSpacingY");
    if (!lua_isnil(L, -1)) GlyphExtraSpacingY = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    config.GlyphExtraSpacing = ImVec2(GlyphExtraSpacingX, GlyphExtraSpacingY);

    lua_getfield(L, index, "glyphOffsetX");
    if (!lua_isnil(L, -1)) GlyphOffsetX = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "glyphOffsetY");
    if (!lua_isnil(L, -1)) GlyphOffsetY = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    config.GlyphOffset = ImVec2(GlyphOffsetX, GlyphOffsetY);

    lua_getfield(L, index, "fontDataOwnedByAtlas");
    if (!lua_isnil(L, -1)) config.FontDataOwnedByAtlas = lua_toboolean(L, -1) > 0;
    lua_pop(L, 1);

    lua_getfield(L, index, "fixelSnapH");
    if (!lua_isnil(L, -1)) config.PixelSnapH = lua_toboolean(L, -1) > 0;
    lua_pop(L, 1);

    lua_getfield(L, index, "fontNo");
    if (!lua_isnil(L, -1)) config.FontNo = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "oversampleH");
    if (!lua_isnil(L, -1)) config.OversampleH = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "oversampleV");
    if (!lua_isnil(L, -1)) config.OversampleV = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "SizePixels");
    if (!lua_isnil(L, -1)) config.SizePixels = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "glyphMinAdvanceX");
    if (!lua_isnil(L, -1)) config.GlyphMinAdvanceX = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "glyphMaxAdvanceX");
    if (!lua_isnil(L, -1)) config.GlyphMaxAdvanceX = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "mergeMode");
    if (!lua_isnil(L, -1)) config.MergeMode = lua_toboolean(L, -1) > 0;
    lua_pop(L, 1);

    lua_getfield(L, index, "rasterizerFlags");
    if (!lua_isnil(L, -1)) config.RasterizerFlags = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "rasterizerMultiply");
    if (!lua_isnil(L, -1)) config.RasterizerMultiply = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "ellipsisChar");
    if (!lua_isnil(L, -1)) config.EllipsisChar = (ImWchar)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "glyphs");
    if (!lua_isnil(L, -1))
    {
        ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;

        luaL_checktype(L, 1, LUA_TTABLE);

        lua_getfield(L, -1, "text");
        if (!lua_isnil(L, -1)) builder.AddText(luaL_checkstring(L, -1));
        lua_pop(L, 1);

        readConfTable(L, "chars", builder, atlas, addConfChars);
        readConfTable(L, "ranges", builder, atlas, addConfRanges);
        //readConfTable(L, "customRanges", builder, atlas, addConfCustomRanges);

        //builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());

        builder.BuildRanges(&ranges);
        config.GlyphRanges = ranges.Data;
    }
    lua_pop(L, 1);
}

ImFont* addFont(ImFontAtlas* atlas, const char* file_name, double size_pixels, ImFontConfig& font_cfg)
{
    const char* p;
    for (p = file_name + strlen(file_name); p > file_name && p[-1] != '/' && p[-1] != '\\'; p--) {}
    ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);

    FontData font_data = getFontData(NULL, file_name);
    return atlas->AddFontFromMemoryTTF(font_data.data, font_data.size, size_pixels, &font_cfg);
}

int FontAtlas_AddFont(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    ImFontConfig font_cfg = ImFontConfig();

    const char* file_name = luaL_checkstring(L, 2);
    double size_pixels = luaL_checknumber(L, 3);

    // load options table
    if (lua_gettop(L) > 3)
    {
        luaL_checktype(L, 4, LUA_TTABLE);
        lua_pushvalue(L, 4); // push options table to top
        loadFontConfig(L, 4, font_cfg, atlas);
        lua_pop(L, 1); // pop options table
    }

    ImFont* font = addFont(atlas, file_name, size_pixels, font_cfg);
    Binder binder(L);
    binder.pushInstance("ImFont", font);
    return 1;
}

int FontAtlas_AddFonts(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);

    luaL_checktype(L, 2, LUA_TTABLE);
    int len = luaL_getn(L, 2);

    for (int i = 0; i < len; i++)
    {
        lua_rawgeti(L, 2, i + 1);

        lua_rawgeti(L, 3, 1);
        const char* file_name = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, 3, 2);
        double size_pixels = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        ImFontConfig font_cfg = ImFontConfig();

        // options table
        lua_rawgeti(L, 3, 3);
        if (!lua_isnil(L, -1))
        {
            luaL_checktype(L, -1, LUA_TTABLE);
            lua_pushvalue(L, -1); // push options table to top
            loadFontConfig(L, -1, font_cfg, atlas);
            lua_pop(L, 1); // pop options table
        }
        lua_pop(L, 1);

        addFont(atlas, file_name, size_pixels, font_cfg);

        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return 0;
}

int FontAtlas_GetFontByIndex(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    int index = 0;
    if (lua_gettop(L) > 1 && !lua_isnil(L, 2))
    {
        index = luaL_checkinteger(L, 2);
    }
    int fonts_count = atlas->Fonts.Size;
    LUA_ASSERT(index >= 0 && index < fonts_count, "Font index is out of bounds!");
    ImFont* font = atlas->Fonts[index];
    LUA_ASSERT(font, "Font is nil");
    Binder binder(L);
    binder.pushInstance("ImFont", font);
    return 1;
}

int FontAtlas_GetCurrentFont(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance("ImFont", ImGui::GetFont());
    return 1;
}

int FontAtlas_AddDefaultFont(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->AddFontDefault();
    return 0;
}

int FontAtlas_BuildFont(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->Build();
    return 0;
}

int FontAtlas_Bake(lua_State* L)
{
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->ClearTexData();

    ImFontAtlas* atlas = getFontAtlas(L);

    unsigned char* pixels;
    int width, height;
    atlas->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_id texture = gtexture_create(width, height, GTEXTURE_RGBA, GTEXTURE_UNSIGNED_BYTE, GTEXTURE_CLAMP, GTEXTURE_LINEAR, pixels, NULL, 0);
    atlas->TexID = (void *)texture;

    return 0;
}

int FontAtlas_ClearInputData(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearInputData();
    return 0;
}

int FontAtlas_ClearTexData(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearTexData();
    return 0;
}

int FontAtlas_ClearFonts(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearFonts();
    return 0;
}

int FontAtlas_Clear(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->Clear();
    return 0;
}

int FontAtlas_IsBuilt(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    lua_pushboolean(L, atlas->IsBuilt());
    return 1;
}

int FontAtlas_AddCustomRectRegular(lua_State* L)
{
    int width  = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    ImFontAtlas* atlas = getFontAtlas(L);
    lua_pushinteger(L, atlas->AddCustomRectRegular(width, height));
    return 1;
}

int FontAtlas_AddCustomRectFontGlyph(lua_State* L)
{
    ImFont* font = getFont(L, 2);
    ImWchar id = (ImWchar)luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    float advance_x = luaL_checkinteger(L, 6);
    const ImVec2& offset = ImVec2(luaL_optnumber(L, 7, 0.0f), luaL_optnumber(L, 8, 0.0f));

    ImFontAtlas* atlas = getFontAtlas(L);
    lua_pushinteger(L, atlas->AddCustomRectFontGlyph(font, id, width, height, advance_x, offset));
    return 1;
}

int FontAtlas_GetCustomRectByIndex(lua_State* L)
{
    int index = luaL_checkinteger(L, 2);
    ImFontAtlas* atlas = getFontAtlas(L);
    ImFontAtlasCustomRect* rect = atlas->GetCustomRectByIndex(index);
    lua_pushinteger(L, rect->Width);
    lua_pushinteger(L, rect->Height);
    lua_pushinteger(L, rect->X);
    lua_pushinteger(L, rect->Y);
    lua_pushinteger(L, rect->X);
    lua_pushinteger(L, rect->GlyphID);
    lua_pushnumber(L, rect->GlyphOffset.x);
    lua_pushnumber(L, rect->GlyphOffset.y);
    Binder binder(L);
    binder.pushInstance("ImFont", rect->Font);
    lua_pushboolean(L, rect->IsPacked());
    return 10;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// DRAW LIST
///
/////////////////////////////////////////////////////////////////////////////////////////////

void ErrorCheck()
{
    ImGuiContext* g = ImGui::GetCurrentContext();
    LUA_ASSERT(g->FrameCount > 0, "Forgot to call newFrame()?");
    //LUA_ASSERT((g->FrameCount == 0 || g->FrameCountEnded == g->FrameCount), "Forgot to call Render() or EndFrame() at the end of the previous frame?");
    //LUA_ASSERT(g->IO.DisplaySize.x >= 0.0f && g->IO.DisplaySize.y >= 0.0f, "Invalid DisplaySize value!");
}

int GetWindowDrawList(lua_State* L)
{
    ErrorCheck();

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    binder.pushInstance("ImDrawList", draw_list);
    return 1;
}

int GetBackgroundDrawList(lua_State* L)
{
    ErrorCheck();

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    binder.pushInstance("ImDrawList", draw_list);
    return 1;
}

int GetForegroundDrawList(lua_State* L)
{
    ErrorCheck();

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    binder.pushInstance("ImDrawList", draw_list);
    return 1;
}

ImDrawList* getDrawList(lua_State* L)
{
    Binder binder(L);
    return static_cast<ImDrawList*>(binder.getInstance("ImDrawList", 1));
}

int DrawList_PushClipRect(lua_State* L)
{
    ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = luaL_optboolean(L, 6, 0) > 0;

    ImDrawList* list = getDrawList(L);
    list->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int DrawList_PushClipRectFullScreen(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PushClipRectFullScreen();
    return 0;
}

int DrawList_PopClipRect(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PopClipRect();
    return 0;
}

int DrawList_PushTextureID(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImTextureID texture_id = getTexture(L, 2).texture;
    list->PushTextureID(texture_id);
    return 0;
}

int DrawList_PopTextureID(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PopTextureID();
    return 0;
}

int DrawList_GetClipRectMin(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImVec2 min = list->GetClipRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int DrawList_GetClipRectMax(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImVec2 max = list->GetClipRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int DrawList_AddLine(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    double thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddLine(p1, p2, col, thickness);
    return 0;
}

int DrawList_AddRect(lua_State* L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    double rounding = luaL_optnumber(L, 8, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 9, ImDrawCornerFlags_All);
    double thickness = luaL_optnumber(L, 10, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddRect(p_min, p_max, col, rounding, rounding_corners, thickness);

    return 0;
}

int DrawList_AddRectFilled(lua_State* L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    double rounding = luaL_optnumber(L, 8, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 9, ImDrawCornerFlags_All);

    ImDrawList* list = getDrawList(L);
    list->AddRectFilled(p_min, p_max, col, rounding, rounding_corners);

    return 0;
}

int DrawList_AddRectFilledMultiColor(lua_State* L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col_upr_left  = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    ImU32 col_upr_right = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));
    ImU32 col_bot_right = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));
    ImU32 col_bot_left  = GColor::toU32(luaL_checkinteger(L, 12), luaL_optnumber(L, 13, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddRectFilledMultiColor(p_min, p_max, col_upr_left, col_upr_right, col_bot_right, col_bot_left);

    return 0;
}

int DrawList_AddQuad(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));
    double thickness = luaL_optnumber(L, 12, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddQuad(p1, p2, p3, p4, col, thickness);

    return  0;
}

int DrawList_AddQuadFilled(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddQuadFilled(p1, p2, p3, p4, col);

    return  0;
}

int DrawList_AddTriangle(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));
    double thickness = luaL_optnumber(L, 10, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddTriangle(p1, p2, p3, col, thickness);

    return  0;
}

int DrawList_AddTriangleFilled(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddTriangleFilled(p1, p2, p3, col);

    return  0;
}

int DrawList_AddCircle(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);
    double thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddCircle(center, radius, col, num_segments, thickness);

    return 0;
}

int DrawList_AddCircleFilled(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList* list = getDrawList(L);
    list->AddCircleFilled(center, radius, col, num_segments);

    return 0;
}

int DrawList_AddNgon(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);
    double thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddNgon(center, radius, col, num_segments, thickness);

    return 0;
}

int DrawList_AddNgonFilled(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList* list = getDrawList(L);
    list->AddNgonFilled(center, radius, col, num_segments);

    return 0;
}

int DrawList_AddText(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 4), luaL_optnumber(L, 5, 1.0f));
    const char* text_begin = luaL_checkstring(L, 6);
    const char* text_end = luaL_optstring(L, 7, NULL);

    ImDrawList* list = getDrawList(L);
    list->AddText(pos, col, text_begin, text_end);

    return 0;
}

int DrawList_AddFontText(lua_State* L)
{
    ImFont* font = getFont(L, 2);
    double font_size = luaL_checknumber(L, 3);
    ImVec2 pos = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    const char* text_begin = luaL_checkstring(L, 8);
    double wrap_width = luaL_optnumber(L, 9, 0.0f);
    ImVec4* cpu_fine_clip_rect = NULL;
    if (lua_gettop(L) > 9)
    {
        ImVec4 rect = ImVec4(luaL_checknumber(L, 10), luaL_checknumber(L, 11), luaL_checknumber(L, 12), luaL_checknumber(L, 13));
        cpu_fine_clip_rect = &rect;
    }
    ImDrawList* list = getDrawList(L);
    list->AddText(font, font_size, pos, col, text_begin, NULL, wrap_width, cpu_fine_clip_rect);
    return 0;
}

int DrawList_AddPolyline(lua_State* L)
{

    luaL_checktype(L, 2, LUA_TTABLE);
    int index = 0;
    int num_points = luaL_getn(L, 2);
    ImVec2* points=new ImVec2[num_points];
    lua_pushvalue(L, 2);
    for (int i = 0; i < num_points; i+=2)
    {
        lua_rawgeti(L, 2, i+1);
        double x = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, 2, i+2);
        double y = luaL_checknumber(L, -1);
        points[index] = ImVec2(x,y);

        index ++;

        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    ImU32 col = GColor::toU32(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    bool closed = lua_toboolean(L, 5) > 0;
    double thickness = luaL_checknumber(L, 6);

    ImDrawList* list = getDrawList(L);
    list->AddPolyline(points, index, col, closed, thickness);
    delete[] points;
    return  0;
}

int DrawList_AddConvexPolyFilled(lua_State* L)
{

    luaL_checktype(L, 2, LUA_TTABLE);
    int index = 0;
    int num_points = luaL_getn(L, 2);
    ImVec2* points = new ImVec2[num_points];
    lua_pushvalue(L, 2);
    for (int i = 0; i < num_points; i+=2)
    {
        lua_rawgeti(L, 2, i + 1);
        double x = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, 2, i + 2);
        double y = luaL_checknumber(L, -1);
        points[index] = ImVec2(x, y);

        index++;

        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    ImU32 col = GColor::toU32(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddConvexPolyFilled(points, index, col);
    delete[] points;
    return  0;
}

int DrawList_AddBezierCurve(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));
    double thickness = luaL_checknumber(L, 12);
    int num_segments = luaL_optinteger(L, 13, 0);

    ImDrawList* list = getDrawList(L);
    list->AddBezierCurve(p1, p2, p3, p4, col, thickness, num_segments);
    return 0;
}

int DrawList_AddImage(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 1.0f));

    ImDrawList* list = getDrawList(L);
    ImGui::FitImage(p_min, p_max, p_max - p_min, data.texture_size, ImVec2(0.5f, 0.5f));

    list->AddImage(data.texture, p_min, p_max, data.uv0, data.uv1, col);
    return 0;
}

int DrawList_AddImageQuad(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 7), luaL_checknumber(L, 8));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 9), luaL_checknumber(L, 10));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 11, 0xffffff), luaL_optnumber(L, 12, 1.0f));
    ImVec2 uv1 = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));
    ImVec2 uv2 = ImVec2(luaL_optnumber(L, 15, 1.0f), luaL_optnumber(L, 16, 0.0f));
    ImVec2 uv3 = ImVec2(luaL_optnumber(L, 17, 1.0f), luaL_optnumber(L, 18, 1.0f));
    ImVec2 uv4 = ImVec2(luaL_optnumber(L, 19, 0.0f), luaL_optnumber(L, 20, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddImageQuad(data.texture, p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);
    return 0;
}

int DrawList_AddImageRounded(lua_State* L)
{
    GTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 7), luaL_optnumber(L, 8, 1.0f));
    double rounding = luaL_checknumber(L, 9);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 10, ImDrawCornerFlags_All);

    ImGui::FitImage(p_min, p_max, p_max - p_min, data.texture_size, ImVec2(0.5f, 0.5f));

    ImDrawList* list = getDrawList(L);
    list->AddImageRounded(data.texture, p_min, p_max, data.uv0, data.uv1, col, rounding, rounding_corners);
    return 0;
}

int DrawList_PathClear(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PathClear();
    return 0;
}

int DrawList_PathLineTo(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList* list = getDrawList(L);
    list->PathLineTo(pos);
    return 0;
}

int DrawList_PathLineToMergeDuplicate(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList* list = getDrawList(L);
    list->PathLineToMergeDuplicate(pos);
    return 0;
}

int DrawList_PathFillConvex(lua_State* L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    ImDrawList* list = getDrawList(L);
    list->PathFillConvex(color);
    return 0;

}

int DrawList_PathStroke(lua_State* L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    bool closed = lua_toboolean(L, 4) > 0;
    float thickness = luaL_optnumber(L, 3, 1.0f);
    ImDrawList* list = getDrawList(L);
    list->PathStroke(color, closed, thickness);
    return 0;
}

int DrawList_PathArcTo(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    double a_min = luaL_checknumber(L, 5);
    double a_max = luaL_checknumber(L, 6);
    int num_segments = luaL_optinteger(L, 7, 10);
    ImDrawList* list = getDrawList(L);
    list->PathArcTo(center, radius, a_min, a_max, num_segments);
    return 0;

}

int DrawList_PathArcToFast(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    int a_min = luaL_checkinteger(L, 5);
    int a_max = luaL_checkinteger(L, 6);
    ImDrawList* list = getDrawList(L);
    list->PathArcToFast(center, radius, a_min, a_max);
    return 0;

}

int DrawList_PathBezierCurveTo(lua_State* L)
{
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    int num_segments = luaL_optinteger(L, 8, 0);
    ImDrawList* list = getDrawList(L);
    list->PathBezierCurveTo(p2, p3, p4, num_segments);
    return 0;
}

int DrawList_PathRect(lua_State* L)
{
    ImVec2 rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    double rounding = luaL_optnumber(L, 6, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 7, ImDrawCornerFlags_All);
    ImDrawList* list = getDrawList(L);
    list->PathRect(rect_min, rect_max, rounding, rounding_corners);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// https://gist.github.com/carasuca/e72aacadcf6cf8139de46f97158f790f
// https://github.com/ocornut/imgui/issues/1286

int rotation_start_index;

int DrawList_RotateStart(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    rotation_start_index = list->VtxBuffer.Size;
    return 0;
}

ImVec2 DrawList_RotationCenter(ImDrawList* list)
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

    const ImVector<ImDrawVert>& buf = list->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x+u.x)/2, (l.y+u.y)/2); // or use _ClipRectStack?
}

int DrawList_RotateEnd(lua_State* L)
{
    float rad = luaL_checknumber(L, 2);
    ImDrawList* list = getDrawList(L);
    ImVec2 center = DrawList_RotationCenter(list);

    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    ImVector<ImDrawVert>& buf = list->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

int SetAutoUpdateCursor(lua_State* L)
{
    autoUpdateCursor = lua_toboolean(L, 2);
    return 0;
}

int GetAutoUpdateCursor(lua_State* L)
{

    lua_pushboolean(L, autoUpdateCursor);
    return 1;
}

int SetResetTouchPosOnEnd(lua_State* L)
{
    resetTouchPosOnEnd = lua_toboolean(L, 2);
    return 0;
}

int GetResetTouchPosOnEnd(lua_State* L)
{

    lua_pushboolean(L, resetTouchPosOnEnd);
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
/*
    int ImGui_my_Test2(lua_State* L)
    {
        float value = luaL_optnumber(L, 2, NULL);
        LUA_ASSERT(value < 0, "bad argument #2");
        lua_getglobal(L, "print");
        lua_pushboolean(L, NULL == value);
        lua_call(L, 1, 0);
        lua_pop(L, 1);
        return 0;
    }
    int ImGui_my_test_key_table(lua_State* L)
    {
        luaL_checktype(L, 2, LUA_TTABLE);
        size_t len = lua_objlen(L, 2);
        lua_pushvalue(L, 2);
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);
            int index = lua_tointeger(L, -1);
            const char* str = lua_tostring(L, -2);
            lua_pop(L, 2);
        }
        lua_pop(L, 1);
        return 0;
    }
    int ImGui_my_test_n_table(lua_State* L)
    {
        luaL_checktype(L, 2, LUA_TTABLE);
        size_t len = luaL_getn(L, 2);
        const char** items=new const char* [len];
        lua_pushvalue(L, 2);
        for (unsigned int i = 0; i < len; i++)
        {
            lua_rawgeti(L, 2, i + 1);
            const char* str = lua_tostring(L, -1);
            lua_getglobal(L, "print");
            lua_pushstring(L, str);
            lua_call(L, 1, 0);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        delete[] items;
        return 0;
    }
    */
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void DrawLuaStyleEditor(const char* title, bool* p_open = NULL, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
{
    if (!ImGui::Begin(title, p_open, flags))
    {
        ImGui::End();
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;
    static ImGuiStyle* ref;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ImGui::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;
    ImGui::ShowFontSelector("Fonts##Selector");

    // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    { bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder",  &border)) { style.FrameBorderSize  = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder",  &border)) { style.PopupBorderSize  = border ? 1.0f : 0.0f; } }

    static int output_dest = 0;

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style =* ref;

    static bool output_only_modified = true;

    if (ImGui::Button("Export"))
    {
        if (output_dest == 0)
            ImGui::LogToClipboard();
        else
            ImGui::LogToTTY();
        ImGui::LogText("%s", "local style = imgui:getStyle()\r\n");
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const ImVec4& col = style.Colors[i];
            const char* name = ImGui::GetStyleColorName(i);
            GColor gcolor = GColor::toHex(col);
            if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                ImGui::LogText("style:setColor(ImGui.Col_%s, 0x%06X, %.2f)\r\n", name, gcolor.hex, gcolor.alpha);
        }
        ImGui::LogFinish();
    }


    ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
    ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

    static ImGuiTextFilter filter;
    filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

    static ImGuiColorEditFlags alpha_flags = 0;
    if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))             { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
    if (ImGui::RadioButton("Alpha",  alpha_flags == ImGuiColorEditFlags_AlphaPreview))     { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
    if (ImGui::RadioButton("Both",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
    HelpMarker(
                "In the color list:\n"
                "Left-click on colored square to open color picker,\n"
                "Right-click to open edit options menu.");

    ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
    ImGui::PushItemWidth(-160);
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        const char* name = ImGui::GetStyleColorName(i);
        if (!filter.PassFilter(name))
            continue;
        ImGui::PushID(i);
        ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
        if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
        {
            // Tips: in a real user application, you may want to merge and use an icon font into the main font,
            // so instead of "Save"/"Revert" you'd use icons!
            // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
            ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
            ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
        }
        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        ImGui::TextUnformatted(name);
        ImGui::PopID();
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();

    ImGui::End();
}

int ShowLuaStyleEditor(lua_State* L)
{
    const char* title = luaL_checkstring(L, 2);

    ImGuiWindowFlags window_flags = luaL_optinteger(L, 4, ImGuiWindowFlags_None);

    int type = lua_type(L, 3);
    if (type == LUA_TBOOLEAN)
    {
        bool p_open = lua_toboolean(L, 3);
        DrawLuaStyleEditor(title, &p_open, window_flags);
        lua_pushboolean(L, p_open);
        return 1;
    }
    else
    {
        DrawLuaStyleEditor(title, NULL, window_flags);
        return 0;
    }
}

struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.
    bool                Shown;

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
    {
        if (!ImGui::Begin(title, p_open, flags))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::SameLine();
        if (ImGui::Button("X"))
            Filter.Clear();

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            Clear();
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

static ExampleAppLog logapp;

int ShowLog(lua_State* L)
{
    const char* title = luaL_checkstring(L, 2);
    bool* p_open = getPopen(L, 3);
    ImGuiWindowFlags window_flags = luaL_optinteger(L, 4, ImGuiWindowFlags_None);

    logapp.Draw(title, p_open, window_flags);

    if (p_open != nullptr)
    {
        logapp.Shown = *p_open;
        lua_pushboolean(L, *p_open);
        delete p_open;
        return 1;
    }
    logapp.Shown = false;
    return 0;
}

int WriteLog(lua_State* L)
{
    if (!logapp.Shown)
        return 0;

    const char* text = luaL_checkstring(L, 2);
    logapp.AddLog("%s\n", text);

    return 0;
}

#ifdef IS_BETA_BUILD

int initNodeEditor(lua_State* L)
{
    Binder binder(L);
    NodeEditor* editor = new NodeEditor();
    binder.pushInstance("ImGuiNodeEditor", editor);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, editor);
    lua_pop(L, 1);

    return 1;
}

int destroyNodeEditor(lua_State* _UNUSED(L))
{
    return 0;
}

int ED_SetCurrentEditor(lua_State* L)
{
    if (lua_type(L, 2) == LUA_TNIL)
    {
        ED::SetCurrentEditor(nullptr);
        return 0;
    }

    Binder binder(L);
    //LUA_ASSERT(binder.isInstanceOf("ImGuiNodeEditor", 2), "");
    NodeEditor* editor = static_cast<NodeEditor*>(binder.getInstance("ImGuiNodeEditor", 2));
    ED::SetCurrentEditor(editor->ctx);
    return 0;
}

/*
int ED_GetCurrentEditor(lua_State* L)
{
    Binder binder(L);
    ED::EditorContext* ctx = static_cast<ED::EditorContext*>(binder.getInstance("ImGuiNodeEditor", 2));
    return 0;
}

int ED_CreateEditor(lua_State* L)
{
    return 0;
}

int ED_DestroyEditor(lua_State* L)
{
    return 0;
}
*/


int getColorIndex(lua_State* L)
{
    int index = luaL_checkinteger(L, 2);
    LUA_ASSERT(index >= 0 && index < ED::StyleColor_Count, "bar argument #1, index is out of bounds");
    return index;
}

int ED_GetStyle(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance("ImGuiEDStyle", &ED::GetStyle());
    return 1;
}

int ED_GetStyleColorName(lua_State* L)
{
    int index = getColorIndex(L);
    lua_pushstring(L, ED::GetStyleColorName((ED::StyleColor)index));
    return 1;
}

int ED_PushStyleColor(lua_State* L)
{
    ED::StyleColor color = (ED::StyleColor)luaL_checkinteger(L, 2);
    LUA_ASSERT(color >= 0 && color < ED::StyleColor_Count, "Color index is out of bounds!");
    ED::PushStyleColor(color, GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)));
    return 0;
}

int ED_PopStyleColor(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ED::PopStyleColor(count);
    return 0;
}

int ED_PushStyleVar(lua_State* L)
{
    ED::StyleVar style = (ED::StyleVar)luaL_checkinteger(L, 2);
    LUA_ASSERT(style >= 0 && style < ED::StyleVar_Count, "StyleVar is out of bounds!");
    int top = lua_gettop(L);

    if (top == 3)
        ED::PushStyleVar(style, luaL_checknumber(L, 3));
    else if (top == 4)
        ED::PushStyleVar(style, ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4)));
    else
        ED::PushStyleVar(style, ImVec4(luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6)));

    return 0;
}

int ED_PopStyleVar(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ED::PopStyleVar(count);
    return 0;
}

int ED_Begin(lua_State* L)
{
    const char* id = luaL_checkstring(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, 0.0f), luaL_optnumber(L, 4, 0.0f));
    ED::Begin(id, size);
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ED_End(lua_State* _UNUSED(L))
{
    ED::End();
    return 0;
}

int ED_BeginNode(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::BeginNode(id);
    return 0;
}

int ED_BeginPin(lua_State* L)
{
    ED::PinId id = luaL_checkinteger(L, 2);
    ED::PinKind kind = (ED::PinKind)luaL_checkinteger(L, 3);
    LUA_ASSERT(kind >= ED::PinKind::Input && kind <= ED::PinKind::Output, "Incorrect arg #3");
    ED::BeginPin(id, kind);
    return 0;
}

int ED_PinRect(lua_State* L)
{
    ImVec2 a = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 b = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ED::PinRect(a, b);
    lua_pushnumber(L, a.x);
    lua_pushnumber(L, a.y);
    lua_pushnumber(L, b.x);
    lua_pushnumber(L, b.y);
    return 4;
}

int ED_PinPivotRect(lua_State* L)
{
    ImVec2 a = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 b = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ED::PinPivotRect(a, b);
    lua_pushnumber(L, a.x);
    lua_pushnumber(L, a.y);
    lua_pushnumber(L, b.x);
    lua_pushnumber(L, b.y);
    return 4;
}

int ED_PinPivotSize(lua_State* L)
{
    ImVec2 size = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ED::PinPivotSize(size);
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ED_PinPivotScale(lua_State* L)
{
    ImVec2 scale = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ED::PinPivotScale(scale);
    lua_pushnumber(L, scale.x);
    lua_pushnumber(L, scale.y);
    return 2;
}

int ED_PinPivotAlignment(lua_State* L)
{
    ImVec2 align = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ED::PinPivotAlignment(align);
    lua_pushnumber(L, align.x);
    lua_pushnumber(L, align.y);
    return 2;
}

int ED_EndPin(lua_State* _UNUSED(L))
{
    ED::EndPin();
    return 0;
}

int ED_Group(lua_State* L)
{
    ImVec2 size = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ED::Group(size);
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ED_EndNode(lua_State* _UNUSED(L))
{
    ED::EndNode();
    return 0;
}

int ED_BeginGroupHint(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::BeginGroupHint(id);
    return 0;
}

int ED_GetGroupMin(lua_State* L)
{
    ImVec2 min = ED::GetGroupMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ED_GetGroupMax(lua_State* L)
{
    ImVec2 max = ED::GetGroupMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ED_GetHintForegroundDrawList(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance("ImDrawList", ED::GetHintForegroundDrawList());
    return 1;
}

int ED_GetHintBackgroundDrawList(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance("ImDrawList", ED::GetHintBackgroundDrawList());
    return 1;
}

int ED_EndGroupHint(lua_State* _UNUSED(L))
{
    ED::EndGroupHint();
    return 0;
}

int ED_GetNodeBackgroundDrawList(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    Binder binder(L);
    binder.pushInstance("ImDrawList", ED::GetNodeBackgroundDrawList(id));
    return 1;
}

int ED_Link(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    ED::PinId startPin = luaL_checkinteger(L, 3);
    ED::PinId endPin = luaL_checkinteger(L, 4);
    ImVec4 color = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    float thickness = luaL_optnumber(L, 7, 1.0f);

    ED::Link(id, startPin, endPin, color, thickness);
    return 0;
}

int ED_Flow(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    ED::Flow(id);
    return 0;
}

int ED_BeginCreate(lua_State* L)
{
    ImVec4 color = GColor::toVec4(luaL_optinteger(L, 2, 0xffffff), luaL_optnumber(L, 3, 1.0f));
    float thickness = luaL_optnumber(L, 4, 1.0f);
    lua_pushboolean(L, ED::BeginCreate(color, thickness));
    return 1;
}

int ED_QueryNewLink(lua_State* L)
{
    ED::PinId startPin = luaL_optnumber(L, 2, NULL);
    ED::PinId endPin = luaL_optnumber(L, 3, NULL);

    if (lua_gettop(L) > 3)
    {
        ImVec4 color = GColor::toVec4(luaL_checknumber(L, 4), luaL_optnumber(L, 5, 1.0f));
        float thickness = luaL_optnumber(L, 6, 1.0f);
        lua_pushboolean(L, ED::QueryNewLink(&startPin, &endPin, color, thickness));

    }
    else
        lua_pushboolean(L, ED::QueryNewLink(&startPin, &endPin));

    lua_pushnumber(L, startPin.Get());
    lua_pushnumber(L, endPin.Get());
    return 3;
}

int ED_QueryNewNode(lua_State* L)
{
    ED::PinId id = luaL_checkinteger(L, 2);
    if (lua_gettop(L) > 2)
    {
        ImVec4 color = GColor::toVec4(luaL_checknumber(L, 3), luaL_optnumber(L, 4, 1.0f));
        float thickness = luaL_optnumber(L, 5, 1.0f);
        lua_pushboolean(L, ED::QueryNewNode(&id, color, thickness));

    }
    else
        lua_pushboolean(L, ED::QueryNewNode(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_AcceptNewItem(lua_State* L)
{
    if (lua_gettop(L) > 1)
    {
        ImVec4 color = GColor::toVec4(luaL_checknumber(L, 2), luaL_optnumber(L, 3, 1.0f));
        float thickness = luaL_optnumber(L, 4, 1.0f);
        lua_pushboolean(L, ED::AcceptNewItem(color, thickness));

    }
    else
        lua_pushboolean(L, ED::AcceptNewItem());
    return 1;
}

int ED_RejectNewItem(lua_State* L)
{
    if (lua_gettop(L) > 1)
    {
        ImVec4 color = GColor::toVec4(luaL_checknumber(L, 2), luaL_optnumber(L, 3, 1.0f));
        float thickness = luaL_optnumber(L, 4, 1.0f);
        ED::RejectNewItem(color, thickness);

    }
    else
        ED::RejectNewItem();
    return 0;
}

int ED_EndCreate(lua_State* _UNUSED(L))
{
    ED::EndCreate();
    return 0;
}

int ED_BeginDelete(lua_State* L)
{
    lua_pushboolean(L, ED::BeginDelete());
    return 1;
}

int ED_QueryDeletedLink(lua_State* L)
{
    ED::LinkId id = luaL_optnumber(L, 2, NULL);
    lua_pushboolean(L, ED::QueryDeletedLink(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_QueryDeletedNode(lua_State* L)
{
    ED::NodeId id = luaL_optnumber(L, 2, NULL);
    lua_pushboolean(L, ED::QueryDeletedNode(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_AcceptDeletedItem(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptDeletedItem());
    return 1;
}

int ED_RejectDeletedItem(lua_State* _UNUSED(L))
{
    ED::RejectDeletedItem();
    return 0;
}

int ED_EndDelete(lua_State* _UNUSED(L))
{
    ED::EndDelete();
    return 0;
}

int ED_SetNodePosition(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ImVec2 pos = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ED::SetNodePosition(id, pos);
    return 0;
}

int ED_GetNodePosition(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ImVec2 pos = ED::GetNodePosition(id);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ED_GetNodeSize(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ImVec2 size = ED::GetNodeSize(id);
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ED_CenterNodeOnScreen(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::CenterNodeOnScreen(id);
    return 0;
}

int ED_RestoreNodeState(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::RestoreNodeState(id);
    return 0;
}

int ED_Suspend(lua_State* _UNUSED(L))
{
    ED::Suspend();
    return 0;
}

int ED_Resume(lua_State* _UNUSED(L))
{
    ED::Resume();
    return 0;
}

int ED_IsSuspended(lua_State* _UNUSED(L))
{
    ED::IsSuspended();
    return 0;
}

int ED_IsActive(lua_State* _UNUSED(L))
{
    ED::IsActive();
    return 0;
}

int ED_HasSelectionChanged(lua_State* L)
{
    lua_pushboolean(L, ED::HasSelectionChanged());
    return 1;
}

int ED_GetSelectedObjectCount(lua_State* L)
{
    lua_pushinteger(L, ED::GetSelectedObjectCount());
    return 1;
}

int ED_GetSelectedNodes(lua_State* L)
{
    int size = luaL_checkinteger(L, 2);
    ED::NodeId* nodes = new ED::NodeId[size];
    int actualSize = ED::GetSelectedNodes(nodes, size);
    if (actualSize <= 0)
    {
        delete[] nodes;
        return 0;
    }

    lua_createtable(L, actualSize, 0);
    for (int i = 0; i < actualSize; i++)
    {
        lua_pushnumber(L, nodes[i].Get());
        lua_rawseti(L, -2, i + 1);
    }
    delete[] nodes;
    return 1;
}

int ED_GetSelectedLinks(lua_State* L)
{
    int size = luaL_checkinteger(L, 2);
    ED::LinkId* links = new ED::LinkId[size];
    int actualSize = ED::GetSelectedLinks(links, size);
    if (actualSize <= 0)
    {
        delete[] links;
        return 0;
    }

    lua_createtable(L, actualSize, 0);
    for (int i = 0; i < actualSize; i++)
    {
        lua_pushnumber(L, links[i].Get());
        lua_rawseti(L, -2, i + 1);
    }
    delete[] links;
    return 1;
}

int ED_ClearSelection(lua_State* _UNUSED(L))
{
    ED::ClearSelection();
    return 0;
}

int ED_SelectNode(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::SelectNode(id, luaL_optboolean(L, 3, 0));
    return 0;
}

int ED_SelectLink(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    ED::SelectLink(id, luaL_optboolean(L, 3, 0));
    return 0;
}

int ED_DeselectNode(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    ED::DeselectNode(id);
    return 0;
}

int ED_DeselectLink(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    ED::DeselectLink(id);
    return 0;
}

int ED_DeleteNode(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ED::DeleteNode(id));
    return 1;
}

int ED_DeleteLink(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ED::DeleteLink(id));
    return 1;
}

int ED_NavigateToContent(lua_State* L)
{
    float duration = luaL_optnumber(L, 2, -1);
    ED::NavigateToContent(duration);
    return 0;
}

int ED_NavigateToSelection(lua_State* L)
{
    bool zoomIn = luaL_optboolean(L, 2, 0);
    float duration = luaL_optnumber(L, 3, -1.0f);
    ED::NavigateToSelection(zoomIn, duration);
    return 0;
}

int ED_ShowNodeContextMenu(lua_State* L)
{
    ED::NodeId id = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ED::ShowNodeContextMenu(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_ShowPinContextMenu(lua_State* L)
{
    ED::PinId id = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ED::ShowPinContextMenu(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_ShowLinkContextMenu(lua_State* L)
{
    ED::LinkId id = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ED::ShowLinkContextMenu(&id));
    lua_pushnumber(L, id.Get());
    return 2;
}

int ED_ShowBackgroundContextMenu(lua_State* L)
{
    lua_pushboolean(L, ED::ShowBackgroundContextMenu());
    return 1;
}

int ED_EnableShortcuts(lua_State* L)
{
    ED::EnableShortcuts(lua_toboolean(L, 2));
    return 0;
}

int ED_AreShortcutsEnabled(lua_State* L)
{
    lua_pushboolean(L, ED::AreShortcutsEnabled());
    return 1;
}

int ED_BeginShortcut(lua_State* L)
{
    lua_pushboolean(L, ED::BeginShortcut());
    return 1;
}

int ED_AcceptCut(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptCut());
    return 1;
}

int ED_AcceptCopy(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptCopy());
    return 1;
}

int ED_AcceptPaste(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptPaste());
    return 1;
}

int ED_AcceptDuplicate(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptDuplicate());
    return 1;
}

int ED_AcceptCreateNode(lua_State* L)
{
    lua_pushboolean(L, ED::AcceptCreateNode());
    return 1;
}

int ED_GetActionContextSize(lua_State* L)
{
    lua_pushinteger(L, ED::GetActionContextSize());
    return 1;
}

int ED_GetActionContextNodes(lua_State* L)
{
    int size = luaL_checkinteger(L, 2);
    ED::NodeId* nodes = new ED::NodeId[size];
    int actualSize = ED::GetActionContextNodes(nodes, size);
    if (actualSize <= 0)
    {
        delete[] nodes;
        return 0;
    }

    lua_createtable(L, actualSize, 0);
    for (int i = 0; i < actualSize; i++)
    {
        lua_pushnumber(L, nodes[i].Get());
        lua_rawseti(L, -2, i + 1);
    }
    delete[] nodes;
    return 1;
}

int ED_GetActionContextLinks(lua_State* L)
{
    int size = luaL_checkinteger(L, 2);
    ED::LinkId* links = new ED::LinkId[size];
    int actualSize = ED::GetActionContextLinks(links, size);
    if (actualSize <= 0)
    {
        delete[] links;
        return 0;
    }

    lua_createtable(L, actualSize, 0);
    for (int i = 0; i < actualSize; i++)
    {
        lua_pushnumber(L, links[i].Get());
        lua_rawseti(L, -2, i + 1);
    }
    delete[] links;
    return 1;
}

int ED_EndShortcut(lua_State* _UNUSED(L))
{
    ED::EndShortcut();
    return 0;
}

int ED_GetCurrentZoom(lua_State* L)
{
    lua_pushnumber(L, ED::GetCurrentZoom());
    return 1;
}

int ED_GetDoubleClickedNode(lua_State* L)
{
    ED::NodeId id = ED::GetDoubleClickedNode();
    lua_pushnumber(L, id.Get());
    return 1;
}

int ED_GetDoubleClickedPin(lua_State* L)
{
    ED::PinId id = ED::GetDoubleClickedPin();
    lua_pushnumber(L, id.Get());
    return 1;
}

int ED_GetDoubleClickedLink(lua_State* L)
{
    ED::LinkId id = ED::GetDoubleClickedLink();
    lua_pushnumber(L, id.Get());
    return 1;
}

int ED_IsBackgroundClicked(lua_State* L)
{
    lua_pushboolean(L, ED::IsBackgroundClicked());
    return 1;
}

int ED_IsBackgroundDoubleClicked(lua_State* L)
{
    lua_pushboolean(L, ED::IsBackgroundDoubleClicked());
    return 1;
}

int ED_PinHadAnyLinks(lua_State* L)
{
    ED::PinId id = luaL_checkinteger(L,2);
    lua_pushboolean(L, ED::PinHadAnyLinks(id));
    return 1;
}

int ED_GetScreenSize(lua_State* L)
{
    ImVec2 size = ED::GetScreenSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ED_ScreenToCanvas(lua_State* L)
{
    ImVec2 spos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 cpos = ED::ScreenToCanvas(spos);
    lua_pushnumber(L, cpos.x);
    lua_pushnumber(L, cpos.y);
    return 2;
}

int ED_CanvasToScreen(lua_State* L)
{
    ImVec2 cpos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 spos = ED::CanvasToScreen(cpos);
    lua_pushnumber(L, spos.x);
    lua_pushnumber(L, spos.y);
    return 2;
}


ED::Style& getEDStyle(lua_State* L, int index = 1)
{
    Binder binder(L);
    ED::Style &style = *(static_cast<ED::Style*>(binder.getInstance("ImGuiEDStyle", index)));
    return style;
}

int ED_StyleGetNodePadding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.NodePadding.x);
    lua_pushnumber(L, style.NodePadding.y);
    lua_pushnumber(L, style.NodePadding.z);
    lua_pushnumber(L, style.NodePadding.w);
    return 4;
}

int ED_StyleSetNodePadding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    float value3 = luaL_checknumber(L, 4);
    float value4 = luaL_checknumber(L, 5);
    style.NodePadding = ImVec4(value1, value2, value3, value4);
    return 0;
}

int ED_StyleGetNodeRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.NodeRounding);
    return 1;
}

int ED_StyleSetNodeRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.NodeRounding = value;
    return 0;
}

int ED_StyleGetNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.NodeBorderWidth);
    return 1;
}

int ED_StyleSetNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.NodeBorderWidth = value;
    return 0;
}

int ED_StyleGetHoveredNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.HoveredNodeBorderWidth);
    return 1;
}

int ED_StyleSetHoveredNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.HoveredNodeBorderWidth = value;
    return 0;
}

int ED_StyleGetSelectedNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.SelectedNodeBorderWidth);
    return 1;
}

int ED_StyleSetSelectedNodeBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.SelectedNodeBorderWidth = value;
    return 0;
}

int ED_StyleGetPinRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinRounding);
    return 1;
}

int ED_StyleSetPinRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinRounding = value;
    return 0;
}

int ED_StyleGetPinBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinBorderWidth);
    return 1;
}

int ED_StyleSetPinBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinBorderWidth = value;
    return 0;
}

int ED_StyleGetLinkStrength(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.LinkStrength);
    return 1;
}

int ED_StyleSetLinkStrength(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.LinkStrength = value;
    return 0;
}

int ED_StyleGetSourceDirection(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.SourceDirection.x);
    lua_pushnumber(L, style.SourceDirection.y);
    return 2;
}

int ED_StyleSetSourceDirection(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    style.SourceDirection = ImVec2(value1, value2);
    return 0;
}

int ED_StyleGetTargetDirection(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.TargetDirection.x);
    lua_pushnumber(L, style.TargetDirection.y);
    return 2;
}

int ED_StyleSetTargetDirection(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    style.TargetDirection = ImVec2(value1, value2);
    return 0;
}

int ED_StyleGetScrollDuration(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.ScrollDuration);
    return 1;
}

int ED_StyleSetScrollDuration(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.ScrollDuration = value;
    return 0;
}

int ED_StyleGetFlowMarkerDistance(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.FlowMarkerDistance);
    return 1;
}

int ED_StyleSetFlowMarkerDistance(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.FlowMarkerDistance = value;
    return 0;
}

int ED_StyleGetFlowSpeed(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.FlowSpeed);
    return 1;
}

int ED_StyleSetFlowSpeed(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.FlowSpeed = value;
    return 0;
}

int ED_StyleGetFlowDuration(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.FlowDuration);
    return 1;
}

int ED_StyleSetFlowDuration(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.FlowDuration = value;
    return 0;
}

int ED_StyleGetPivotAlignment(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PivotAlignment.x);
    lua_pushnumber(L, style.PivotAlignment.y);
    return 2;
}

int ED_StyleSetPivotAlignment(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    style.PivotAlignment = ImVec2(value1, value2);
    return 0;
}

int ED_StyleGetPivotSize(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PivotSize.x);
    lua_pushnumber(L, style.PivotSize.y);
    return 2;
}

int ED_StyleSetPivotSize(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    style.PivotSize = ImVec2(value1, value2);
    return 0;
}

int ED_StyleGetPivotScale(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PivotScale.x);
    lua_pushnumber(L, style.PivotScale.y);
    return 2;
}

int ED_StyleSetPivotScale(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value1 = luaL_checknumber(L, 2);
    float value2 = luaL_checknumber(L, 3);
    style.PivotScale = ImVec2(value1, value2);
    return 0;
}

int ED_StyleGetPinCorners(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinCorners);
    return 1;
}

int ED_StyleSetPinCorners(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinCorners = value;
    return 0;
}

int ED_StyleGetPinRadius(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinRadius);
    return 1;
}

int ED_StyleSetPinRadius(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinRadius = value;
    return 0;
}

int ED_StyleGetPinArrowSize(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinArrowSize);
    return 1;
}

int ED_StyleSetPinArrowSize(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinArrowSize = value;
    return 0;
}

int ED_StyleGetPinArrowWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.PinArrowWidth);
    return 1;
}

int ED_StyleSetPinArrowWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.PinArrowWidth = value;
    return 0;
}

int ED_StyleGetGroupRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.GroupRounding);
    return 1;
}

int ED_StyleSetGroupRounding(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.GroupRounding = value;
    return 0;
}

int ED_StyleGetGroupBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    lua_pushnumber(L, style.GroupBorderWidth);
    return 1;
}

int ED_StyleSetGroupBorderWidth(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    float value = luaL_checknumber(L, 2);
    style.GroupBorderWidth = value;
    return 0;
}

int ED_StyleGetColor(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    int index = getColorIndex(L);
    GColor color = GColor::toHex(style.Colors[index]);
    lua_pushinteger(L, color.hex);
    lua_pushnumber(L, color.alpha);
    return 2;
}

int ED_StyleSetColor(lua_State* L)
{
    ED::Style style = getEDStyle(L);
    int index = getColorIndex(L);
    style.Colors[index] = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    return 0;
}

#endif

int loader(lua_State* L)
{
    Binder binder(L);

    const luaL_Reg imguiStylesFunctionList[] =
    {
        {"setColor", Style_SetColor},
        {"getColor", Style_GetColor},
        {"setAlpha", Style_SetAlpha},
        {"getAlpha", Style_GetAlpha},
        {"setWindowRounding", Style_SetWindowRounding},
        {"getWindowRounding", Style_GetWindowRounding},
        {"setWindowBorderSize", Style_SetWindowBorderSize},
        {"getWindowBorderSize", Style_GetWindowBorderSize},
        {"setChildRounding", Style_SetChildRounding},
        {"getChildRounding", Style_GetChildRounding},
        {"setChildBorderSize", Style_SetChildBorderSize},
        {"getChildBorderSize", Style_GetChildBorderSize},
        {"setPopupRounding", Style_SetPopupRounding},
        {"getPopupRounding", Style_GetPopupRounding},
        {"setPopupBorderSize", Style_SetPopupBorderSize},
        {"getPopupBorderSize", Style_GetPopupBorderSize},
        {"setFrameRounding", Style_SetFrameRounding},
        {"getFrameRounding", Style_GetFrameRounding},
        {"setFrameBorderSize", Style_SetFrameBorderSize},
        {"getFrameBorderSize", Style_GetFrameBorderSize},
        {"setIndentSpacing", Style_SetIndentSpacing},
        {"getIndentSpacing", Style_GetIndentSpacing},
        {"setColumnsMinSpacing", Style_SetColumnsMinSpacing},
        {"getColumnsMinSpacing", Style_GetColumnsMinSpacing},
        {"setScrollbarSize", Style_SetScrollbarSize},
        {"getScrollbarSize", Style_GetScrollbarSize},
        {"setScrollbarRounding", Style_SetScrollbarRounding},
        {"getScrollbarRounding", Style_GetScrollbarRounding},
        {"setGrabMinSize", Style_SetGrabMinSize},
        {"getGrabMinSize", Style_GetGrabMinSize},
        {"setGrabRounding", Style_SetGrabRounding},
        {"getGrabRounding", Style_GetGrabRounding},
        {"setLogSliderDeadzone", Style_SetLogSliderDeadzone},
        {"getLogSliderDeadzone", Style_GetLogSliderDeadzone},
        {"setTabRounding", Style_SetTabRounding},
        {"getTabRounding", Style_GetTabRounding},
        {"setTabBorderSize", Style_SetTabBorderSize},
        {"getTabBorderSize", Style_GetTabBorderSize},
        {"setTabMinWidthForUnselectedCloseButton", Style_SetTabMinWidthForUnselectedCloseButton},
        {"getTabMinWidthForUnselectedCloseButton", Style_GetTabMinWidthForUnselectedCloseButton},
        {"setTabMinWidthForCloseButton", Style_SetTabMinWidthForCloseButton},
        {"getTabMinWidthForCloseButton", Style_GetTabMinWidthForCloseButton},
        {"setMouseCursorScale", Style_SetMouseCursorScale},
        {"getMouseCursorScale", Style_GetMouseCursorScale},
        {"setCurveTessellationTol", Style_SetCurveTessellationTol},
        {"getCurveTessellationTol", Style_GetCurveTessellationTol},
        {"setCircleSegmentMaxError", Style_SetCircleSegmentMaxError},
        {"getCircleSegmentMaxError", Style_GetCircleSegmentMaxError},
        {"setWindowPadding", Style_SetWindowPadding},
        {"getWindowPadding", Style_GetWindowPadding},
        {"setWindowMinSize", Style_SetWindowMinSize},
        {"getWindowMinSize", Style_GetWindowMinSize},
        {"setWindowTitleAlign", Style_SetWindowTitleAlign},
        {"getWindowTitleAlign", Style_GetWindowTitleAlign},
        {"setFramePadding", Style_SetFramePadding},
        {"getFramePadding", Style_GetFramePadding},
        {"setItemSpacing", Style_SetItemSpacing},
        {"getItemSpacing", Style_GetItemSpacing},
        {"setItemInnerSpacing", Style_SetItemInnerSpacing},
        {"getItemInnerSpacing", Style_GetItemInnerSpacing},
        {"setTouchExtraPadding", Style_SetTouchExtraPadding},
        {"getTouchExtraPadding", Style_GetTouchExtraPadding},
        {"setButtonTextAlign", Style_SetButtonTextAlign},
        {"getButtonTextAlign", Style_GetButtonTextAlign},
        {"setSelectableTextAlign", Style_SetSelectableTextAlign},
        {"getSelectableTextAlign", Style_GetSelectableTextAlign},
        {"setDisplayWindowPadding", Style_SetDisplayWindowPadding},
        {"getDisplayWindowPadding", Style_GetDisplayWindowPadding},
        {"setDisplaySafeAreaPadding", Style_SetDisplaySafeAreaPadding},
        {"getDisplaySafeAreaPadding", Style_GetDisplaySafeAreaPadding},
        {"setWindowMenuButtonPosition", Style_SetWindowMenuButtonPosition},
        {"getWindowMenuButtonPosition", Style_GetWindowMenuButtonPosition},
        {"setColorButtonPosition", Style_SetColorButtonPosition},
        {"getColorButtonPosition", Style_GetColorButtonPosition},
        {"setAntiAliasedLines", Style_SetAntiAliasedLines},
        {"getAntiAliasedLines", Style_GetAntiAliasedLines},
        {"setAntiAliasedLinesUseTex", Style_SetAntiAliasedLinesUseTex},
        {"getAntiAliasedLinesUseTex", Style_GetAntiAliasedLinesUseTex},
        {"setAntiAliasedFill", Style_SetAntiAliasedFill},
        {"getAntiAliasedFill", Style_GetAntiAliasedFill},

        {NULL, NULL},
    };
    binder.createClass("ImGuiStyle", 0, NULL, NULL, imguiStylesFunctionList);

    const luaL_Reg imguiDrawListFunctionList[] =
    {
        {"pushClipRect", DrawList_PushClipRect},
        {"pushClipRectFullScreen", DrawList_PushClipRectFullScreen},
        {"popClipRect", DrawList_PopClipRect},
        {"pushTextureID", DrawList_PushTextureID},
        {"popTextureID", DrawList_PopTextureID},
        {"getClipRectMin", DrawList_GetClipRectMin},
        {"getClipRectMax", DrawList_GetClipRectMax},
        {"addLine", DrawList_AddLine},
        {"addRect", DrawList_AddRect},
        {"addRectFilled", DrawList_AddRectFilled},
        {"addRectFilledMultiColor", DrawList_AddRectFilledMultiColor},
        {"addQuad", DrawList_AddQuad},
        {"addQuadFilled", DrawList_AddQuadFilled},
        {"addTriangle", DrawList_AddTriangle},
        {"addTriangleFilled", DrawList_AddTriangleFilled},
        {"addCircle", DrawList_AddCircle},
        {"addCircleFilled", DrawList_AddCircleFilled},
        {"addNgon", DrawList_AddNgon},
        {"addNgonFilled", DrawList_AddNgonFilled},
        {"addText", DrawList_AddText},
        {"addFontText", DrawList_AddFontText},
        {"addPolyline", DrawList_AddPolyline},
        {"addConvexPolyFilled", DrawList_AddConvexPolyFilled},
        {"addBezierCurve", DrawList_AddBezierCurve},

        {"addImage", DrawList_AddImage},
        {"addImageQuad", DrawList_AddImageQuad},
        {"addImageRounded", DrawList_AddImageRounded},
        {"pathClear", DrawList_PathClear},
        {"pathLineTo", DrawList_PathLineTo},
        {"pathLineToMergeDuplicate", DrawList_PathLineToMergeDuplicate},
        {"pathFillConvex", DrawList_PathFillConvex},
        {"pathStroke", DrawList_PathStroke},
        {"pathArcTo", DrawList_PathArcTo},
        {"pathArcToFast", DrawList_PathArcToFast},
        {"pathBezierCurveTo", DrawList_PathBezierCurveTo},
        {"pathRect", DrawList_PathRect},

        {"rotateBegin", DrawList_RotateStart},
        {"rotateEnd", DrawList_RotateEnd},
        {NULL, NULL}
    };
    binder.createClass("ImDrawList", 0, NULL, NULL, imguiDrawListFunctionList);

    const luaL_Reg imguiIoFunctionList[] =
    {
        {"setFontDefault", IO_SetFontDefault},
        {"getFonts", IO_GetFonts},
        {"getDeltaTime", IO_GetDeltaTime},
        {"isMouseDown", IO_isMouseDown},
        {"getMouseWheel", IO_GetMouseWheel},
        {"getMouseWheelH", IO_GetMouseWheelH},
        {"isKeyCtrl", IO_isKeyCtrl},
        {"isKeyShift", IO_isKeyShift},
        {"isKeyAlt", IO_isKeyAlt},
        {"isKeySuper", IO_isKeySuper},
        {"getKeysDown", IO_GetKeysDown},
        {"wantCaptureMouse", IO_WantCaptureMouse},
        {"wantCaptureKeyboard", IO_WantCaptureKeyboard},
        {"wantTextInput", IO_WantTextInput},
        {"wantSetMousePos", IO_WantSetMousePos},
        {"wantSaveIniSettings", IO_WantSaveIniSettings},
        /// NAVIGATION +
        {"setNavInput", IO_SetNavInput},
        {"getNavInput", IO_GetNavInput},
        {"setNavNavInputsDownDuration", IO_SetNavInputsDownDuration},
        {"getNavNavInputsDownDuration", IO_GetNavInputsDownDuration},
        {"setNavNavInputsDownDurationPrev", IO_SetNavInputsDownDurationPrev},
        {"getNavNavInputsDownDurationPrev", IO_GetNavInputsDownDurationPrev},
        {"isNavActive", IO_IsNavActive},
        {"isNavVisible", IO_IsNavVisible},
        /// NAVIGATION -
        {"getFramerate", IO_GetFramerate},
        {"getMetricsRenderVertices", IO_GetMetricsRenderVertices},
        {"getMetricsRenderIndices", IO_GetMetricsRenderIndices},
        {"getMetricsRenderWindows", IO_GetMetricsRenderWindows},
        {"getMetricsActiveWindows", IO_GetMetricsActiveWindows},
        {"getMetricsActiveAllocations", IO_GetMetricsActiveAllocations},
        {"getMouseDelta", IO_GetMouseDelta},
        {"getMouseDownSec", IO_GetMouseDownSec},
        {"setDisplaySize", IO_SetDisplaySize},
        {"getDisplaySize", IO_GetDisplaySize},

    #ifdef IS_BETA_BUILD
        {"setConfigDockingNoSplit", IO_GetConfigDockingNoSplit},
        {"setConfigDockingNoSplit", IO_SetConfigDockingNoSplit},
        {"setConfigDockingWithShift", IO_GetConfigDockingWithShift},
        {"setConfigDockingWithShift", IO_SetConfigDockingWithShift},
        {"setConfigDockingAlwaysTabBar", IO_GetConfigDockingAlwaysTabBar},
        {"setConfigDockingAlwaysTabBar", IO_SetConfigDockingAlwaysTabBar},
        {"setConfigDockingTransparentPayload", IO_GetConfigDockingTransparentPayload},
        {"setConfigDockingTransparentPayload", IO_SetConfigDockingTransparentPayload},
    #endif
        {"getConfigFlags", IO_GetConfigFlags},
        {"setConfigFlags", IO_SetConfigFlags},
        {"addConfigFlags", IO_AddConfigFlags},
        {"getBackendFlags", IO_GetBackendFlags},
        {"setBackendFlags", IO_SetBackendFlags},
        {"addBackendFlags", IO_AddBackendFlags},
        {"getIniSavingRate", IO_GetIniSavingRate},
        {"setIniSavingRate", IO_SetIniSavingRate},
        {"getIniFilename", IO_GetIniFilename},
        {"setIniFilename", IO_SetIniFilename},
        {"getLogFilename", IO_GetLogFilename},
        {"setLogFilename", IO_SetLogFilename},
        {"getMouseDoubleClickTime", IO_GetMouseDoubleClickTime},
        {"setMouseDoubleClickTime", IO_SetMouseDoubleClickTime},
        {"getMouseDragThreshold", IO_GetMouseDragThreshold},
        {"setMouseDragThreshold", IO_SetMouseDragThreshold},
        {"getMouseDrawCursor", IO_GetMouseDrawCursor},
        {"setMouseDrawCursor", IO_SetMouseDrawCursor},
        {"getMouseDoubleClickMaxDist", IO_GetMouseDoubleClickMaxDist},
        {"setMouseDoubleClickMaxDist", IO_SetMouseDoubleClickMaxDist},
        {"setMouseDown", IO_SetMouseDown},
        {"setMousePos", IO_SetMousePos},
        {"setMouseWheel", IO_SetMouseWheel},
        {"getKeyMapValue", IO_GetKeyMapValue},
        {"setKeyMapValue", IO_SetKeyMapValue},
        {"getKeyRepeatDelay", IO_GetKeyRepeatDelay},
        {"setKeyRepeatDelay", IO_SetKeyRepeatDelay},
        {"getKeyRepeatRate", IO_GetKeyRepeatRate},
        {"setKeyRepeatRate", IO_SetKeyRepeatRate},
        {"getFontGlobalScale", IO_GetFontGlobalScale},
        {"setFontGlobalScale", IO_SetFontGlobalScale},
        {"getFontAllowUserScaling", IO_GetFontAllowUserScaling},
        {"setFontAllowUserScaling", IO_SetFontAllowUserScaling},
        {"getDisplayFramebufferScale", IO_GetDisplayFramebufferScale},
        {"setDisplayFramebufferScale", IO_SetDisplayFramebufferScale},
        {"getConfigMacOSXBehaviors", IO_GetConfigMacOSXBehaviors},
        {"setConfigMacOSXBehaviors", IO_SetConfigMacOSXBehaviors},
        {"getConfigInputTextCursorBlink", IO_GetConfigInputTextCursorBlink},
        {"setConfigInputTextCursorBlink", IO_SetConfigInputTextCursorBlink},
        {"getConfigWindowsResizeFromEdges", IO_GetConfigWindowsResizeFromEdges},
        {"setConfigWindowsResizeFromEdges", IO_SetConfigWindowsResizeFromEdges},
        {"getConfigWindowsMoveFromTitleBarOnly", IO_GetConfigWindowsMoveFromTitleBarOnly},
        {"setConfigWindowsMoveFromTitleBarOnly", IO_SetConfigWindowsMoveFromTitleBarOnly},
        {"getConfigWindowsMemoryCompactTimer", IO_GetConfigWindowsMemoryCompactTimer},
        {"setConfigWindowsMemoryCompactTimer", IO_SetConfigWindowsMemoryCompactTimer},

        {"getBackendPlatformName", IO_GetBackendPlatformName},
        {"getBackendRendererName", IO_GetBackendRendererName},

        {NULL, NULL}
    };
    binder.createClass("ImGuiIO", 0, NULL, NULL, imguiIoFunctionList);

    const luaL_Reg imguiFontAtlasFunctionList[] =
    {
        {"addFont", FontAtlas_AddFont},
        {"addFonts", FontAtlas_AddFonts},
        {"getFont", FontAtlas_GetFontByIndex},
        {"getCurrentFont", FontAtlas_GetCurrentFont},
        {"addDefaultFont", FontAtlas_AddDefaultFont},
        {"build", FontAtlas_BuildFont},
        {"bake", FontAtlas_Bake},
        {"clearInputData", FontAtlas_ClearInputData},
        {"clearTexData", FontAtlas_ClearTexData},
        {"clearFonts", FontAtlas_ClearFonts},
        {"clear", FontAtlas_Clear},
        {"isBuilt", FontAtlas_IsBuilt},
        {"addCustomRectRegular", FontAtlas_AddCustomRectRegular},
        {"addCustomRectFontGlyph", FontAtlas_AddCustomRectFontGlyph},
        {"getCustomRectByIndex", FontAtlas_GetCustomRectByIndex},
        {NULL, NULL}
    };
    binder.createClass("ImFontAtlas", 0, NULL, NULL, imguiFontAtlasFunctionList);

    const luaL_Reg imguiFontFunctionList[] = {
        {NULL, NULL}
    };
    binder.createClass("ImFont", 0, NULL, NULL, imguiFontFunctionList);

#ifdef IS_BETA_BUILD
    const luaL_Reg imguiDockNodeFunctionList[] = {
        {"getID", DockBuilder_Node_GetID},
        {"getSharedFlags", DockBuilder_Node_GetSharedFlags},
        {"getLocalFlags", DockBuilder_Node_GetLocalFlags},
        {"getParentNode", DockBuilder_Node_GetParentNode},
        {"getChildNodes", DockBuilder_Node_GetChildNodes},
        //{"getWindows", DockBuilder_Node_GetWindows},
        {"getTabBar", DockBuilder_Node_GetTabBar},
        {"getPos", DockBuilder_Node_GetPos},
        {"getSize", DockBuilder_Node_GetSize},
        {"getSizeRef", DockBuilder_Node_GetSizeRef},
        {"getSplitAxis", DockBuilder_Node_GetSplitAxis},
        //{"getWindowClass", DockBuilder_Node_GetWindowClass},
        {"getState", DockBuilder_Node_GetState},
        //{"getHostWindow", DockBuilder_Node_GetHostWindow},
        //{"getVisibleWindow", DockBuilder_Node_GetVisibleWindow},
        {"getCentralNode", DockBuilder_Node_GetCentralNode},
        {"getOnlyNodeWithWindows", DockBuilder_Node_GetOnlyNodeWithWindows},
        {"getLastFrameAlive", DockBuilder_Node_GetLastFrameAlive},
        {"getLastFrameActive", DockBuilder_Node_GetLastFrameActive},
        {"getLastFrameFocused", DockBuilder_Node_GetLastFrameFocused},
        {"getLastFocusedNodeId", DockBuilder_Node_GetLastFocusedNodeId},
        {"getSelectedTabId", DockBuilder_Node_GetSelectedTabId},
        {"getWantCloseTabId", DockBuilder_Node_WantCloseTabId},
        {"getAuthorityForPos", DockBuilder_Node_GetAuthorityForPos},
        {"getAuthorityForSize", DockBuilder_Node_GetAuthorityForSize},
        {"getAuthorityForViewport", DockBuilder_Node_GetAuthorityForViewport},
        {"isVisible", DockBuilder_Node_IsVisible},
        {"isFocused", DockBuilder_Node_IsFocused},
        {"hasCloseButton", DockBuilder_Node_HasCloseButton},
        {"hasWindowMenuButton", DockBuilder_Node_HasWindowMenuButton},
        {"enableCloseButton", DockBuilder_Node_EnableCloseButton},
        {"isCloseButtonEnable", DockBuilder_Node_IsCloseButtonEnable},
        {"wantCloseAll", DockBuilder_Node_WantCloseAll},
        {"wantLockSizeOnce", DockBuilder_Node_WantLockSizeOnce},
        {"wantMouseMove", DockBuilder_Node_WantMouseMove},
        {"wantHiddenTabBarUpdate", DockBuilder_Node_WantHiddenTabBarUpdate},
        {"wantHiddenTabBarToggle", DockBuilder_Node_WantHiddenTabBarToggle},
        {"isMarkedForPosSizeWrite", DockBuilder_Node_MarkedForPosSizeWrite},

        {"isRootNode", DockBuilder_Node_IsRootNode},
        {"isDockSpace", DockBuilder_Node_IsDockSpace},
        {"isFloatingNode", DockBuilder_Node_IsFloatingNode},
        {"isCentralNode", DockBuilder_Node_IsCentralNode},
        {"isHiddenTabBar", DockBuilder_Node_IsHiddenTabBar},
        {"isNoTabBar", DockBuilder_Node_IsNoTabBar},
        {"isSplitNode", DockBuilder_Node_IsSplitNode},
        {"isLeafNode", DockBuilder_Node_IsLeafNode},
        {"isEmpty", DockBuilder_Node_IsEmpty},
        {"getMergedFlags", DockBuilder_Node_GetMergedFlags},
        {"rect", DockBuilder_Node_Rect},
        {NULL, NULL}
    };
    binder.createClass("ImGuiDockNode", 0, NULL, NULL, imguiDockNodeFunctionList);

    const luaL_Reg imguiTabBarFunctionList[] = {
        {"getTabs", TabBar_GetTabs},
        {"getTab", TabBar_GetTab},
        {"getTabCount", TabBar_GetTabCount},
        {"getFlags", TabBar_GetFlags},
        {"getID", TabBar_GetID},
        {"getSelectedTabId", TabBar_GetSelectedTabId},
        {"getNextSelectedTabId", TabBar_GetNextSelectedTabId},
        {"getVisibleTabId", TabBar_GetVisibleTabId},
        {"getCurrFrameVisible", TabBar_GetCurrFrameVisible},
        {"getPrevFrameVisible", TabBar_GetPrevFrameVisible},
        {"getBarRect", TabBar_GetBarRect},
        {"getCurrTabsContentsHeight", TabBar_GetCurrTabsContentsHeight},
        {"getPrevTabsContentsHeight", TabBar_GetPrevTabsContentsHeight},
        {"getWidthAllTabs", TabBar_GetWidthAllTabs},
        {"getWidthAllTabsIdeal", TabBar_GetWidthAllTabsIdeal},
        {"getScrollingAnim", TabBar_GetScrollingAnim},
        {"getScrollingTarget", TabBar_GetScrollingTarget},
        {"getScrollingTargetDistToVisibility", TabBar_GetScrollingTargetDistToVisibility},
        {"getScrollingSpeed", TabBar_GetScrollingSpeed},
        {"getScrollingRectMinX", TabBar_GetScrollingRectMinX},
        {"getScrollingRectMaxX", TabBar_GetScrollingRectMaxX},
        {"getReorderRequestTabId", TabBar_GetReorderRequestTabId},
        {"getReorderRequestDir", TabBar_GetReorderRequestDir},
        {"getBeginCount", TabBar_GetBeginCount},
        {"wantLayout", TabBar_WantLayout},
        {"visibleTabWasSubmitted", TabBar_VisibleTabWasSubmitted},
        {"getTabsAddedNew", TabBar_TabsAddedNew},
        {"getTabsActiveCount", TabBar_GetTabsActiveCount},
        {"getLastTabItemIdx", TabBar_GetLastTabItemIdx},
        {"getItemSpacingY", TabBar_GetItemSpacingY},
        {"getFramePadding", TabBar_GetFramePadding},
        {"getBackupCursorPos", TabBar_GetBackupCursorPos},
        {"getTabsNames", TabBar_GetTabsNames},
        {"getTabOrder", TabBar_GetTabOrder},
        {"getTabName", TabBar_GetTabName},
        {NULL, NULL}
    };
    binder.createClass("ImGuiTabBar", 0, NULL, NULL, imguiTabBarFunctionList);

    const luaL_Reg imguiTabItemFunctionList[] = {
        {"getID", TabItem_GetID},
        {"getFlags", TabItem_GetFlags},
        {"getLastFrameVisible", TabItem_GetLastFrameVisible},
        {"getLastFrameSelected", TabItem_GetLastFrameSelected},
        {"getOffset", TabItem_GetOffset},
        {"getWidth", TabItem_GetWidth},
        {"getContentWidth", TabItem_GetContentWidth},
        {"getNameOffset", TabItem_GetNameOffset},
        {"getBeginOrder", TabItem_GetBeginOrder},
        {"getIndexDuringLayout", TabItem_GetIndexDuringLayout},
        {"wantClose", TabItem_WantClose},
        {NULL, NULL}
    };
    binder.createClass("ImGuiTabItem", 0, NULL, NULL, imguiTabItemFunctionList);
#endif
#ifdef IS_BETA_BUILD
    const luaL_Reg imguiNodeEditorFunctionList[] = {
        //{"getCurrentEditor", ED_GetCurrentEditor},
        //{"createEditor", ED_CreateEditor},
        //{"destroyEditor", ED_DestroyEditor},
        {"getStyle", ED_GetStyle},
        {"getStyleColorName", ED_GetStyleColorName},
        {"pushStyleColor", ED_PushStyleColor},
        {"popStyleColor", ED_PopStyleColor},
        {"pushStyleVar", ED_PushStyleVar},
        {"pushStyleVar", ED_PushStyleVar},
        {"pushStyleVar", ED_PushStyleVar},
        {"popStyleVar", ED_PopStyleVar},
        {"beginEditor", ED_Begin},
        {"endEditor", ED_End},
        {"beginNode", ED_BeginNode},
        {"beginPin", ED_BeginPin},
        {"pinRect", ED_PinRect},
        {"pinPivotRect", ED_PinPivotRect},
        {"pinPivotSize", ED_PinPivotSize},
        {"pinPivotScale", ED_PinPivotScale},
        {"pinPivotAlignment", ED_PinPivotAlignment},
        {"endPin", ED_EndPin},
        {"group", ED_Group},
        {"endNode", ED_EndNode},
        {"beginGroupHint", ED_BeginGroupHint},
        {"endGroupHint", ED_EndGroupHint},
        {"getGroupMin", ED_GetGroupMin},
        {"getGroupMax", ED_GetGroupMax},
        {"getHintForegroundDrawList", ED_GetHintForegroundDrawList},
        {"getHintBackgroundDrawList", ED_GetHintBackgroundDrawList},
        {"getNodeBackgroundDrawList", ED_GetNodeBackgroundDrawList},
        {"link", ED_Link},
        {"flow", ED_Flow},
        {"beginCreate", ED_BeginCreate},
        {"queryNewLink", ED_QueryNewLink},
        {"queryNewLink", ED_QueryNewLink},
        {"queryNewNode", ED_QueryNewNode},
        {"queryNewNode", ED_QueryNewNode},
        {"acceptNewItem", ED_AcceptNewItem},
        {"acceptNewItem", ED_AcceptNewItem},
        {"rejectNewItem", ED_RejectNewItem},
        {"rejectNewItem", ED_RejectNewItem},
        {"endCreate", ED_EndCreate},
        {"beginDelete", ED_BeginDelete},
        {"queryDeletedLink", ED_QueryDeletedLink},
        {"queryDeletedNode", ED_QueryDeletedNode},
        {"acceptDeletedItem", ED_AcceptDeletedItem},
        {"rejectDeletedItem", ED_RejectDeletedItem},
        {"endDelete", ED_EndDelete},
        {"setNodePosition", ED_SetNodePosition},
        {"getNodePosition", ED_GetNodePosition},
        {"getNodeSize", ED_GetNodeSize},
        {"centerNodeOnScreen", ED_CenterNodeOnScreen},
        {"restoreNodeState", ED_RestoreNodeState},
        {"suspend", ED_Suspend},
        {"resume", ED_Resume},
        {"isSuspended", ED_IsSuspended},
        {"isActive", ED_IsActive},
        {"hasSelectionChanged", ED_HasSelectionChanged},
        {"getSelectedObjectCount", ED_GetSelectedObjectCount},
        {"getSelectedNodes", ED_GetSelectedNodes},
        {"getSelectedLinks", ED_GetSelectedLinks},
        {"clearSelection", ED_ClearSelection},
        {"selectNode", ED_SelectNode},
        {"selectLink", ED_SelectLink},
        {"deselectNode", ED_DeselectNode},
        {"deselectLink", ED_DeselectLink},
        {"deleteNode", ED_DeleteNode},
        {"deleteLink", ED_DeleteLink},
        {"navigateToContent", ED_NavigateToContent},
        {"navigateToSelection", ED_NavigateToSelection},
        {"showNodeContextMenu", ED_ShowNodeContextMenu},
        {"showPinContextMenu", ED_ShowPinContextMenu},
        {"showLinkContextMenu", ED_ShowLinkContextMenu},
        {"showBackgroundContextMenu", ED_ShowBackgroundContextMenu},
        {"enableShortcuts", ED_EnableShortcuts},
        {"areShortcutsEnabled", ED_AreShortcutsEnabled},
        {"beginShortcut", ED_BeginShortcut},
        {"acceptCut", ED_AcceptCut},
        {"acceptCopy", ED_AcceptCopy},
        {"acceptPaste", ED_AcceptPaste},
        {"acceptDuplicate", ED_AcceptDuplicate},
        {"acceptCreateNode", ED_AcceptCreateNode},
        {"getActionContextSize", ED_GetActionContextSize},
        {"getActionContextNodes", ED_GetActionContextNodes},
        {"getActionContextLinks", ED_GetActionContextLinks},
        {"endShortcut", ED_EndShortcut},
        {"getCurrentZoom", ED_GetCurrentZoom},
        {"getDoubleClickedNode", ED_GetDoubleClickedNode},
        {"getDoubleClickedPin", ED_GetDoubleClickedPin},
        {"getDoubleClickedLink", ED_GetDoubleClickedLink},
        {"isBackgroundClicked", ED_IsBackgroundClicked},
        {"isBackgroundDoubleClicked", ED_IsBackgroundDoubleClicked},
        {"pinHadAnyLinks", ED_PinHadAnyLinks},
        {"getScreenSize", ED_GetScreenSize},
        {"screenToCanvas", ED_ScreenToCanvas},
        {"canvasToScreen", ED_CanvasToScreen},
        {NULL, NULL},
    };
    binder.createClass("ImGuiNodeEditor", 0, initNodeEditor, destroyNodeEditor, imguiNodeEditorFunctionList);

    const luaL_Reg imguiEDStyleFunctionsList[] = {
        {"getNodePadding", ED_StyleGetNodePadding},
        {"setNodePadding", ED_StyleSetNodePadding},
        {"getNodeRounding", ED_StyleGetNodeRounding},
        {"setNodeRounding", ED_StyleSetNodeRounding},
        {"getNodeBorderWidth", ED_StyleGetNodeBorderWidth},
        {"setNodeBorderWidth", ED_StyleSetNodeBorderWidth},
        {"getHoveredNodeBorderWidth", ED_StyleGetHoveredNodeBorderWidth},
        {"setHoveredNodeBorderWidth", ED_StyleSetHoveredNodeBorderWidth},
        {"getSelectedNodeBorderWidth", ED_StyleGetSelectedNodeBorderWidth},
        {"setSelectedNodeBorderWidth", ED_StyleSetSelectedNodeBorderWidth},
        {"getPinRounding", ED_StyleGetPinRounding},
        {"setPinRounding", ED_StyleSetPinRounding},
        {"getPinBorderWidth", ED_StyleGetPinBorderWidth},
        {"setPinBorderWidth", ED_StyleSetPinBorderWidth},
        {"getLinkStrength", ED_StyleGetLinkStrength},
        {"setLinkStrength", ED_StyleSetLinkStrength},
        {"getSourceDirection", ED_StyleGetSourceDirection},
        {"setSourceDirection", ED_StyleSetSourceDirection},
        {"getTargetDirection", ED_StyleGetTargetDirection},
        {"setTargetDirection", ED_StyleSetTargetDirection},
        {"getScrollDuration", ED_StyleGetScrollDuration},
        {"setScrollDuration", ED_StyleSetScrollDuration},
        {"getFlowMarkerDistance", ED_StyleGetFlowMarkerDistance},
        {"setFlowMarkerDistance", ED_StyleSetFlowMarkerDistance},
        {"getFlowSpeed", ED_StyleGetFlowSpeed},
        {"setFlowSpeed", ED_StyleSetFlowSpeed},
        {"getFlowDuration", ED_StyleGetFlowDuration},
        {"setFlowDuration", ED_StyleSetFlowDuration},
        {"getPivotAlignment", ED_StyleGetPivotAlignment},
        {"setPivotAlignment", ED_StyleSetPivotAlignment},
        {"getPivotSize", ED_StyleGetPivotSize},
        {"setPivotSize", ED_StyleSetPivotSize},
        {"getPivotScale", ED_StyleGetPivotScale},
        {"setPivotScale", ED_StyleSetPivotScale},
        {"getPinCorners", ED_StyleGetPinCorners},
        {"setPinCorners", ED_StyleSetPinCorners},
        {"getPinRadius", ED_StyleGetPinRadius},
        {"setPinRadius", ED_StyleSetPinRadius},
        {"getPinArrowSize", ED_StyleGetPinArrowSize},
        {"setPinArrowSize", ED_StyleSetPinArrowSize},
        {"getPinArrowWidth", ED_StyleGetPinArrowWidth},
        {"setPinArrowWidth", ED_StyleSetPinArrowWidth},
        {"getGroupRounding", ED_StyleGetGroupRounding},
        {"setGroupRounding", ED_StyleSetGroupRounding},
        {"getGroupBorderWidth", ED_StyleGetGroupBorderWidth},
        {"setGroupBorderWidth", ED_StyleSetGroupBorderWidth},
        {"getColor", ED_StyleGetColor},
        {"setColor", ED_StyleSetColor},
        {NULL, NULL}
    };
    binder.createClass("ImGuiEDStyle", 0, NULL, NULL, imguiEDStyleFunctionsList);

#endif
    const luaL_Reg imguiPayloadFunctionsList[] = {
        {"getNumData", Payload_GetNumberData},
        {"getStrData", Payload_GetStringData},
        {"clear", Payload_Clear},
        {"getDataSize", Payload_GetDataSize},
        {"isDataType", Payload_IsDataType},
        {"isPreview", Payload_IsPreview},
        {"isDelivery", Payload_IsDelivery},
        {NULL, NULL}
    };
    binder.createClass("ImGuiPayload", 0, NULL, NULL, imguiPayloadFunctionsList);

    const luaL_Reg imguiFunctionList[] =
    {
#ifdef IS_BETA_BUILD
        {"setCurrentEditor", ED_SetCurrentEditor},
#endif
        {"setAutoUpdateCursor", SetAutoUpdateCursor},
        {"getAutoUpdateCursor", GetAutoUpdateCursor},
        {"setResetTouchPosOnEnd", SetResetTouchPosOnEnd},
        {"getResetTouchPosOnEnd", GetResetTouchPosOnEnd},

        // Fonts API
        {"pushFont", Fonts_PushFont},
        {"popFont", Fonts_PopFont},

        {"setStyleColor", Style_old_SetColor}, // Backward capability

        // Draw list
        {"getStyle", GetStyle},
        {"getWindowDrawList", GetWindowDrawList},
        {"getBackgroundDrawList", GetBackgroundDrawList},
        {"getForegroundDrawList", GetForegroundDrawList},
        {"getIO", GetIO},

        /////////////////////////////////////////////////////////////////////////////// Inputs +

        /// Mouse
        {"onMouseHover", MouseHover},
        {"onMouseMove", MouseMove},
        {"onMouseDown", MouseDown},
        {"onMouseUp", MouseUp},
        {"onMouseWheel", MouseWheel},

        /// Touch
        {"onTouchCancel", TouchCancel},
        {"onTouchMove", TouchMove},
        {"onTouchBegin", TouchBegin},
        {"onTouchEnd", TouchEnd},

        /// Keyboard
        {"onKeyUp", KeyUp},
        {"onKeyDown", KeyDown},
        {"onKeyChar", KeyChar},

        /////////////////////////////////////////////////////////////////////////////// Inputs -

        // Colors
        {"colorConvertHEXtoRGB", ColorConvertHEXtoRGB},
        {"colorConvertRGBtoHEX", ColorConvertRGBtoHEX},
        {"colorConvertRGBtoHSV", ColorConvertRGBtoHSV},
        {"colorConvertHSVtoRGB", ColorConvertHSVtoRGB},

        // Style themes
        {"setDarkStyle", StyleDark},
        {"setLightStyle", StyleLight},
        {"setClassicStyle", StyleClassic},

        // Childs
        {"beginChild", BeginChild},
        {"endChild", EndChild},

        {"isWindowAppearing", IsWindowAppearing},
        {"isWindowCollapsed", IsWindowCollapsed},
        {"isWindowFocused", IsWindowFocused},
        {"isWindowHovered", IsWindowHovered},
        {"getWindowPos", GetWindowPos},
        {"getWindowSize", GetWindowSize},
        {"getWindowWidth", GetWindowWidth},
        {"getWindowHeight", GetWindowHeight},
        {"getWindowBounds", GetWindowBounds},

        {"setNextWindowPos", SetNextWindowPos},
        {"setNextWindowSize", SetNextWindowSize},
        {"setNextWindowSizeConstraints", SetNextWindowSizeConstraints},
        {"setNextWindowContentSize", SetNextWindowContentSize},
        {"setNextWindowCollapsed", SetNextWindowCollapsed},
        {"setNextWindowFocus", SetNextWindowFocus},
        {"setNextWindowBgAlpha", SetNextWindowBgAlpha},
        {"setWindowPos", SetWindowPos},
        {"setWindowSize", SetWindowSize},
        {"setWindowCollapsed", SetWindowCollapsed},
        {"setWindowFocus", SetWindowFocus},
        {"setWindowFontScale", SetWindowFontScale},

        {"getContentRegionMax", GetContentRegionMax},
        {"getContentRegionAvail", GetContentRegionAvail},
        {"getWindowContentRegionMin", GetWindowContentRegionMin},
        {"getWindowContentRegionMax", GetWindowContentRegionMax},
        {"getWindowContentRegionWidth", GetWindowContentRegionWidth},

        {"getScrollX", GetScrollX},
        {"getScrollY", GetScrollY},
        {"getScrollMaxX", GetScrollMaxX},
        {"getScrollMaxY", GetScrollMaxY},
        {"setScrollX", SetScrollX},
        {"setScrollY", SetScrollY},
        {"setScrollHereX", SetScrollHereX},
        {"setScrollHereY", SetScrollHereY},
        {"setScrollFromPosX", SetScrollFromPosX},
        {"setScrollFromPosY", SetScrollFromPosY},

        {"pushStyleColor", PushStyleColor},
        {"popStyleColor", PopStyleColor},
        {"pushStyleVar", PushStyleVar},
        {"popStyleVar", PopStyleVar},
        {"getFontSize", GetFontSize},

        {"pushItemWidth", PushItemWidth},
        {"popItemWidth", PopItemWidth},
        {"pushItemFlag", PushItemFlag},
        {"popItemFlag", PopItemFlag},
        {"setNextItemWidth", SetNextItemWidth},
        {"calcItemWidth", CalcItemWidth},
        {"pushTextWrapPos", PushTextWrapPos},
        {"popTextWrapPos", PopTextWrapPos},
        {"pushAllowKeyboardFocus", PushAllowKeyboardFocus},
        {"popAllowKeyboardFocus", PopAllowKeyboardFocus},
        {"pushButtonRepeat", PushButtonRepeat},
        {"popButtonRepeat", PopButtonRepeat},

        {"separator", Separator},
        {"sameLine", SameLine},
        {"newLine", NewLine},
        {"spacing", Spacing},
        {"dummy", Dummy},
        {"indent", Indent},
        {"unindent", Unindent},
        {"beginGroup", BeginGroup},
        {"endGroup", EndGroup},

        {"getCursorPos", GetCursorPos},
        {"getCursorPosX", GetCursorPosX},
        {"getCursorPosY", GetCursorPosY},
        {"setCursorPos", SetCursorPos},
        {"setCursorPosX", SetCursorPosX},
        {"setCursorPosY", SetCursorPosY},
        {"getCursorStartPos", GetCursorStartPos},
        {"getCursorScreenPos", GetCursorScreenPos},
        {"setCursorScreenPos", SetCursorScreenPos},
        {"alignTextToFramePadding", AlignTextToFramePadding},
        {"getTextLineHeight", GetTextLineHeight},
        {"getTextLineHeightWithSpacing", GetTextLineHeightWithSpacing},
        {"getFrameHeight", GetFrameHeight},
        {"getFrameHeightWithSpacing", GetFrameHeightWithSpacing},

        {"pushID", PushID},
        {"popID", PopID},
        {"getID", GetID},

        {"text", Text},
        {"textColored", TextColored},
        {"textDisabled", TextDisabled},
        {"textWrapped", TextWrapped},
        {"labelText", LabelText},
        {"bulletText", BulletText},

        {"button", Button},
        {"smallButton", SmallButton},
        {"invisibleButton", InvisibleButton},
        {"arrowButton", ArrowButton},

        /// Images +

        {"image", Image},
        {"imageFilled", ImageFilled},
        {"imageButton", ImageButton},
        {"imageButtonWithText", ImageButtonWithText},

        {"scaledImage", ScaledImage},
        {"scaledImageFilled", ScaledImageFilled},
        {"scaledImageButton", ScaledImageButton},
        {"scaledImageButtonWithText", ScaledImageButtonWithText},

        /// Images -

        {"checkbox", Checkbox},
        {"checkboxFlags", CheckboxFlags},
        {"radioButton", RadioButton},
        {"progressBar", ProgressBar},
        {"bullet", Bullet},
        {"beginCombo", BeginCombo},
        {"endCombo", EndCombo},
        {"combo", Combo},

        {"dragFloat", DragFloat},
        {"dragFloat2", DragFloat2},
        {"dragFloat3", DragFloat3},
        {"dragFloat4", DragFloat4},
        {"dragFloatRange2", DragFloatRange2},

        {"dragInt", DragInt},
        {"dragInt2", DragInt2},
        {"dragInt3", DragInt3},
        {"dragInt4", DragInt4},
        {"dragIntRange2", DragIntRange2},
        {"dragScalar", DragScalar},

        {"sliderFloat", SliderFloat},
        {"sliderFloat2", SliderFloat2},
        {"sliderFloat3", SliderFloat3},
        {"sliderFloat4", SliderFloat4},
        {"sliderAngle", SliderAngle},
        {"sliderInt", SliderInt},
        {"sliderInt2", SliderInt2},
        {"sliderInt3", SliderInt3},
        {"sliderInt4", SliderInt4},
        {"sliderScalar", SliderScalar},
        {"vSliderFloat", VSliderFloat},
        {"vSliderInt", VSliderInt},
        {"vSliderScalar", VSliderScalar},

        {"filledSliderFloat", FilledSliderFloat},
        {"filledSliderFloat2", FilledSliderFloat2},
        {"filledSliderFloat3", FilledSliderFloat3},
        {"filledSliderFloat4", FilledSliderFloat4},
        {"filledSliderAngle", FilledSliderAngle},
        {"filledSliderInt", FilledSliderInt},
        {"filledSliderInt2", FilledSliderInt2},
        {"filledSliderInt3", FilledSliderInt3},
        {"filledSliderInt4", FilledSliderInt4},
        {"filledSliderScalar", FilledSliderScalar},
        {"vFilledSliderFloat", VFilledSliderFloat},
        {"vFilledSliderInt", VFilledSliderInt},
        {"vFilledSliderScalar", VFilledSliderScalar},

        {"inputText", InputText},
        {"inputTextMultiline", InputTextMultiline},
        {"inputTextWithHint", InputTextWithHint},
        {"inputFloat", InputFloat},
        {"inputFloat2", InputFloat2},
        {"inputFloat3", InputFloat3},
        {"inputFloat4", InputFloat4},
        {"inputInt", InputInt},
        {"inputInt2", InputInt2},
        {"inputInt3", InputInt3},
        {"inputInt4", InputInt4},
        {"inputDouble", InputDouble},
        {"inputScalar", InputScalar},

        {"colorEdit3", ColorEdit3},
        {"colorEdit4", ColorEdit4},
        {"colorPicker3", ColorPicker3},
        {"colorPicker4", ColorPicker4},
        {"colorButton", ColorButton},
        {"setColorEditOptions", SetColorEditOptions},

        {"treeNode", TreeNode},
        {"treeNodeEx", TreeNodeEx},
        {"treePush", TreePush},
        {"treePop", TreePop},
        {"getTreeNodeToLabelSpacing", GetTreeNodeToLabelSpacing},
        {"collapsingHeader", CollapsingHeader},
        {"setNextItemOpen", SetNextItemOpen},
        {"selectable", Selectable},

        {"listBox", ListBox},
        {"listBoxHeader", ListBoxHeader},
        {"listBoxFooter", ListBoxFooter},
        {"plotLines", PlotLines},
        {"plotHistogram", PlotHistogram},
        {"value", Value},

        {"beginMenuBar", BeginMenuBar },
        {"endMenuBar", EndMenuBar },
        {"beginMainMenuBar", BeginMainMenuBar },
        {"endMainMenuBar", EndMainMenuBar },
        {"beginMenu", BeginMenu },
        {"endMenu", EndMenu },
        {"menuItem", MenuItem },
        {"menuItemWithShortcut", MenuItemWithShortcut },
        {"beginTooltip", BeginTooltip },
        {"endTooltip", EndTooltip },
        {"setTooltip", SetTooltip },
        {"beginPopup", BeginPopup},
        {"beginPopupModal", BeginPopupModal },
        {"endPopup", EndPopup },
        {"openPopup", OpenPopup },
        {"openPopupContextItem", OpenPopupContextItem },
        {"closeCurrentPopup", CloseCurrentPopup },
        {"beginPopupContextItem", BeginPopupContextItem },
        {"beginPopupContextWindow", BeginPopupContextWindow },
        {"beginPopupContextVoid", BeginPopupContextVoid },
        {"isPopupOpen", IsPopupOpen },

        {"columns", Columns},
        {"nextColumn", NextColumn},
        {"getColumnIndex", GetColumnIndex},
        {"getColumnWidth", GetColumnWidth},
        {"setColumnWidth", SetColumnWidth},
        {"getColumnOffset", GetColumnOffset},
        {"setColumnOffset", SetColumnOffset},
        {"getColumnsCount", GetColumnsCount},

        {"beginTabBar", BeginTabBar},
        {"endTabBar", EndTabBar},
        {"beginTabItem", BeginTabItem},
        {"endTabItem", EndTabItem},
        {"tabItemButton", TabItemButton},
        {"setTabItemClosed", SetTabItemClosed},

        {"logToTTY", LogToTTY},
        {"logToFile", LogToFile},
        {"logToClipboard", LogToClipboard},
        {"logFinish", LogFinish},
        {"logButtons", LogButtons},
        {"logText", LogText},

        {"pushClipRect", PushClipRect},
        {"popClipRect", PopClipRect},

        {"setItemDefaultFocus", SetItemDefaultFocus},
        {"setKeyboardFocusHere", SetKeyboardFocusHere},

        {"isItemHovered", IsItemHovered},
        {"isItemActive", IsItemActive},
        {"isItemFocused", IsItemFocused},
        {"isItemClicked", IsItemClicked},
        {"isItemVisible", IsItemVisible},
        {"isItemEdited", IsItemEdited},
        {"isItemActivated", IsItemActivated},
        {"isItemDeactivated", IsItemDeactivated},
        {"isItemDeactivatedAfterEdit", IsItemDeactivatedAfterEdit},
        {"isItemToggledOpen", IsItemToggledOpen},
        {"isAnyItemHovered", IsAnyItemHovered},
        {"isAnyItemActive", IsAnyItemActive},
        {"isAnyItemFocused", IsAnyItemFocused},
        {"getItemRectMin", GetItemRectMin},
        {"getItemRectMax", GetItemRectMax},
        {"getItemRectSize", GetItemRectSize},
        {"setItemAllowOverlap", SetItemAllowOverlap},

        // Miscellaneous Utilities
        {"isRectVisible", IsRectVisible},
        {"getTime", GetTime},
        {"getFrameCount", GetFrameCount},
        {"getStyleColorName", GetStyleColorName},
        {"getStyleColor", GetStyleColor},
        {"calcListClipping", CalcListClipping},
        {"beginChildFrame", BeginChildFrame},
        {"endChildFrame", EndChildFrame},

        // Text Utilities
        {"calcTextSize", CalcTextSize},

        // Inputs Utilities: Keyboard
        {"getKeyIndex", GetKeyIndex},
        {"isKeyDown", IsKeyDown},
        {"isKeyPressed", IsKeyPressed},
        {"isKeyReleased", IsKeyReleased},
        {"getKeyPressedAmount", GetKeyPressedAmount},
        {"captureKeyboardFromApp", CaptureKeyboardFromApp},

        // Inputs Utilities: Mouse
        {"isMouseDown", IsMouseDown},
        {"isMouseClicked", IsMouseClicked},
        {"isMouseReleased", IsMouseReleased},
        {"isMouseDoubleClicked", IsMouseDoubleClicked},
        {"isMouseHoveringRect", IsMouseHoveringRect},
        {"isMousePosValid", IsMousePosValid},
        {"isAnyMouseDown", IsAnyMouseDown},
        {"getMousePos", GetMousePos},
        {"getMousePosOnOpeningCurrentPopup", GetMousePosOnOpeningCurrentPopup},
        {"isMouseDragging", IsMouseDragging},
        {"getMouseDragDelta", GetMouseDragDelta},
        {"resetMouseDragDelta", ResetMouseDragDelta},
        {"getMouseCursor", GetMouseCursor},
        {"setMouseCursor", SetMouseCursor},
        {"captureMouseFromApp", CaptureMouseFromApp},

        // Windows
        {"beginWindow", Begin},
        {"endWindow", End},
        {"beginFullScreenWindow", BeginFullScreenWindow},

        // Render
        {"newFrame", NewFrame},
        {"render", Render},
        {"endFrame", EndFrame},

        // Demos
        {"showUserGuide", ShowUserGuide},
        {"showDemoWindow", ShowDemoWindow},
        {"showAboutWindow", ShowAboutWindow},
        {"showStyleEditor", ShowStyleEditor},
        {"showFontSelector", ShowFontSelector},
        {"showMetricsWindow", ShowMetricsWindow},
        {"showStyleSelector", ShowStyleSelector},
        {"showLuaStyleEditor", ShowLuaStyleEditor},

        // Logs
        {"showLog", ShowLog},
        {"writeLog", WriteLog},

        // Drag & Drop
        {"beginDragDropSource", BeginDragDropSource},
        {"setNumDragDropPayload", SetNumberDragDropPayload},
        {"setStrDragDropPayload", SetStringDragDropPayload},
        {"endDragDropSource", EndDragDropSource},
        {"beginDragDropTarget", BeginDragDropTarget},
        {"acceptDragDropPayload", AcceptDragDropPayload},
        {"endDragDropTarget", EndDragDropTarget},
        {"getDragDropPayload", GetDragDropPayload},

#ifdef IS_BETA_BUILD
        {"dockSpace", DockSpace},
        {"dockSpaceOverViewport", DockSpaceOverViewport},
        {"setNextWindowDockID", SetNextWindowDockID},
        {"getWindowDockID", GetWindowDockID},
        {"isWindowDocked", IsWindowDocked},

        {"dockBuilderDockWindow", DockBuilderDockWindow},
        {"dockBuilderGetNode", DockBuilderGetNode},
        {"dockBuilderCheckNode", DockBuilderCheckNode},
        {"dockBuilderSetNodePos", DockBuilderSetNodePos},
        {"dockBuilderSetNodeSize", DockBuilderSetNodeSize},
        {"dockBuilderAddNode", DockBuilderAddNode},
        {"dockBuilderRemoveNode", DockBuilderRemoveNode},
        {"dockBuilderRemoveNodeChildNodes", DockBuilderRemoveNodeChildNodes},
        {"dockBuilderRemoveNodeDockedWindows", DockBuilderRemoveNodeDockedWindows},
        {"dockBuilderSplitNode", DockBuilderSplitNode},
        //{"dockBuilderCopyNode", DockBuilderCopyNode},
        {"dockBuilderCopyWindowSettings", DockBuilderCopyWindowSettings},
        {"dockBuilderCopyDockSpace", DockBuilderCopyDockSpace},
        {"dockBuilderFinish", DockBuilderFinish},
#endif
        {NULL, NULL}
    };
    binder.createClass("ImGui", "Sprite", initImGui, destroyImGui, imguiFunctionList);
    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    bindEnums(L);

    lua_getglobal(L, "ImGui");
    lua_pushstring(L, ImGui::GetVersion());
    lua_setfield(L, -2, "_VERSION");
    lua_pop(L, 1);

    return 1;
}

}

static void g_initializePlugin(lua_State* L)
{
    ::L = L;

    resetStaticVars();

    giderosCursorMap[ImGuiMouseCursor_Hand]        = "pointingHand";
    giderosCursorMap[ImGuiMouseCursor_None]        = "blank";
    giderosCursorMap[ImGuiMouseCursor_Arrow]       = "arrow";
    giderosCursorMap[ImGuiMouseCursor_ResizeEW]    = "sizeHor";
    giderosCursorMap[ImGuiMouseCursor_ResizeNS]    = "sizeVer";
    giderosCursorMap[ImGuiMouseCursor_ResizeAll]   = "sizeAll";
    giderosCursorMap[ImGuiMouseCursor_TextInput]   = "IBeam";
    giderosCursorMap[ImGuiMouseCursor_NotAllowed]  = "forbidden";
    giderosCursorMap[ImGuiMouseCursor_ResizeNESW]  = "sizeBDiag";
    giderosCursorMap[ImGuiMouseCursor_ResizeNWSE]  = "sizeFDiag";

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, ImGui_impl::loader);
    lua_setfield(L, -2, PLUGIN_NAME);

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* _UNUSED(L)) { resetStaticVars(); }

#ifdef IS_BETA_BUILD
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", imgui_beta)
#else
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", Imgui)
#endif
