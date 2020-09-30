// regex: (\s\*)+\b

#define _UNUSED(n)
#define PLUGIN_NAME "ImGui"
#define CLASS_NAME "ImGui"
#define IO_CLASS_NAME "ImGuiIO"
#define FONT_ATLAS_CLASS_NAME "ImFontAtlas"
#define FONT_CLASS_NAME "ImFont"
#define DRAW_LIST_CLASS_NAME "ImDrawList"
#define STYLES_CLASS_NAME "ImGuiStyle"
#define DOCK_NODE_CLASS_NAME "ImGuiDockNode"

#include "gplugin.h"
#include "gfile.h"
#include "gstdio.h"
#include "ginput.h"

#include "application.h"
#include "luaapplication.h"
#include "lua.hpp"
#include "luautil.h"
#include "sprite.h"
#include "binder.h"
#include "mouseevent.h"
#include "keyboardevent.h"

#include "texturebase.h"
#include "bitmapdata.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static lua_State* L;
static Application* application;
static char keyWeak = ' ';

#define LUA_ASSERT(L, EXP, MSG) if (!(EXP)) { lua_pushfstring(L, MSG); lua_error(L); }
//#define LUA_ASSERT(L, EXP, MSG)  ((void)(_EXPR))

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui_src/imgui.h"
#include "imgui_src/imgui_internal.h"



////////////////////////////////////////////////////////////////////////////////
///
/// DEBUG TOOL
///
////////////////////////////////////////////////////////////////////////////////

static int dump_index = 0;

static void stackDump(lua_State* L)
{
    int i = lua_gettop(L);
    lua_getglobal(L, "print");
    lua_pushfstring(L, "----------------      %d      ----------------\n----------------  Stack Dump ----------------", dump_index);
    lua_call(L, 1, 0);
    while (i) {
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING:
            {
                lua_getglobal(L, "print");
                lua_pushfstring(L, "%d:`%s'", i, lua_tostring(L, i));
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
                lua_pushfstring(L, "%d: %g", i, lua_tonumber(L, i));
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

    dump_index++;
}

////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
////////////////////////////////////////////////////////////////////////////////


/*
static std::map<int, int> mouse_map =
{
    {GINPUT_LEFT_BUTTON, ImGuiMouseButton_Left},
    {GINPUT_RIGHT_BUTTON, ImGuiMouseButton_Right},
    {GINPUT_MIDDLE_BUTTON, ImGuiMouseButton_Middle},
};

static std::map<int, int> key_map =
{
    {GINPUT_ALT_MODIFIER, ImGuiKeyModFlags_Alt},
};
*/

struct MyTextureData
{
    void* texture;
    ImVec2 uv;

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

double getfield(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);  // get table[key]
    double result = lua_tonumber(L, -1);
    lua_pop(L, 1);  // remove number
    return result;
}

int getKeyboardModifiers(lua_State* L)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, "getKeyboardModifiers");
    lua_getglobal(L, "application");
    lua_call(L,1,1);
    int mod = luaL_checkinteger(L, -1);
    lua_pop(L, 2);

    return mod;
}

double getApplicationProperty(lua_State* L, const char* name)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, name);
    lua_getglobal(L, "application");
    lua_call(L,1,1);
    double value = luaL_checknumber(L, -1);
    lua_pop(L, 2);

    return value;
}

void setApplicationCursor(lua_State* L, const char* name)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, "set");
    lua_getglobal(L, "application");
    lua_pushstring(L, "cursor");
    lua_pushstring(L, name);
    lua_call(L, 3, 0);
    lua_pop(L, 2);
}

MyTextureData getTexture(lua_State* L, int idx = 1)
{
    Binder binder(L);

    if (binder.isInstanceOf("TextureBase", idx))
    {
        MyTextureData data;
        TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", idx));

        TextureData* gdata = textureBase->data;
        data.texture = (void*)gdata->gid;
        data.uv.x = (double)gdata->width / (double)gdata->exwidth;
        data.uv.y = (double)gdata->width / (double)gdata->exheight;
        return data;
    }
    else if (binder.isInstanceOf("TextureRegion", idx))
    {
        MyTextureData data;
        BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", idx));

        TextureData* gdata = bitmapData->texture()->data;
        data.texture = (void*)gdata->gid;
        data.uv.x = (double)gdata->width / (double)gdata->exwidth;
        data.uv.x = (double)gdata->width / (double)gdata->exheight;
        return data;
    }
    else
    {
        lua_pushstring(L, "Type mismatch. 'TextureBase' or 'TextureRegion' expected.");
        lua_error(L);
    }
}

int convertGiderosMouseButton(lua_State* L, int button)
{
    if (button <= 0)
    {
        lua_pushstring(L, "Button index must be > 0");
        lua_error(L);
    }

    return log2(button);
}

int luaL_optboolean(lua_State* L, int narg, int def)
{
    return lua_isboolean(L, narg) ? lua_toboolean(L, narg) : def;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void BindEnums(lua_State* L)
{
    lua_getglobal(L, CLASS_NAME);
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

    // ImGuiTabItemFlags
    lua_pushinteger(L, ImGuiTabItemFlags_SetSelected);                  lua_setfield(L, -2, "TabItemFlags_SetSelected");
    lua_pushinteger(L, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton); lua_setfield(L, -2, "TabItemFlags_NoCloseWithMiddleMouseButton");
    lua_pushinteger(L, ImGuiTabItemFlags_NoTooltip);                    lua_setfield(L, -2, "TabItemFlags_NoTooltip");
    lua_pushinteger(L, ImGuiTabItemFlags_None);                         lua_setfield(L, -2, "TabItemFlags_None");
    lua_pushinteger(L, ImGuiTabItemFlags_NoPushId);                     lua_setfield(L, -2, "TabItemFlags_NoPushId");
    lua_pushinteger(L, ImGuiTabItemFlags_UnsavedDocument);              lua_setfield(L, -2, "TabItemFlags_UnsavedDocument");

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

    // 1.78 *NEW*
    //ImGuiSliderFlags_

    lua_pushinteger(L, ImGuiSliderFlags_None);                          lua_setfield(L, -2, "SliderFlags_None");
    lua_pushinteger(L, ImGuiSliderFlags_ClampOnInput);                  lua_setfield(L, -2, "SliderFlags_ClampOnInput");
    lua_pushinteger(L, ImGuiSliderFlags_Logarithmic);                   lua_setfield(L, -2, "SliderFlags_Logarithmic");
    lua_pushinteger(L, ImGuiSliderFlags_NoRoundToFormat);               lua_setfield(L, -2, "SliderFlags_NoRoundToFormat");
    lua_pushinteger(L, ImGuiSliderFlags_NoInput);                       lua_setfield(L, -2, "SliderFlags_NoInput");

    // Custom enum
    lua_pushinteger(L, ImGuiInputTextFlags_NoBackground);               lua_setfield(L, -2, "InputTextFlags_NoBackground");

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

    lua_pushinteger(L, ImGuiDockNodeFlags_None);                        lua_setfield(L, -2, "DockNodeFlags_None");
    lua_pushinteger(L, ImGuiDockNodeFlags_KeepAliveOnly);               lua_setfield(L, -2, "DockNodeFlags_KeepAliveOnly");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoDockingInCentralNode);      lua_setfield(L, -2, "DockNodeFlags_NoDockingInCentralNode");
    lua_pushinteger(L, ImGuiDockNodeFlags_PassthruCentralNode);         lua_setfield(L, -2, "DockNodeFlags_PassthruCentralNode");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoSplit);                     lua_setfield(L, -2, "DockNodeFlags_NoSplit");
    lua_pushinteger(L, ImGuiDockNodeFlags_NoResize);                    lua_setfield(L, -2, "DockNodeFlags_NoResize");
    lua_pushinteger(L, ImGuiDockNodeFlags_AutoHideTabBar);              lua_setfield(L, -2, "DockNodeFlags_AutoHideTabBar");

    lua_pushinteger(L, ImGuiCol_DockingPreview);                        lua_setfield(L, -2, "Col_DockingPreview");
    lua_pushinteger(L, ImGuiCol_DockingEmptyBg);                        lua_setfield(L, -2, "Col_DockingEmptyBg");
#endif

    lua_pushinteger(L, ImGuiGlyphRanges_Default);                       lua_setfield(L, -2, "GlyphRanges_Default");
    lua_pushinteger(L, ImGuiGlyphRanges_Korean);                        lua_setfield(L, -2, "GlyphRanges_Korean");
    lua_pushinteger(L, ImGuiGlyphRanges_ChineseFull);                   lua_setfield(L, -2, "GlyphRanges_ChineseFull");
    lua_pushinteger(L, ImGuiGlyphRanges_ChineseSimplifiedCommon);       lua_setfield(L, -2, "GlyphRanges_ChineseSimplifiedCommon");
    lua_pushinteger(L, ImGuiGlyphRanges_Japanese);                      lua_setfield(L, -2, "GlyphRanges_Japanese");
    lua_pushinteger(L, ImGuiGlyphRanges_Cyrillic);                      lua_setfield(L, -2, "GlyphRanges_Cyrillic");
    lua_pushinteger(L, ImGuiGlyphRanges_Thai);                          lua_setfield(L, -2, "GlyphRanges_Thai");
    lua_pushinteger(L, ImGuiGlyphRanges_Vietnamese);                    lua_setfield(L, -2, "GlyphRanges_Vietnamese");
    lua_pop(L, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// EVENT LISTENER
///
/////////////////////////////////////////////////////////////////////////////////////////////

// called when using ImGui:mouseDown(e) / ImGui:mouseMove(e) / ImGui:mouseUp(e) / ImGui:mouseHover(e)
static ImVec2 getTranslatedMousePos(lua_State* L)
{
    Binder binder(L);
    SpriteProxy* sprite = static_cast<SpriteProxy*>(binder.getInstance(CLASS_NAME, 1));
    const Matrix4 mat = sprite->matrix().inverse();

    float event_x = getfield(L, "x");
    float event_y = getfield(L, "y");

    Vector3 v = mat * Vector3(event_x, event_y, 0.0f);

    return ImVec2(v.x, v.y);
}

// used by EventListener
static ImVec2 getTranslatedMousePos(SpriteProxy* sprite, float event_x, float event_y)
{
    const Matrix4 mat = sprite->matrix().inverse();

    Vector3 v = mat * Vector3(event_x, event_y, 0.0f);

    return ImVec2(v.x, v.y);
}

class EventListener : public EventDispatcher
{
public:
    EventListener(lua_State *L, SpriteProxy *proxy) :
        L(L),
        proxy(proxy)
    {
    }

    ~EventListener()
    {
    }

    void mouseDown(MouseEvent *event)
    {
        int button = convertGiderosMouseButton(L, event->button);

        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = getTranslatedMousePos(proxy, (float)event->x, (float)event->y);
        io.MouseDown[button] = true;
    }

    void mouseUp(MouseEvent *event)
    {
        int button = convertGiderosMouseButton(L, event->button);

        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = getTranslatedMousePos(proxy, (float)event->x, (float)event->y);
        io.MouseDown[button] = false;
    }

    void mouseHover(MouseEvent *event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = getTranslatedMousePos(proxy, (float)event->x, (float)event->y);
    }

    void mouseWheel(MouseEvent *event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel += event->wheel < 0 ? -1.0f : 1.0f;
        io.MousePos = getTranslatedMousePos(proxy, (float)event->x, (float)event->y);
    }

    void keyDown(KeyboardEvent *event)
    {
        int keyCode = event->keyCode;
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[keyCode] = true;

        int mod = getKeyboardModifiers(L);
        io.KeyAlt = (mod & GINPUT_ALT_MODIFIER) > 0;
        io.KeyCtrl = (mod & GINPUT_CTRL_MODIFIER) > 0;
        io.KeyShift = (mod & GINPUT_SHIFT_MODIFIER) > 0;
        io.KeySuper = (mod & GINPUT_META_MODIFIER) > 0;
    }

    void keyUp(KeyboardEvent *event)
    {
        int keyCode = event->keyCode;
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[keyCode] = false;

        int mod = getKeyboardModifiers(L);
        io.KeyAlt = (mod & GINPUT_ALT_MODIFIER) > 0;
        io.KeyCtrl = (mod & GINPUT_CTRL_MODIFIER) > 0;
        io.KeyShift = (mod & GINPUT_SHIFT_MODIFIER) > 0;
        io.KeySuper = (mod & GINPUT_META_MODIFIER) > 0;
    }

    void keyChar(KeyboardEvent *event)
    {
        std::string text = event->charCode;
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(text.c_str());
    }

    lua_State *L;
    SpriteProxy *proxy;
};

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// GidImGui
///
/////////////////////////////////////////////////////////////////////////////////////////////

class GidImGui
{
public:
    GidImGui(LuaApplication* application, lua_State* L);
    ~GidImGui();

    SpriteProxy* proxy;
    EventListener* eventListener;

    void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
    LuaApplication* application;
    VertexBuffer<Point2f> vertices;
    VertexBuffer<Point2f> texcoords;
    VertexBuffer<VColor> colors;
};

static void _Draw(void* c, const CurrentTransform&t, float sx, float sy, float ex, float ey)
{
    ((GidImGui* ) c)->doDraw(t, sx, sy, ex, ey);
}

static void _Destroy(void* c)
{
    delete ((GidImGui* ) c);
}

GidImGui::GidImGui(LuaApplication* application, lua_State* L)
{
    this->application = application;
    proxy = gtexture_get_spritefactory()->createProxy(application->getApplication(), this, _Draw, _Destroy);

    eventListener = new EventListener(L, proxy);
    proxy->addEventListener(MouseEvent::MOUSE_DOWN,  eventListener, &EventListener::mouseDown);
    proxy->addEventListener(MouseEvent::MOUSE_UP,    eventListener, &EventListener::mouseUp);
    proxy->addEventListener(MouseEvent::MOUSE_MOVE,  eventListener, &EventListener::mouseDown);
    proxy->addEventListener(MouseEvent::MOUSE_HOVER, eventListener, &EventListener::mouseHover);
    proxy->addEventListener(MouseEvent::MOUSE_WHEEL, eventListener, &EventListener::mouseWheel);

    proxy->addEventListener(KeyboardEvent::KEY_DOWN, eventListener, &EventListener::keyDown);
    proxy->addEventListener(KeyboardEvent::KEY_UP,   eventListener, &EventListener::keyUp);
    proxy->addEventListener(KeyboardEvent::KEY_CHAR, eventListener, &EventListener::keyChar);
}

GidImGui::~GidImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->TexID = 0;
    ImGui::DestroyContext();
    proxy->removeEventListeners();
    delete proxy;
    delete eventListener;
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

              engine->pushClip((int)(pcmd->ClipRect.x - pos.x), (int)(pcmd->ClipRect.y - pos.y), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
              shp->drawElements(ShaderProgram::Triangles, pcmd->ElemCount,ShaderProgram::DUSHORT, idx_buffer, true, NULL);
              engine->popClip();
         }
         idx_buffer += pcmd->ElemCount;
       }

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

int initImGui(lua_State* L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    ::application = application->getApplication();

    // init ImGui itself
    ImGui::CreateContext();

    // Setup style theme
    ImGui::StyleColorsDark();

    // Create font
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = "Gideros";
    io.BackendRendererName = "Gideros";

    // Setup display size
    int swidth = 0;
    int sheight = 0;

    if (lua_gettop(L) > 0)
    {
        swidth = luaL_checkinteger(L, 1);
        sheight = luaL_checkinteger(L, 2);
    }
    else
    {
        swidth = (int)getApplicationProperty(L, "getContentWidth");
        sheight = (int)getApplicationProperty(L, "getContentHeight");
    }

    io.DisplaySize.x = swidth;
    io.DisplaySize.y = sheight;

    // Keyboard map
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GINPUT_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GINPUT_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GINPUT_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GINPUT_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GINPUT_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GINPUT_KEY_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = GINPUT_KEY_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = GINPUT_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GINPUT_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GINPUT_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GINPUT_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GINPUT_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GINPUT_KEY_ESC;
    io.KeyMap[ImGuiKey_Insert] = GINPUT_KEY_INSERT;
    io.KeyMap[ImGuiKey_A] = GINPUT_KEY_A;
    io.KeyMap[ImGuiKey_C] = GINPUT_KEY_C;
    io.KeyMap[ImGuiKey_V] = GINPUT_KEY_V;
    io.KeyMap[ImGuiKey_X] = GINPUT_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GINPUT_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GINPUT_KEY_Z;

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_id texture = gtexture_create(width, height, GTEXTURE_RGBA, GTEXTURE_UNSIGNED_BYTE, GTEXTURE_CLAMP, GTEXTURE_LINEAR, pixels, NULL, 0);
    io.Fonts->TexID = (void* )texture;

    Binder binder(L);
    GidImGui* imgui = new GidImGui(application, L);
    binder.pushInstance(CLASS_NAME, imgui->proxy);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, imgui);
    lua_pop(L, 1);

    return 1;
}

int destroyImGui(lua_State* _UNUSED(L))
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->TexID = 0;
    ImGui::DestroyContext();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// BINDINGS.
///
////////////////////////////////////////////////////////////////////////////////

/// MOUSE INPUTS

int ImGui_impl_MouseHover(lua_State* L)
{

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = getTranslatedMousePos(L);

    return 0;
}

int ImGui_impl_MouseMove(lua_State* L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = getTranslatedMousePos(L);
    io.MouseDown[button] = true;

    return 0;
}

int ImGui_impl_MouseDown(lua_State* L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = getTranslatedMousePos(L);
    io.MouseDown[button] = true;

    return 0;
}

int ImGui_impl_MouseUp(lua_State* L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = getTranslatedMousePos(L);
    io.MouseDown[button] = false;

    return 0;
}

int ImGui_impl_MouseWheel(lua_State* L)
{
    double wheel = getfield(L, "wheel");

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += wheel < 0 ? -1.0f : 1.0f;
    io.MousePos = getTranslatedMousePos(L);

    return 0;
}


/// KEYBOARD INPUTS

int ImGui_impl_KeyUp(lua_State* L)
{
    int keyCode = (int)getfield(L, "keyCode");

    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[keyCode] = false;

    int mod = getKeyboardModifiers(L);
    io.KeyAlt = (mod & GINPUT_ALT_MODIFIER) > 0;
    io.KeyCtrl = (mod & GINPUT_CTRL_MODIFIER) > 0;
    io.KeyShift = (mod & GINPUT_SHIFT_MODIFIER) > 0;
    io.KeySuper = (mod & GINPUT_META_MODIFIER) > 0;

    return 0;
}

int ImGui_impl_KeyDown(lua_State* L)
{
    int keyCode = (int)getfield(L, "keyCode");

    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[keyCode] = true;

    int mod = getKeyboardModifiers(L);
    io.KeyAlt = (mod & GINPUT_ALT_MODIFIER) > 0;
    io.KeyCtrl = (mod & GINPUT_CTRL_MODIFIER) > 0;
    io.KeyShift = (mod & GINPUT_SHIFT_MODIFIER) > 0;
    io.KeySuper = (mod & GINPUT_META_MODIFIER) > 0;

    return 0;
}

int ImGui_impl_KeyChar(lua_State* L)
{
    lua_pushstring(L, "text");
    lua_gettable(L, -2);
    const char* text = lua_tostring(L, -1);
    lua_pop(L, 1);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharactersUTF8(text);

    return 0;
}


/// DRAWING STUFF

int ImGui_impl_NewFrame(lua_State* L)
{
    double deltaTime = getfield(L, "deltaTime");

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;
    ImGui::NewFrame();

    return 0;
}

int ImGui_impl_Render(lua_State* _UNUSED(L))
{
    ImGui::Render();
    return 0;
}

int ImGui_impl_EndFrame(lua_State* _UNUSED(L))
{
    ImGui::EndFrame();
    return 0;
}

// Windows
int ImGui_impl_Begin(lua_State* L)
{
    const char* name = luaL_checkstring(L, 2);

    switch(lua_type(L, 3))
    {
        case LUA_TBOOLEAN:
            {
                bool p_open = lua_toboolean(L, 3) > 0;
                ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);
                bool result = ImGui::Begin(name, &p_open, flags);

                lua_pushboolean(L, p_open);
                lua_pushboolean(L, result);
                return 2;
            }
            break;
        case LUA_TNIL:
            {
                ImGuiWindowFlags flags = luaL_checkinteger(L, 4);
                lua_pushboolean(L, ImGui::Begin(name, NULL, flags));
                return 1;
            }
            break;
        default:
        {
            lua_pushfstring(L, "bad argument #2 to 'beginWindow' (boolean/nil expected, got %s)", lua_typename(L, 3));
            lua_error(L);
            return 0;
        }
        break;
    }
}

int ImGui_impl_End(lua_State* _UNUSED(L))
{
    ImGui::End();
    return 0;
}

// Child Windows
int ImGui_impl_BeginChild(lua_State* L)
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
        ImGuiID id = luaL_checkinteger(L, 2);
        result = ImGui::BeginChild(id, size, border, flags);
    }

    lua_pushboolean(L, result);

    return 1;
}

int ImGui_impl_EndChild(lua_State* _UNUSED(L))
{
    ImGui::EndChild();
    return 0;
}

// Windows Utilities
int ImGui_impl_IsWindowAppearing(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowAppearing());
    return 1;
}

int ImGui_impl_IsWindowCollapsed(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowCollapsed());
    return 1;
}

int ImGui_impl_IsWindowFocused(lua_State* L)
{
    ImGuiFocusedFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowFocused(flag));
    return 1;
}

int ImGui_impl_IsWindowHovered(lua_State* L)
{
    ImGuiHoveredFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowHovered(flag));
    return 1;
}

int ImGui_impl_GetWindowPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetWindowPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_GetWindowSize(lua_State* L)
{
    ImVec2 size = ImGui::GetWindowSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return  2;
}

int ImGui_impl_GetWindowWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowWidth());
    return  1;
}

int ImGui_impl_GetWindowHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowHeight());
    return  1;
}

int ImGui_impl_SetNextWindowPos(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    ImVec2 pivot = ImVec2(luaL_optnumber(L, 5, 0), luaL_optnumber(L, 6, 0));

    ImGui::SetNextWindowPos(pos, cond, pivot);

    return 0;
}

int ImGui_impl_SetNextWindowSize(lua_State* L)
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
    //double step = (double)(int)(intptr_t)data->UserData;
    //ImVec2 snap_pos = ImVec2((int)(data->Pos.x / step + 0.5f) * step, (int)(data->Pos.y / step + 0.5f) * step);
    //ImGui::SetNextWindowPos(snap_pos, ImGuiCond_Always);
    //data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step);

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

int ImGui_impl_SetNextWindowSizeConstraints(lua_State* L)
{
    ImVec2 size_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 size_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImGui::SetNextWindowSizeConstraints(size_min, size_max, NextWindowSizeConstraintCallback, (void *)L);
    return 0;
}

int ImGui_impl_SetNextWindowContentSize(lua_State* L)
{
    double w = luaL_checknumber(L, 2);
    double h = luaL_checknumber(L, 3);
    const ImVec2& size = ImVec2(w, h);

    ImGui::SetNextWindowContentSize(size);

    return 0;
}

int ImGui_impl_SetNextWindowCollapsed(lua_State* L)
{
    // bool collapsed, ImGuiCond cond = 0
    bool collapsed = lua_toboolean(L, 2) > 0;
    ImGuiCond cond = luaL_optinteger(L, 3, 0);

    ImGui::SetNextWindowCollapsed(collapsed, cond);
    return 0;
}

int ImGui_impl_SetNextWindowFocus(lua_State* _UNUSED(L))
{
    ImGui::SetNextWindowFocus();
    return 0;
}

int ImGui_impl_SetNextWindowBgAlpha(lua_State* L)
{
    double alpha = luaL_checknumber(L, 2);
    ImGui::SetNextWindowBgAlpha(alpha);
    return 0;
}

int ImGui_impl_SetWindowPos(lua_State* L)
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

int ImGui_impl_SetWindowSize(lua_State* L)
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

int ImGui_impl_SetWindowCollapsed(lua_State* L)
{
    if (lua_gettop(L) == 4)
    {
        const char* name = luaL_checkstring(L, 2);
        bool collapsed = lua_toboolean(L, 3) > 0;
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowCollapsed(name, collapsed, cond);
    }
    else
    {
        bool collapsed = lua_toboolean(L, 2) > 0;
        ImGuiCond cond = luaL_optinteger(L, 3, 0);

        ImGui::SetWindowCollapsed(collapsed, cond);
    }

    return 0;
}

int ImGui_impl_SetWindowFocus(lua_State* L)
{
    if (lua_gettop(L) == 2)
    {
        const char* name = luaL_checkstring(L, 2);
        ImGui::SetWindowFocus(name);
    }
    else
        ImGui::SetWindowFocus();

    return 0;
}

int ImGui_impl_SetWindowFontScale(lua_State* L)
{
    double scale = luaL_checknumber(L, 2);
    ImGui::SetWindowFontScale(scale);
    return 0;
}

// Content region
// - Those functions are bound to be redesigned soon (they are confusing, incomplete and return values in local window coordinates which increases confusion)
int ImGui_impl_GetContentRegionMax(lua_State* L)
{
    ImVec2 max = ImGui::GetContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetContentRegionAvail(lua_State* L)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    lua_pushnumber(L, avail.x);
    lua_pushnumber(L, avail.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionMin(lua_State* L)
{
    ImVec2 min = ImGui::GetWindowContentRegionMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionMax(lua_State* L)
{
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetWindowContentRegionWidth());
    return 1;
}

// Windows Scrolling
int ImGui_impl_GetScrollX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollX());
    return 1;
}

int ImGui_impl_GetScrollY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollY());
    return 1;
}

int ImGui_impl_GetScrollMaxX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxX());
    return 1;
}

int ImGui_impl_GetScrollMaxY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxY());
    return 1;
}

int ImGui_impl_SetScrollX(lua_State* L)
{
    double scroll_x = luaL_checknumber(L, 2);
    ImGui::SetScrollX(scroll_x);
    return 0;
}

int ImGui_impl_SetScrollY(lua_State* L)
{
    double scroll_y = luaL_checknumber(L, 2);
    ImGui::SetScrollY(scroll_y);
    return 0;
}

int ImGui_impl_SetScrollHereX(lua_State* L)
{
    double center_x_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereX(center_x_ratio);
    return 0;
}

int ImGui_impl_SetScrollHereY(lua_State* L)
{
    double center_y_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereY(center_y_ratio);
    return 0;
}

int ImGui_impl_SetScrollFromPosX(lua_State* L)
{
    double local_x = luaL_checknumber(L, 2);
    double center_x_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosX(local_x, center_x_ratio);
    return 0;
}

int ImGui_impl_SetScrollFromPosY(lua_State* L)
{
    double local_y = luaL_checknumber(L, 2);
    double center_y_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosY(local_y, center_y_ratio);
    return 0;
}

// Parameters stacks (shared)
int ImGui_impl_PushStyleColor(lua_State* L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);

    ImGui::PushStyleColor(idx, GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)));

    return 0;
}

int ImGui_impl_PopStyleColor(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleColor(count);
    return 0;
}

int ImGui_impl_PushStyleVar(lua_State* L)
{
    ImGuiStyleVar idx = luaL_checkinteger(L, 2);

    if (lua_gettop(L) == 4)
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

int ImGui_impl_PopStyleVar(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleVar(count);
    return 0;
}

int ImGui_impl_GetFontSize(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFontSize());
    return 1;
}

// Parameters stacks (current window)
int ImGui_impl_PushItemWidth(lua_State* L)
{
    double item_width = luaL_checknumber(L, 2);
    ImGui::PushItemWidth(item_width);
    return 0;
}

int ImGui_impl_PopItemWidth(lua_State* _UNUSED(L))
{
    ImGui::PopItemWidth();
    return 0;
}

int ImGui_impl_SetNextItemWidth(lua_State* L)
{
    double item_width = luaL_checknumber(L, 2);
    ImGui::SetNextItemWidth(item_width);
    return 0;
}

int ImGui_impl_CalcItemWidth(lua_State* L)
{
    lua_pushnumber(L, ImGui::CalcItemWidth());
    return 1;
}

int ImGui_impl_PushTextWrapPos(lua_State* _UNUSED(L))
{
    double wrap_local_pos_x = luaL_optnumber(L, 2, 0.0f);
    ImGui::PushTextWrapPos(wrap_local_pos_x);
    return 0;
}

int ImGui_impl_PopTextWrapPos(lua_State* _UNUSED(L))
{
    ImGui::PopTextWrapPos();
    return 0;
}

int ImGui_impl_PushAllowKeyboardFocus(lua_State* L)
{
    bool allow_keyboard_focus = lua_toboolean(L, 2) > 0;
    ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
    return 0;
}

int ImGui_impl_PopAllowKeyboardFocus(lua_State* _UNUSED(L))
{
    ImGui::PopAllowKeyboardFocus();
    return 0;
}

int ImGui_impl_PushButtonRepeat(lua_State* L)
{
    bool repeat = lua_toboolean(L, 2) > 0;
    ImGui::PushButtonRepeat(repeat);
    return 0;
}

int ImGui_impl_PopButtonRepeat(lua_State* _UNUSED(L))
{
    ImGui::PopButtonRepeat();
    return 0;
}

// Cursor / Layout
int ImGui_impl_Separator(lua_State* _UNUSED(L))
{
    ImGui::Separator();
    return 0;
}

int ImGui_impl_SameLine(lua_State* L)
{
    double offset_from_start_x = luaL_optnumber(L, 2, 0.0f);
    double spacing = luaL_optnumber(L, 3, -1.0f);
    ImGui::SameLine(offset_from_start_x, spacing);
    return 0;
}

int ImGui_impl_NewLine(lua_State* _UNUSED(L))
{
    ImGui::NewLine();
    return 0;
}

int ImGui_impl_Spacing(lua_State* _UNUSED(L))
{
    ImGui::Spacing();
    return 0;
}

int ImGui_impl_Dummy(lua_State* L)
{
    double w = luaL_checknumber(L, 2);
    double h = luaL_checknumber(L, 3);

    ImGui::Dummy(ImVec2(w, h));
    return 0;
}

int ImGui_impl_Indent(lua_State* L)
{
    double indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Indent(indent_w);
    return 0;
}

int ImGui_impl_Unindent(lua_State* L)
{
    double indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Unindent(indent_w);
    return 0;
}

int ImGui_impl_BeginGroup(lua_State* _UNUSED(L))
{
    ImGui::BeginGroup();
    return 0;
}

int ImGui_impl_EndGroup(lua_State* _UNUSED(L))
{
    ImGui::EndGroup();
    return 0;
}

int ImGui_impl_GetCursorPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_GetCursorPosX(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetCursorPosX());
    return 1;
}

int ImGui_impl_GetCursorPosY(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetCursorPosY());
    return 1;
}

int ImGui_impl_SetCursorPos(lua_State* L)
{
    ImVec2 local_pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImGui::SetCursorPos(local_pos);
    return 0;
}

int ImGui_impl_SetCursorPosX(lua_State* L)
{
    double local_x = luaL_checknumber(L, 2);
    ImGui::SetCursorPosX(local_x);
    return 0;
}

int ImGui_impl_SetCursorPosY(lua_State* L)
{
    double local_y = luaL_checknumber(L, 2);
    ImGui::SetCursorPosY(local_y);
    return 0;
}

int ImGui_impl_GetCursorStartPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorStartPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_GetCursorScreenPos(lua_State* L)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

//void SetCursorScreenPos(const ImVec2& pos);

int ImGui_impl_AlignTextToFramePadding(lua_State* _UNUSED(L))
{
    ImGui::AlignTextToFramePadding();
    return 0;
}

int ImGui_impl_GetTextLineHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeight());
    return 1;
}

int ImGui_impl_GetTextLineHeightWithSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeightWithSpacing());
    return 1;
}

int ImGui_impl_GetFrameHeight(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFrameHeight());
    return 1;
}

int ImGui_impl_GetFrameHeightWithSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetFrameHeightWithSpacing());
    return 1;
}

int ImGui_impl_PushID(lua_State* L)
{
    if (lua_gettop(L) == 2)
    {
        const int arg_type = lua_type(L, 2);
        switch(arg_type)
        {
        case(LUA_TNIL):
            {
                lua_pushstring(L, "bad argument #2 to 'pushID' (string/number/table/function expected, got nil)");
                lua_error(L);
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
        ImGui::PushID(luaL_checkstring(L, 2), luaL_checkstring(L, 3));
    }
    return 0;
}

int ImGui_impl_PopID(lua_State* _UNUSED(L))
{
    ImGui::PopID();
    return 0;
}

int ImGui_impl_GetID(lua_State* L)
{

    if (lua_gettop(L) == 2)
    {
        lua_pushnumber(L, ImGui::GetID(lua_topointer(L, 2)));
    }
    else
    {
        lua_pushnumber(L, ImGui::GetID(luaL_checkstring(L, 2), luaL_checkstring(L, 3)));
    }
    return 1;
}

// Widgets: Text

int ImGui_impl_TextUnformatted(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    const char* text_end = luaL_optstring(L, 3, NULL);
    ImGui::TextUnformatted(text, text_end);
    return 0;
}

int ImGui_impl_Text(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::Text("%s", text);
    return 0;
}

int ImGui_impl_TextColored(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextColored(GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)), "%s", text);
    return 0;
}

int ImGui_impl_TextDisabled(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextDisabled("%s", text);
    return 0;
}

int ImGui_impl_TextWrapped(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextWrapped("%s", text);
    return 0;
}

int ImGui_impl_LabelText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImGui::LabelText(label, "%s", text);
    return 0;
}

int ImGui_impl_BulletText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::BulletText("%s", text);
    return 0;
}

// Widgets: Main
int ImGui_impl_Button(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    double w = luaL_optnumber(L, 3, 0.0f);
    double h = luaL_optnumber(L, 4, 0.0f);
    const ImVec2& size = ImVec2(w, h);
    lua_pushboolean(L, ImGui::Button(label, size));
    return 1;
}

int ImGui_impl_SmallButton(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    lua_pushboolean(L, ImGui::SmallButton(label));
    return 1;
}

int ImGui_impl_InvisibleButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    double w = luaL_optnumber(L, 3, 0.0f);
    double h = luaL_optnumber(L, 4, 0.0f);
    const ImVec2& size = ImVec2(w, h);
    lua_pushboolean(L, ImGui::InvisibleButton(str_id, size));
    return 1;
}

int ImGui_impl_ArrowButton(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiDir dir = luaL_checknumber(L, 3);
    lua_pushboolean(L, ImGui::ArrowButton(str_id, dir));
    return 1;
}

int ImGui_impl_Image(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L,  9, 0.0f), luaL_optnumber(L, 10, 0.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 11, 1.0f), luaL_optnumber(L, 12, 1.0f));

    ImGui::Image(data.texture, size, uv0 * data.uv, uv1 * data.uv, tint, border);
    return 0;
}

int ImGui_impl_ImageFilled(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 0.0f));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 11, 0.0f), luaL_optnumber(L, 12, 0.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 13, 1.0f), luaL_optnumber(L, 14, 1.0f));

    ImGui::ImageFilled(data.texture, size, uv0 * data.uv, uv1 * data.uv, bg_col, tint, border);
    return 0;
}

int ImGui_impl_ImageButton(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    int frame_padding = luaL_optinteger(L, 5, -1);
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 1.0f));
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 8, 0xffffff), luaL_optnumber(L, 9, 0.0f));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 10, 0.0f), luaL_optnumber(L, 11, 0.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 12, 1.0f), luaL_optnumber(L, 13, 1.0f));

    lua_pushboolean(L, ImGui::ImageButton(data.texture, size, uv0 * data.uv, uv1 * data.uv, frame_padding, bg_col, tint));
    return 1;
}

//ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)


int ImGui_impl_ImageButtonWithText(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    int frame_padding = luaL_optinteger(L, 6, -1);
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 tint_col = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 1.0f));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 11, 1.0f), luaL_optnumber(L, 12, 1.0f));
    lua_pushboolean(L, ImGui::ImageButtonWithText(data.texture, label, size, uv0 * data.uv, uv1 * data.uv, frame_padding, bg_col, tint_col));
    return 1;
}

int ImGui_impl_Checkbox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool v = lua_toboolean2(L, 3) > 0;
    bool result = ImGui::Checkbox(label, &v);
    lua_pushboolean(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_CheckboxFlags(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    unsigned int* flags = 0;
    unsigned int flags_value = 0;

    lua_pushboolean(L, ImGui::CheckboxFlags(label, flags, flags_value));
    return 1;
}

int ImGui_impl_RadioButton(lua_State* L)
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

int ImGui_impl_ProgressBar(lua_State* L)
{
    double fraction = luaL_checknumber(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, -1.0f), luaL_optnumber(L, 4, 0.0f));
    const char* overlay = luaL_optstring(L, 5, NULL);
    ImGui::ProgressBar(fraction, size, overlay);
    return  0;
}

int ImGui_impl_Bullet(lua_State* _UNUSED(L))
{
    ImGui::Bullet();
    return 0;
}

// Widgets: Combo Box
int ImGui_impl_BeginCombo(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* preview_value = luaL_checkstring(L, 3);
    ImGuiComboFlags flags = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::BeginCombo(label, preview_value, flags));
    return 1;
}

int ImGui_impl_EndCombo(lua_State* _UNUSED(L))
{
    ImGui::EndCombo();
    return 0;
}

int ImGui_impl_Combo(lua_State* L)
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
            luaL_checktype(L, 4, LUA_TTABLE);
            int len = luaL_getn(L, 4);
            if (len == 0)
            {
                lua_pushnumber(L, -1);
                lua_pushboolean(L, false);
                return 2;
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
            lua_pushfstring(L, "bad argument #3 to 'combo' (table/string expected, got %s)", lua_typename(L, 4));
            lua_error(L);
            return 0;
        }
    }

    lua_pushinteger(L, item_current);
    lua_pushboolean(L, result);
    return 2;
}

// Widgets: Drags
int ImGui_impl_DragFloat(lua_State* L)
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

int ImGui_impl_DragFloat2(lua_State* L)
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

int ImGui_impl_DragFloat3(lua_State* L)
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

int ImGui_impl_DragFloat4(lua_State* L)
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

int ImGui_impl_DragFloatRange2(lua_State* L)
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

int ImGui_impl_DragInt(lua_State* L)
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

int ImGui_impl_DragInt2(lua_State* L)
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

int ImGui_impl_DragInt3(lua_State* L)
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

int ImGui_impl_DragInt4(lua_State* L)
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

int ImGui_impl_DragIntRange2(lua_State* L)
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

int ImGui_impl_DragScalar(lua_State* L)
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

// Widgets: Sliders
int ImGui_impl_SliderFloat(lua_State* L)
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

int ImGui_impl_SliderFloat2(lua_State* L)
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

int ImGui_impl_SliderFloat3(lua_State* L)
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

int ImGui_impl_SliderFloat4(lua_State* L)
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

int ImGui_impl_SliderAngle(lua_State* L)
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

int ImGui_impl_SliderInt(lua_State* L)
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

int ImGui_impl_SliderInt2(lua_State* L)
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

int ImGui_impl_SliderInt3(lua_State* L)
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

int ImGui_impl_SliderInt4(lua_State* L)
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

int ImGui_impl_SliderScalar(lua_State* L)
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

int ImGui_impl_VSliderFloat(lua_State* L)
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

int ImGui_impl_VSliderInt(lua_State* L)
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

int ImGui_impl_VSliderScalar(lua_State* L)
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

//---------------------------------------------------
// Custom filled sliders
//---------------------------------------------------

int ImGui_impl_FilledSliderFloat(lua_State* L)
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

int ImGui_impl_FilledSliderFloat2(lua_State* L)
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

int ImGui_impl_FilledSliderFloat3(lua_State* L)
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

int ImGui_impl_FilledSliderFloat4(lua_State* L)
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

int ImGui_impl_FilledSliderAngle(lua_State* L)
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

int ImGui_impl_FilledSliderInt(lua_State* L)
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

int ImGui_impl_FilledSliderInt2(lua_State* L)
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

int ImGui_impl_FilledSliderInt3(lua_State* L)
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

int ImGui_impl_FilledSliderInt4(lua_State* L)
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

int ImGui_impl_FilledSliderScalar(lua_State* L)
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

int ImGui_impl_VFilledSliderFloat(lua_State* L)
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

int ImGui_impl_VFilledSliderInt(lua_State* L)
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

int ImGui_impl_VFilledSliderScalar(lua_State* L)
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
static int TextInputCallback(ImGuiInputTextCallbackData* data)
{
    //lua_State* L = (lua_State *)data->UserData;
    //lua_pushstring(L, data->Buf);
    //lua_error(L);
    return 0;
}

// Widgets: Input with Keyboard
int ImGui_impl_InputText(lua_State* L)
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

int ImGui_impl_InputTextMultiline(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    int buffer_size = luaL_checkinteger(L, 4);

    char* buffer = new char[buffer_size];
    sprintf(buffer, "%s", text);

    ImVec2 size = ImVec2(luaL_optnumber(L, 5, 0.0f), luaL_optnumber(L, 6, 0.0f));
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);
    // ImGuiInputTextCallback callback = NULL; void* user_data = NULL;

    bool result = ImGui::InputTextMultiline(label, buffer, buffer_size, size, flags);
    lua_pushstring(L, &(*buffer));
    lua_pushboolean(L, result);
    delete[] buffer;
    return 2;

}

int ImGui_impl_InputTextWithHint(lua_State* L)
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

int ImGui_impl_InputFloat(lua_State* L)
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

int ImGui_impl_InputFloat2(lua_State* L)
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

int ImGui_impl_InputFloat3(lua_State* L)
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

int ImGui_impl_InputFloat4(lua_State* L)
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

int ImGui_impl_InputInt(lua_State* L)
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

int ImGui_impl_InputInt2(lua_State* L)
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

int ImGui_impl_InputInt3(lua_State* L)
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

int ImGui_impl_InputInt4(lua_State* L)
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

int ImGui_impl_InputDouble(lua_State* L)
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

int ImGui_impl_InputScalar(lua_State* L)
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

// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little colored preview square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
int ImGui_impl_ColorEdit3(lua_State* L)
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

int ImGui_impl_ColorEdit4(lua_State* L)
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

int ImGui_impl_ColorPicker3(lua_State* L)
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

int ImGui_impl_ColorPicker4(lua_State* L)
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

int ImGui_impl_ColorButton(lua_State* L)
{
    const char* desc_id = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);
    ImVec2 size = ImVec2(luaL_optnumber(L, 6, 0), luaL_optnumber(L, 7, 0));

    bool result = ImGui::ColorButton(desc_id, col, flags, size);

    //GColor conv = GColor::toHex(col);
    //lua_pushnumber(L, conv.hex);
    //lua_pushnumber(L, conv.alpha);
    lua_pushboolean(L, result);
    return 1;
}

int ImGui_impl_SetColorEditOptions(lua_State* L)
{
    ImGuiColorEditFlags flags = luaL_checkinteger(L, 2);
    ImGui::SetColorEditOptions(flags);
    return 0;
}

// Widgets: Trees
int ImGui_impl_TreeNode(lua_State* L)
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

int ImGui_impl_TreeNodeEx(lua_State* L)
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

int ImGui_impl_TreePush(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGui::TreePush(str_id);
    return 0;
}

int ImGui_impl_TreePop(lua_State* _UNUSED(L))
{
    ImGui::TreePop();
    return 0;
}

int ImGui_impl_GetTreeNodeToLabelSpacing(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTreeNodeToLabelSpacing());
    return 1;
}

// FIXME lua_type
int ImGui_impl_CollapsingHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    if (lua_type(L, 3) == LUA_TBOOLEAN)
    {
        bool p_open = lua_toboolean(L, 3) > 0;
        ImGuiTreeNodeFlags flags = luaL_optinteger(L, 4, 0);
        lua_pushboolean(L, ImGui::CollapsingHeader(label, &p_open, flags));
        lua_pushboolean(L, p_open);
        return 2;
    }
    else
    {
        ImGuiTreeNodeFlags flags = luaL_optinteger(L, 3, 0);
        lua_pushboolean(L, ImGui::CollapsingHeader(label, flags));
        return 1;
    }
}

int ImGui_impl_SetNextItemOpen(lua_State* L)
{
    bool is_open = lua_toboolean(L, 2);
    ImGuiCond cond = luaL_optinteger(L, 3, 0);
    ImGui::SetNextItemOpen(is_open, cond);
    return 0;
}

// Widgets: Selectables
int ImGui_impl_Selectable(lua_State* L)
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

// Widgets: List Boxes
int ImGui_impl_ListBox(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    static int current_item = luaL_checkinteger(L, 3);

    luaL_checktype(L, 4, LUA_TTABLE);
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
    int maxItems = luaL_optinteger(L, 5, -1);

    bool result = ImGui::ListBox(label, &current_item, items, len, maxItems);

    lua_pushinteger(L, current_item);
    lua_pushboolean(L, result);
    delete[] items;
    return 2;
}

int ImGui_impl_ListBoxHeader(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, 0), luaL_optnumber(L, 4, 0));

    lua_pushboolean(L, ImGui::ListBoxHeader(label, size));
    return 1;
}

int ImGui_impl_ListBoxHeader2(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    int items_count = luaL_checkinteger(L, 3);

    lua_pushboolean(L, ImGui::ListBoxHeader(label, items_count));
    return 1;
}

int ImGui_impl_ListBoxFooter(lua_State* _UNUSED(L))
{
    ImGui::ListBoxFooter();
    return 0;
}

// Widgets: Data Plotting
int ImGui_impl_PlotLines(lua_State* L)
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

    ImGui::PlotLines(label, (float*)&values, len, values_offset, overlay_text, scale_min, scale_max, graph_size, stride);
    delete[] values;
    return 0;
}

int ImGui_impl_PlotHistogram(lua_State* L)
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

    ImGui::PlotHistogram(label, (float*)&values, len, values_offset, overlay_text, scale_min, scale_max, graph_size, stride);
    delete[] values;
    return 0;
}

// Widgets: Value() Helpers.
int ImGui_impl_Value(lua_State* L)
{
    const char* prefix = luaL_checkstring(L, 2);
    const int valueType = lua_type(L, 3);
    switch(valueType)
    {
        case LUA_TBOOLEAN:
            {
                ImGui::Value(prefix, lua_toboolean(L, 3) > 0);
                break;
            }
        case LUA_TNUMBER:
            {
                double n = lua_tonumber(L, 3);
                int intN = (int)n;
                if (n == intN)
                {
                    ImGui::Value(prefix, intN);
                }
                else
                {
                    ImGui::Value(prefix, n, luaL_optstring(L, 4, NULL));
                }
                break;
            }
        default:
            {
                lua_pushstring(L, "Type mismatch. 'Number' or 'Boolean' expected.");
                lua_error(L);
                break;
            }
    }

    return 0;
}

// Widgets: Menus
int ImGui_impl_BeginMenuBar(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginMenuBar());
    return 1;
}

int ImGui_impl_EndMenuBar(lua_State* _UNUSED(L))
{
    ImGui::EndMenuBar();
    return 0;
}

int ImGui_impl_BeginMainMenuBar(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginMainMenuBar());
    return 1;
}

int ImGui_impl_EndMainMenuBar(lua_State* _UNUSED(L))
{
    ImGui::EndMainMenuBar();
    return 0;
}

int ImGui_impl_BeginMenu(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool enabled = lua_toboolean(L, 3) > 0;
    lua_pushboolean(L, ImGui::BeginMenu(label, enabled));
    return 1;
}

int ImGui_impl_EndMenu(lua_State* _UNUSED(L))
{
    ImGui::EndMenu();
    return 0;
}

int ImGui_impl_MenuItem(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* shortcut = luaL_optstring(L, 3, NULL);
    int selected = luaL_optboolean(L, 4, 0);
    int enabled = luaL_optboolean(L, 5, 1);

    lua_pushboolean(L, ImGui::MenuItem(label, shortcut, selected, enabled));

    return 1;
}

int ImGui_impl_MenuItemWithShortcut(lua_State* L)
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

// Tooltips
int ImGui_impl_BeginTooltip(lua_State* _UNUSED(L))
{
    ImGui::BeginTooltip();
    return 0;
}

int ImGui_impl_EndTooltip(lua_State* _UNUSED(L))
{
    ImGui::EndTooltip();
    return 0;
}

int ImGui_impl_SetTooltip(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::SetTooltip("%s", text);
    return 0;
}

// Popups, Modals
int ImGui_impl_BeginPopup(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiWindowFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushboolean(L, ImGui::BeginPopup(str_id, flags));
    return 1;
}

int ImGui_impl_BeginPopupModal(lua_State* L)
{
    const char* name = luaL_checkstring(L, 2);
    bool p_open = lua_tointeger(L, 3) > 0;
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::BeginPopupModal(name, &p_open, flags);

    lua_pushboolean(L, p_open);
    lua_pushboolean(L, result);

    return 2;
}

int ImGui_impl_EndPopup(lua_State* _UNUSED(L))
{
    ImGui::EndPopup();
    return 0;
}

// Popups: open/close functions
int ImGui_impl_OpenPopup(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 0);
    ImGui::OpenPopup(str_id, popup_flags);
    return 0;
}

int ImGui_impl_OpenPopupContextItem(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::OpenPopupContextItem(str_id, popup_flags));
    return 1;
}

int ImGui_impl_CloseCurrentPopup(lua_State* _UNUSED(L))
{
    ImGui::CloseCurrentPopup();
    return 0;
}

// Popups: open+begin combined functions helpers
int ImGui_impl_BeginPopupContextItem(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextItem(str_id, popup_flags));
    return 1;
}

int ImGui_impl_BeginPopupContextWindow(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextWindow(str_id, popup_flags));
    return 1;
}

int ImGui_impl_BeginPopupContextVoid(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextVoid(str_id, popup_flags));
    return 1;
}

// Popups: test function
int ImGui_impl_IsPopupOpen(lua_State* L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::IsPopupOpen(str_id, popup_flags));
    return 1;
}

// Columns
int ImGui_impl_Columns(lua_State* L)
{
    int count = luaL_optinteger(L, 2, 1);
    const char* id = luaL_optstring(L, 3, NULL);
    bool border = luaL_optboolean(L, 4, 1);

    ImGui::Columns(count, id, border);

    return 0;
}

int ImGui_impl_NextColumn(lua_State* _UNUSED(L))
{
    ImGui::NextColumn();
    return 0;
}

int ImGui_impl_GetColumnIndex(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetColumnIndex());
    return 1;
}

int ImGui_impl_GetColumnWidth(lua_State* L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnWidth(column_index));
    return 1;
}

int ImGui_impl_SetColumnWidth(lua_State* L)
{
    int column_index = luaL_checkinteger(L, 2);
    double width = luaL_checknumber(L, 3);
    ImGui::SetColumnWidth(column_index, width);
    return 0;
}

int ImGui_impl_GetColumnOffset(lua_State* L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnOffset(column_index));
    return 1;
}

int ImGui_impl_SetColumnOffset(lua_State* L)
{
    int column_index = luaL_checkinteger(L, 2);
    double offset = luaL_checknumber(L, 3);
    ImGui::SetColumnOffset(column_index, offset);
    return 0;
}

int ImGui_impl_GetColumnsCount(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetColumnsCount());
    return 1;
}

// Tab Bars, Tabs
int ImGui_impl_BeginTabBar(lua_State* L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiTabBarFlags flags = luaL_optinteger(L, 3, 0);

    lua_pushboolean(L, ImGui::BeginTabBar(str_id, flags));
    return 1;
}

int ImGui_impl_EndTabBar(lua_State* _UNUSED(L))
{
    ImGui::EndTabBar();
    return  0;
}

int ImGui_impl_BeginTabItem(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);
    bool p_open = lua_toboolean(L, 3) > 0;
    ImGuiTabItemFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::BeginTabItem(label, &p_open, flags);

    lua_pushboolean(L, p_open);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_EndTabItem(lua_State* _UNUSED(L))
{
    ImGui::EndTabItem();
    return 0;
}

int ImGui_impl_SetTabItemClosed(lua_State* L)
{
    const char* tab_or_docked_window_label = luaL_checkstring(L, 2);
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
    return 0;
}

#ifdef IMGUI_HAS_DOCK

int ImGui_impl_DockSpace(lua_State* L)
{
    ImGuiID id = luaL_checkinteger(L, 2);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 5, 0);
    ImGui::DockSpace(id, size, flags);
    return 0;
}

int ImGui_impl_DockSpaceOverViewport(lua_State* L)
{
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushinteger(L, ImGui::DockSpaceOverViewport(NULL, flags));
    return 1;
}

int ImGui_impl_SetNextWindowDockID(lua_State* L)
{
    ImGuiID dock_id = luaL_checkinteger(L, 2);
    ImGuiCond cond = luaL_optinteger(L, 3, 0);
    ImGui::SetNextWindowDockID(dock_id, cond);
    return 0;
}

int ImGui_impl_GetWindowDockID(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetWindowDockID());
    return 1;
}

int ImGui_impl_IsWindowDocked(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsWindowDocked());
    return 1;
}

// DockBuilder [BETA API]

void lua_setintfield(lua_State* L, int idx, int index)
{
    lua_pushinteger(L, index);
    lua_insert(L, -2);
    lua_settable(L,idx-(idx<0));
}
/*
void createDockNodeTable(lua_State* L, ImGuiDockNode* node)
{
    if (!node)
    {
        lua_pushnil(L);
        return;
    }

    lua_newtable(L);
    lua_pushinteger(L, node->ID); lua_setfield(L, -2, "ID");
    lua_pushinteger(L, node->SharedFlags); lua_setfield(L, -2, "sharedFlags");
    lua_pushinteger(L, node->LocalFlags); lua_setfield(L, -2, "localFlags");

    // ParentNode
    if (node->ParentNode)
    {
        createDockNodeTable(L, node->ParentNode);
        lua_pushvalue(L, -1);
        lua_pop(L, 1);
        lua_setfield(L, -2, "parentNode");
    }
}
*/

int ImGui_impl_DockBuilderDockWindow(lua_State* L)
{
    const char* window_name = luaL_checkstring(L, 2);
    ImGuiID node_id = luaL_checkinteger(L, 3);
    ImGui::DockBuilderDockWindow(window_name, node_id);
    return 0;
}
/*
int ImGui_impl_DockBuilderGetNode(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(node_id);

    createDockNodeTable(L, node);
    return 1;
}
*/
int ImGui_impl_DockBuilderSetNodePos(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImVec2 pos = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGui::DockBuilderSetNodePos(node_id, pos);
    return 0;
}

int ImGui_impl_DockBuilderSetNodeSize(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGui::DockBuilderSetNodeSize(node_id, size);
    return 0;
}

int ImGui_impl_DockBuilderAddNode(lua_State* L)
{
    ImGuiID node_id = luaL_optinteger(L, 2, 0);
    ImGuiDockNodeFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushinteger(L, ImGui::DockBuilderAddNode(node_id, flags));
    return 1;
}

int ImGui_impl_DockBuilderRemoveNode(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImGui::DockBuilderRemoveNode(node_id);
    return 0;
}

int ImGui_impl_DockBuilderRemoveNodeChildNodes(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImGui::DockBuilderRemoveNodeChildNodes(node_id);
    return 0;
}

int ImGui_impl_DockBuilderRemoveNodeDockedWindows(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    bool clear_settings_refs = lua_toboolean(L, 3);
    ImGui::DockBuilderRemoveNodeDockedWindows(node_id, clear_settings_refs);
    return 0;
}

int ImGui_impl_DockBuilderSplitNode(lua_State* L)
{
    ImGuiID id = luaL_checkinteger(L, 2);
    ImGuiDir split_dir = luaL_checkinteger(L, 3);
    float size_ratio_for_node_at_dir = luaL_checknumber(L, 4);
    ImGuiID* out_id_at_dir;
    ImGuiID* out_id_at_opposite_dir;

    if (lua_isnil(L, 5))
        out_id_at_dir = nullptr;
    else
    {
        ImGuiID id = luaL_checkinteger(L, 5);
        out_id_at_dir = &id;
    }

    if (lua_isnil(L, 6))
        out_id_at_opposite_dir = nullptr;
    else
    {
        ImGuiID id = luaL_checkinteger(L, 6);
        out_id_at_opposite_dir = &id;
    }

    lua_pushinteger(L, ImGui::DockBuilderSplitNode(id, split_dir, size_ratio_for_node_at_dir, out_id_at_dir, out_id_at_opposite_dir));
    lua_pushinteger(L,* out_id_at_dir);
    lua_pushinteger(L,* out_id_at_opposite_dir);
    return 3;
}

// TODO
int ImGui_impl_DockBuilderCopyNode(lua_State* L)
{
    ImGuiID src_node_id = luaL_checkinteger(L, 2);
    ImGuiID dst_node_id = luaL_checkinteger(L, 3);
    ImVector<ImGuiID>* out_node_remap_pairs;

    ImGui::DockBuilderCopyNode(src_node_id, dst_node_id, out_node_remap_pairs);
    return 0;
}

int ImGui_impl_DockBuilderCopyWindowSettings(lua_State* L)
{
    const char* src_name = luaL_checkstring(L, 2);
    const char* dst_name = luaL_checkstring(L, 3);
    ImGui::DockBuilderCopyWindowSettings(src_name, dst_name);
    return 0;
}

int ImGui_impl_DockBuilderCopyDockSpace(lua_State* L)
{
    ImGuiID src_dockspace_id = luaL_checkinteger(L, 2);
    ImGuiID dst_dockspace_id = luaL_checkinteger(L, 3);
    ImVector<const char*>* in_window_remap_pairs;
    ImGui::DockBuilderCopyDockSpace(src_dockspace_id, dst_dockspace_id, in_window_remap_pairs);
    return 0;
}

int ImGui_impl_DockBuilderFinish(lua_State* L)
{
    ImGuiID node_id = luaL_checkinteger(L, 2);
    ImGui::DockBuilderFinish(node_id);
    return 0;
}

#endif // IMGUI_HAS_DOCK

// Logging/Capture
int ImGui_impl_LogToTTY(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);
    ImGui::LogToTTY(auto_open_depth);
    return 0;
}

int ImGui_impl_LogToFile(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);
    const char* filename = luaL_optstring(L, 3, NULL);
    ImGui::LogToFile(auto_open_depth, filename);
    return 0;
}

int ImGui_impl_LogToClipboard(lua_State* L)
{
    int auto_open_depth = luaL_optinteger(L, 2, -1);
    ImGui::LogToClipboard(auto_open_depth);
    return 0;
}

int ImGui_impl_LogFinish(lua_State* _UNUSED(L))
{
    ImGui::LogFinish();
    return 0;
}

int ImGui_impl_LogButtons(lua_State* _UNUSED(L))
{
    ImGui::LogButtons();
    return 0;
}

int ImGui_impl_LogText(lua_State* L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::LogText("%s", text);
    return 0;
}

// Drag and Drop
int ImGui_impl_BeginDragDropSource(lua_State* L)
{
    ImGuiDragDropFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::BeginDragDropSource(flags));
    return 1;
}

int ImGui_impl_SetDragDropPayload(lua_State* L)
{
    const char* type = luaL_checkstring(L, 2);
    double data = luaL_checknumber(L, 3);
    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::SetDragDropPayload(type, (void*)&data, sizeof(double), cond));
    return 1;
}

int ImGui_impl_EndDragDropSource(lua_State* _UNUSED(L))
{
    ImGui::EndDragDropSource();
    return 0;
}

int ImGui_impl_BeginDragDropTarget(lua_State* L)
{
    lua_pushboolean(L, ImGui::BeginDragDropTarget());
    return 1;
}

int ImGui_impl_AcceptDragDropPayload(lua_State* _UNUSED(L))
{
    //const char* type = luaL_checkstring(L, 2);
    //ImGuiDragDropFlags flags = luaL_optinteger(L, 3, 0);

    //const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type, flags);
    // TODO
    return 0;
}

int ImGui_impl_EndDragDropTarget(lua_State* _UNUSED(L))
{
    ImGui::EndDragDropTarget();
    return 0;
}

int ImGui_impl_GetDragDropPayload(lua_State* _UNUSED(L))
{
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    //TODO
    return 0;
}

// Clipping
int ImGui_impl_PushClipRect(lua_State* L)
{
    const ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    const ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = lua_toboolean(L, 6) > 0;
    ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int ImGui_impl_PopClipRect(lua_State* _UNUSED(L))
{
    ImGui::PopClipRect();
    return 0;
}

// Focus, Activation
int ImGui_impl_SetItemDefaultFocus(lua_State* _UNUSED(L))
{
    ImGui::SetItemDefaultFocus();
    return 0;
}

int ImGui_impl_SetKeyboardFocusHere(lua_State* L)
{
    int offset = luaL_optinteger(L, 2, 0);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

// Item/Widgets Utilities
int ImGui_impl_IsItemHovered(lua_State* L)
{
    ImGuiHoveredFlags flags = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsItemHovered(flags));
    return 1;
}

int ImGui_impl_IsItemActive(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemActive());
    return 1;
}

int ImGui_impl_IsItemFocused(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemFocused());
    return 1;
}

int ImGui_impl_IsItemClicked(lua_State* L)
{
    ImGuiMouseButton mouse_button = convertGiderosMouseButton(L, luaL_optinteger(L, 2, 1));
    lua_pushboolean(L, ImGui::IsItemClicked(mouse_button));
    return 1;
}

int ImGui_impl_IsItemVisible(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemVisible());
    return 1;
}

int ImGui_impl_IsItemEdited(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemEdited());
    return 1;
}

int ImGui_impl_IsItemActivated(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemActivated());
    return 1;
}

int ImGui_impl_IsItemDeactivated(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivated());
    return 1;
}

int ImGui_impl_IsItemDeactivatedAfterEdit(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivatedAfterEdit());
    return 1;
}

int ImGui_impl_IsItemToggledOpen(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsItemToggledOpen());
    return 1;
}

int ImGui_impl_IsAnyItemHovered(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemHovered());
    return 1;
}

int ImGui_impl_IsAnyItemActive(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemActive());
    return 1;
}

int ImGui_impl_IsAnyItemFocused(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyItemFocused());
    return 1;
}

int ImGui_impl_GetItemRectMin(lua_State* L)
{
    ImVec2 min = ImGui::GetItemRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_GetItemRectMax(lua_State* L)
{
    ImVec2 max = ImGui::GetItemRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetItemRectSize(lua_State* L)
{
    ImVec2 size = ImGui::GetItemRectSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ImGui_impl_SetItemAllowOverlap(lua_State* _UNUSED(L))
{
    ImGui::SetItemAllowOverlap();
    return 0;
}

// Miscellaneous Utilities
int ImGui_impl_IsRectVisible(lua_State* L)
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

int ImGui_impl_GetTime(lua_State* L)
{
    lua_pushnumber(L, ImGui::GetTime());
    return 1;
}

int ImGui_impl_GetFrameCount(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetFrameCount());
    return 1;
}

int ImGui_impl_GetStyleColorName(lua_State* L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);
    lua_pushstring(L, ImGui::GetStyleColorName(idx));
    return 1;
}

int ImGui_impl_CalcListClipping(lua_State* L)
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

int ImGui_impl_BeginChildFrame(lua_State* L)
{
    ImGuiID id = luaL_checkinteger(L, 2);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiWindowFlags flags = luaL_optinteger(L, 5, 0);

    lua_pushboolean(L, ImGui::BeginChildFrame(id, size, flags));
    return 1;
}

int ImGui_impl_EndChildFrame(lua_State* _UNUSED(L))
{
    ImGui::EndChildFrame();
    return 0;
}


// Text Utilities
int ImGui_impl_CalcTextSize(lua_State* L)
{
    //const char* text, const char* text_end = NULL, , double wrap_width = -1.0f);
    const char* text = luaL_checkstring(L, 2);
    const char* text_end = luaL_optstring(L, 3, NULL);
    bool hide_text_after_double_hash = luaL_optboolean(L, 4, 0);
    float wrap_width = luaL_optnumber(L, 5, -1.0);

    ImVec2 size = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);

    return 2;
}

// Inputs Utilities: Keyboard
int ImGui_impl_GetKeyIndex(lua_State* L)
{
    ImGuiKey imgui_key = luaL_checkinteger(L, 2);
    lua_pushinteger(L, ImGui::GetKeyIndex(imgui_key));
    return  1;
}

int ImGui_impl_IsKeyDown(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyDown(user_key_index));
    return  1;
}

int ImGui_impl_IsKeyPressed(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    bool repeat = luaL_optboolean(L, 3, 1);

    lua_pushboolean(L, ImGui::IsKeyPressed(user_key_index, repeat));
    return 1;
}

int ImGui_impl_IsKeyReleased(lua_State* L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyReleased(user_key_index));
    return 1;
}

int ImGui_impl_GetKeyPressedAmount(lua_State* L)
{
    int key_index = luaL_checkinteger(L, 2);
    double repeat_delay = luaL_checknumber(L, 3);
    double rate = luaL_checknumber(L, 4);
    lua_pushinteger(L, ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate));
    return 1;
}

int ImGui_impl_CaptureKeyboardFromApp(lua_State* L)
{
    bool want_capture_keyboard_value = luaL_optboolean(L, 2, 1);
    ImGui::CaptureKeyboardFromApp(want_capture_keyboard_value);
    return 0;
}


// Inputs Utilities: Mouse
int ImGui_impl_IsMouseDown(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDown(button));
    return 1;
}

int ImGui_impl_IsMouseClicked(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    bool repeat = luaL_optboolean(L, 3, 0);
    lua_pushboolean(L, ImGui::IsMouseClicked(button, repeat));
    return 1;
}

int ImGui_impl_IsMouseReleased(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseReleased(button));
    return 1;
}

int ImGui_impl_IsMouseDoubleClicked(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDoubleClicked(button));
    return 1;
}

int ImGui_impl_IsMouseHoveringRect(lua_State* L)
{
    ImVec2 r_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 r_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool clip = luaL_optboolean(L, 6, 1);
    lua_pushboolean(L, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
    return 1;
}

int ImGui_impl_IsMousePosValid(lua_State* L)
{
    ImVec2 mouse_pos = ImVec2(luaL_optnumber(L, 2, -FLT_MAX), luaL_optnumber(L, 3, -FLT_MAX));
    lua_pushboolean(L, ImGui::IsMousePosValid(&mouse_pos));
    return 1;
}

int ImGui_impl_IsAnyMouseDown(lua_State* L)
{
    lua_pushboolean(L, ImGui::IsAnyMouseDown());
    return 1;
}

int ImGui_impl_GetMousePos(lua_State* L)
{
    ImVec2 pos = ImGui::GetMousePos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return  2;
}

int ImGui_impl_GetMousePosOnOpeningCurrentPopup(lua_State* L)
{
    ImVec2 pos = ImGui::GetMousePosOnOpeningCurrentPopup();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_IsMouseDragging(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, -1.0f);
    lua_pushboolean(L, ImGui::IsMouseDragging(button, lock_threshold));
    return 1;
}

int ImGui_impl_GetMouseDragDelta(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, -1.0f);
    ImVec2 pos = ImGui::GetMouseDragDelta(button, lock_threshold);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_ResetMouseDragDelta(lua_State* L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    ImGui::ResetMouseDragDelta(button);
    return 0;
}

int ImGui_impl_GetMouseCursor(lua_State* L)
{
    lua_pushinteger(L, ImGui::GetMouseCursor());
    return 1;
}

int ImGui_impl_SetMouseCursor(lua_State* L)
{
    ImGuiMouseCursor cursor_type = luaL_checkinteger(L, 2);
    ImGui::SetMouseCursor(cursor_type);
    return 0;
}

int ImGui_impl_CaptureMouseFromApp(lua_State* L)
{
    bool want_capture_mouse_value = luaL_optboolean(L, 2, 1) > 0;
    ImGui::CaptureMouseFromApp(want_capture_mouse_value);
    return 0;
}

/// STYLES

int ImGui_impl_StyleDark(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsDark();
    return 0;
}

int ImGui_impl_StyleLight(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsLight();
    return 0;
}

int ImGui_impl_StyleClassic(lua_State* _UNUSED(L))
{
    ImGui::StyleColorsClassic();
    return 0;
}

/// Color Utilities

int ImGui_impl_ColorConvertHEXtoRGB(lua_State* L)
{
    GColor color = GColor(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    ImVec4 vec = GColor::toVec4(color);

    lua_pushnumber(L, vec.x);
    lua_pushnumber(L, vec.y);
    lua_pushnumber(L, vec.z);
    lua_pushnumber(L, vec.w);
    return 4;
}

int ImGui_impl_ColorConvertRGBtoHEX(lua_State* L)
{
    float r = luaL_checknumber(L, 2);
    float g = luaL_checknumber(L, 3);
    float b = luaL_checknumber(L, 4);

    GColor color = GColor::toHex(r, g, b, 1.0f);

    lua_pushinteger(L, color.hex);
    return 1;
}

int ImGui_impl_ColorConvertRGBtoHSV(lua_State* L)
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

int ImGui_impl_ColorConvertHSVtoRGB(lua_State* L)
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

////////////////////////////////////////////////////////////////////////////////
///
/// DEMOS
///
////////////////////////////////////////////////////////////////////////////////

int ImGui_impl_ShowUserGuide(lua_State* _UNUSED(L))
{
    ImGui::ShowUserGuide();

    return 0;
}

int ImGui_impl_ShowDemoWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowDemoWindow();

    return 0;
}

int ImGui_impl_ShowAboutWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowAboutWindow();

    return 0;
}

int ImGui_impl_ShowStyleEditor(lua_State* _UNUSED(L))
{
    ImGui::ShowStyleEditor();

    return 0;
}

int ImGui_impl_ShowFontSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    ImGui::ShowFontSelector(label);

    return 0;
}

int ImGui_impl_ShowMetricsWindow(lua_State* _UNUSED(L))
{
    ImGui::ShowMetricsWindow();

    return 0;
}

int ImGui_impl_ShowStyleSelector(lua_State* L)
{
    const char* label = luaL_checkstring(L, 2);

    bool open = ImGui::ShowStyleSelector(label);

    lua_pushboolean(L, open);

    return 1;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Style class
int ImGui_impl_GetStyle(lua_State* L)
{
    Binder binder(L);

    ImGuiStyle* style = &ImGui::GetStyle();
    binder.pushInstance(STYLES_CLASS_NAME, style);
    return 1;
}

ImGuiStyle& getStyle(lua_State* L)
{
    Binder binder(L);
    ImGuiStyle &style = *(static_cast<ImGuiStyle*>(binder.getInstance(STYLES_CLASS_NAME, 1)));
    return style;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// AUTO GENERATED STYLE METHODS ////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

int ImGui_impl_Style_old_SetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    if (idx < 0 || idx > ImGuiCol_COUNT - 1)
    {
        lua_pushstring(L, "Color index is out of bounds.");
        lua_error(L);
    }

    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[idx] = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    return 0;
}

int ImGui_impl_Style_SetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    if (idx < 0 || idx > ImGuiCol_COUNT - 1)
    {
        lua_pushstring(L, "Color index is out of bounds.");
        lua_error(L);
    }

    ImGuiStyle &style = getStyle(L);
    style.Colors[idx] = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    return 0;
}

int ImGui_impl_Style_GetColor(lua_State* L)
{
    int idx = luaL_checkinteger(L, 2);
    if (idx < 0 || idx > ImGuiCol_COUNT - 1)
    {
        lua_pushstring(L, "Color index is out of bounds.");
        lua_error(L);
    }

    ImGuiStyle &style = getStyle(L);
    GColor color = GColor::toHex(style.Colors[idx]);
    lua_pushinteger(L, color.hex);
    lua_pushnumber(L, color.alpha);
    return 2;
}

int ImGui_impl_Style_SetAlpha(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.Alpha = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetAlpha(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.Alpha);
    return 1;
}

int ImGui_impl_Style_SetWindowRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetWindowRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowRounding);
    return 1;
}

int ImGui_impl_Style_SetWindowBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetWindowBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowBorderSize);
    return 1;
}

int ImGui_impl_Style_SetChildRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ChildRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetChildRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ChildRounding);
    return 1;
}

int ImGui_impl_Style_SetChildBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ChildBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetChildBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ChildBorderSize);
    return 1;
}

int ImGui_impl_Style_SetPopupRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.PopupRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetPopupRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.PopupRounding);
    return 1;
}

int ImGui_impl_Style_SetPopupBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.PopupBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetPopupBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.PopupBorderSize);
    return 1;
}

int ImGui_impl_Style_SetFrameRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FrameRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetFrameRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FrameRounding);
    return 1;
}

int ImGui_impl_Style_SetFrameBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FrameBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetFrameBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FrameBorderSize);
    return 1;
}

int ImGui_impl_Style_SetIndentSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.IndentSpacing = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetIndentSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.IndentSpacing);
    return 1;
}

int ImGui_impl_Style_SetColumnsMinSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ColumnsMinSpacing = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetColumnsMinSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ColumnsMinSpacing);
    return 1;
}

int ImGui_impl_Style_SetScrollbarSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ScrollbarSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetScrollbarSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ScrollbarSize);
    return 1;
}

int ImGui_impl_Style_SetScrollbarRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ScrollbarRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetScrollbarRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ScrollbarRounding);
    return 1;
}

int ImGui_impl_Style_SetGrabMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.GrabMinSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetGrabMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.GrabMinSize);
    return 1;
}

int ImGui_impl_Style_SetGrabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.GrabRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetGrabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.GrabRounding);
    return 1;
}

int ImGui_impl_Style_SetLogSliderDeadzone(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.LogSliderDeadzone = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetLogSliderDeadzone(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.LogSliderDeadzone);
    return 1;
}

int ImGui_impl_Style_SetTabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabRounding = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetTabRounding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabRounding);
    return 1;
}

int ImGui_impl_Style_SetTabBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabBorderSize = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetTabBorderSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabBorderSize);
    return 1;
}

int ImGui_impl_Style_SetTabMinWidthForUnselectedCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TabMinWidthForUnselectedCloseButton = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetTabMinWidthForUnselectedCloseButton(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TabMinWidthForUnselectedCloseButton);
    return 1;
}

int ImGui_impl_Style_SetMouseCursorScale(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.MouseCursorScale = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetMouseCursorScale(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.MouseCursorScale);
    return 1;
}

int ImGui_impl_Style_SetCurveTessellationTol(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.CurveTessellationTol = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetCurveTessellationTol(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.CurveTessellationTol);
    return 1;
}

int ImGui_impl_Style_SetCircleSegmentMaxError(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.CircleSegmentMaxError = luaL_checknumber(L, 2);
    return 0;
}

int ImGui_impl_Style_GetCircleSegmentMaxError(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.CircleSegmentMaxError);
    return 1;
}

int ImGui_impl_Style_SetWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowPadding.x);
    lua_pushnumber(L, style.WindowPadding.y);
    return 2;
}

int ImGui_impl_Style_SetWindowMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowMinSize = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetWindowMinSize(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowMinSize.x);
    lua_pushnumber(L, style.WindowMinSize.y);
    return 2;
}

int ImGui_impl_Style_SetWindowTitleAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowTitleAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetWindowTitleAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.WindowTitleAlign.x);
    lua_pushnumber(L, style.WindowTitleAlign.y);
    return 2;
}

int ImGui_impl_Style_SetFramePadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.FramePadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetFramePadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.FramePadding.x);
    lua_pushnumber(L, style.FramePadding.y);
    return 2;
}

int ImGui_impl_Style_SetItemSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ItemSpacing = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetItemSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ItemSpacing.x);
    lua_pushnumber(L, style.ItemSpacing.y);
    return 2;
}

int ImGui_impl_Style_SetItemInnerSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ItemInnerSpacing = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetItemInnerSpacing(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ItemInnerSpacing.x);
    lua_pushnumber(L, style.ItemInnerSpacing.y);
    return 2;
}

int ImGui_impl_Style_SetTouchExtraPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.TouchExtraPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetTouchExtraPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.TouchExtraPadding.x);
    lua_pushnumber(L, style.TouchExtraPadding.y);
    return 2;
}

int ImGui_impl_Style_SetButtonTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ButtonTextAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetButtonTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.ButtonTextAlign.x);
    lua_pushnumber(L, style.ButtonTextAlign.y);
    return 2;
}

int ImGui_impl_Style_SetSelectableTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.SelectableTextAlign = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetSelectableTextAlign(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.SelectableTextAlign.x);
    lua_pushnumber(L, style.SelectableTextAlign.y);
    return 2;
}

int ImGui_impl_Style_SetDisplayWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.DisplayWindowPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetDisplayWindowPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.DisplayWindowPadding.x);
    lua_pushnumber(L, style.DisplayWindowPadding.y);
    return 2;
}

int ImGui_impl_Style_SetDisplaySafeAreaPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.DisplaySafeAreaPadding = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    return 0;
}

int ImGui_impl_Style_GetDisplaySafeAreaPadding(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushnumber(L, style.DisplaySafeAreaPadding.x);
    lua_pushnumber(L, style.DisplaySafeAreaPadding.y);
    return 2;
}

int ImGui_impl_Style_SetWindowMenuButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.WindowMenuButtonPosition = luaL_checkinteger(L, 2);
    return 0;
}

int ImGui_impl_Style_GetWindowMenuButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushinteger(L, style.WindowMenuButtonPosition);
    return 1;
}

int ImGui_impl_Style_SetColorButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.ColorButtonPosition = luaL_checkinteger(L, 2);
    return 0;
}

int ImGui_impl_Style_GetColorButtonPosition(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushinteger(L, style.ColorButtonPosition);
    return 1;
}

int ImGui_impl_Style_SetAntiAliasedLines(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedLines = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_Style_GetAntiAliasedLines(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedLines);
    return 1;
}

int ImGui_impl_Style_SetAntiAliasedLinesUseTex(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedLinesUseTex = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_Style_GetAntiAliasedLinesUseTex(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedLinesUseTex);
    return 1;
}

int ImGui_impl_Style_SetAntiAliasedFill(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    style.AntiAliasedFill = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_Style_GetAntiAliasedFill(lua_State* L)
{
    ImGuiStyle &style = getStyle(L);
    lua_pushboolean(L, style.AntiAliasedFill);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ImFontAtlas* getFontAtlas(lua_State* L, int index = 1)
{
    Binder binder(L);
    return static_cast<ImFontAtlas*>(binder.getInstance(FONT_ATLAS_CLASS_NAME, index));
}

ImFont* getFont(lua_State* L, int index = 1)
{
    Binder binder(L);
    ImFont* font = static_cast<ImFont*>(binder.getInstance(FONT_CLASS_NAME, index));
    LUA_ASSERT(L, font, "Font is nil!");
    return font;
}

ImGuiIO& getIO(lua_State* L, int index = 1)
{
    Binder binder(L);
    ImGuiIO &io = *(static_cast<ImGuiIO*>(binder.getInstance(IO_CLASS_NAME, index)));
    return io;
}

int ImGui_impl_GetIO(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance(IO_CLASS_NAME, &ImGui::GetIO());
    return 1;
}

#ifdef IMGUI_HAS_DOCK
int ImGui_impl_IO_GetConfigDockingNoSplit(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingNoSplit);
    return 0;
}

int ImGui_impl_IO_SetConfigDockingNoSplit(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingNoSplit = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_IO_GetConfigDockingWithShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingWithShift);
    return 0;
}

int ImGui_impl_IO_SetConfigDockingWithShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingWithShift = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_IO_GetConfigDockingAlwaysTabBar(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingAlwaysTabBar);
    return 0;
}

int ImGui_impl_IO_SetConfigDockingAlwaysTabBar(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingAlwaysTabBar = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_IO_GetConfigDockingTransparentPayload(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigDockingTransparentPayload);
    return 0;
}

int ImGui_impl_IO_SetConfigDockingTransparentPayload(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.ConfigDockingTransparentPayload = lua_toboolean(L, 2) > 0;
    return 0;
}
#endif

int ImGui_impl_IO_SetFontDefault(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    ImFont* font = getFont(L, 2);
    if (font)
        io.FontDefault = font;
    return 0;
}

int ImGui_impl_IO_GetFonts(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    Binder binder(L);
    binder.pushInstance(FONT_ATLAS_CLASS_NAME, io.Fonts);
    return 1;
}

int ImGui_impl_IO_GetDeltaTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DeltaTime);
    return 1;
}

int ImGui_impl_IO_GetMouseWheel(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseWheel);
    return 1;
}

int ImGui_impl_IO_GetMouseWheelH(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseWheelH);
    return 1;
}

int ImGui_impl_IO_isMouseDown(lua_State* L)
{
    int button = convertGiderosMouseButton(L, luaL_checkinteger(L, 2));
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.MouseDown[button]);
    return  1;
}

int ImGui_impl_IO_isKeyCtrl(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyCtrl);
    return 1;
}

int ImGui_impl_IO_isKeyShift(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyShift);
    return 1;
}

int ImGui_impl_IO_isKeyAlt(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeyAlt);
    return 1;
}

int ImGui_impl_IO_isKeySuper(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeySuper);
    return 1;
}

int ImGui_impl_IO_GetKeysDown(lua_State* L)
{
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > 512)
    {
        lua_pushstring(L, "KeyDown index is out of bounds!");
        lua_error(L);
    }
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.KeysDown[index]);
    return 1;
}

int ImGui_impl_IO_WantCaptureMouse(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantCaptureMouse);
    return 1;
}

int ImGui_impl_IO_WantCaptureKeyboard(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantCaptureKeyboard);
    return 1;
}

int ImGui_impl_IO_WantTextInput(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantTextInput);
    return 1;
}

int ImGui_impl_IO_WantSetMousePos(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantSetMousePos);
    return 1;
}

int ImGui_impl_IO_WantSaveIniSettings(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.WantSaveIniSettings);
    return 1;
}

int ImGui_impl_IO_IsNavActive(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.NavActive);
    return 1;
}

int ImGui_impl_IO_IsNavVisible(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushboolean(L, io.NavVisible);
    return 1;
}

int ImGui_impl_IO_GetFramerate(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushnumber(L, io.Framerate);
    return 1;
}

int ImGui_impl_IO_GetMetricsRenderVertices(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderVertices);
    return 1;
}

int ImGui_impl_IO_GetMetricsRenderIndices(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderIndices);
    return 1;
}

int ImGui_impl_IO_GetMetricsRenderWindows(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsRenderWindows);
    return 1;
}

int ImGui_impl_IO_GetMetricsActiveWindows(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsActiveWindows);
    return 1;
}

int ImGui_impl_IO_GetMetricsActiveAllocations(lua_State* L)
{
    ImGuiIO& io = getIO(L);

    lua_pushinteger(L, io.MetricsActiveAllocations);
    return 1;
}

int ImGui_impl_IO_GetMouseDelta(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDelta.x);
    lua_pushnumber(L, io.MouseDelta.y);
    return 2;
}

int ImGui_impl_IO_GetMouseDownSec(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int button = convertGiderosMouseButton(L, lua_tointeger(L, 2));

    lua_pushnumber(L, io.MouseDownDuration[button]);
    return 1;
}

int ImGui_impl_IO_SetDisplaySize(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.DisplaySize.x = luaL_checknumber(L, 2);
    io.DisplaySize.y = luaL_checknumber(L, 3);

    return 0;
}

int ImGui_impl_IO_GetDisplaySize(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DisplaySize.x);
    lua_pushnumber(L, io.DisplaySize.y);

    return 2;
}


int ImGui_impl_IO_GetConfigFlags(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushinteger(L, io.ConfigFlags);
    return 1;
}

int ImGui_impl_IO_SetConfigFlags(lua_State* L)
{
    ImGuiConfigFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.ConfigFlags = flags;
    return 0;
}

int ImGui_impl_IO_AddConfigFlags(lua_State* L)
{
    ImGuiConfigFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.ConfigFlags |= flags;
    return 0;
}

int ImGui_impl_IO_GetBackendFlags(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushinteger(L, io.BackendFlags);
    return 1;
}

int ImGui_impl_IO_SetBackendFlags(lua_State* L)
{
    ImGuiBackendFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = getIO(L);
    io.BackendFlags = flags;
    return 0;
}

int ImGui_impl_IO_GetIniSavingRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.IniSavingRate);
    return 1;
}

int ImGui_impl_IO_SetIniSavingRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.IniSavingRate = luaL_optnumber(L, 2, 5.0f);
    return 1;
}

int ImGui_impl_IO_GetIniFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.IniFilename);
    return 1;
}

int ImGui_impl_IO_SetIniFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.IniFilename = luaL_optstring(L, 2, "imgui.ini");
    return 0;
}

int ImGui_impl_IO_GetLogFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.LogFilename);
    return 1;
}

int ImGui_impl_IO_SetLogFilename(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.LogFilename = luaL_optstring(L, 2, "imgui_log.txt");
    return 0;
}

int ImGui_impl_IO_GetMouseDoubleClickTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDoubleClickTime);
    return 1;
}

int ImGui_impl_IO_SetMouseDoubleClickTime(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDoubleClickTime = luaL_optnumber(L, 2, 0.30f);
    return 0;
}

int ImGui_impl_IO_GetMouseDragThreshold(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDragThreshold);
    return 1;
}

int ImGui_impl_IO_SetMouseDragThreshold(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDragThreshold = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int ImGui_impl_IO_GetMouseDrawCursor(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.MouseDrawCursor);
    return 1;
}

int ImGui_impl_IO_SetMouseDrawCursor(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDrawCursor = lua_toboolean(L, 2) > 0;
    if (io.MouseDrawCursor)
    {
        setApplicationCursor(L, "blank");
    }
    else
    {
        setApplicationCursor(L, "arrow");
    }
    return 0;
}

int ImGui_impl_IO_GetMouseDoubleClickMaxDist(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.MouseDoubleClickMaxDist);
    return 1;
}

int ImGui_impl_IO_SetMouseDoubleClickMaxDist(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.MouseDoubleClickMaxDist = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int ImGui_impl_IO_GetKeyMapValue(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > ImGuiKey_COUNT)
    {
        lua_pushstring(L, "KeyMap index is out of bounds!");
        lua_error(L);
    }
    lua_pushinteger(L, io.KeyMap[index]);
    return 1;
}

int ImGui_impl_IO_SetKeyMapValue(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > ImGuiKey_COUNT)
    {
        lua_pushstring(L, "KeyMap index is out of bounds!");
        lua_error(L);
    }

    io.KeyMap[index] = luaL_checkinteger(L, 3);
    return 0;
}

int ImGui_impl_IO_GetKeyRepeatDelay(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.KeyRepeatDelay);
    return 1;
}

int ImGui_impl_IO_SetKeyRepeatDelay(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.KeyRepeatDelay = luaL_optnumber(L, 2, 0.25f);
    return 0;
}

int ImGui_impl_IO_GetKeyRepeatRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.KeyRepeatRate);
    return 1;
}

int ImGui_impl_IO_SetKeyRepeatRate(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.KeyRepeatRate = luaL_optnumber(L, 2, 0.05f);
    return 0;
}

int ImGui_impl_IO_GetFontGlobalScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.FontGlobalScale);
    return 1;
}

int ImGui_impl_IO_SetFontGlobalScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.FontGlobalScale = luaL_optnumber(L, 2, 1.0f);
    return 0;
}

int ImGui_impl_IO_GetFontAllowUserScaling(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.FontAllowUserScaling);
    return 1;
}

int ImGui_impl_IO_SetFontAllowUserScaling(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    io.FontAllowUserScaling = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_IO_GetDisplayFramebufferScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.DisplayFramebufferScale.x);
    lua_pushnumber(L, io.DisplayFramebufferScale.y);
    return 2;
}

int ImGui_impl_IO_SetDisplayFramebufferScale(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    ImVec2 scale = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    io.DisplayFramebufferScale = scale;
    return 0;
}

int ImGui_impl_IO_GetConfigMacOSXBehaviors(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigMacOSXBehaviors);
    return 1;
}

int ImGui_impl_IO_SetConfigMacOSXBehaviors(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigMacOSXBehaviors = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigInputTextCursorBlink(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int ImGui_impl_IO_SetConfigInputTextCursorBlink(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigInputTextCursorBlink = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsResizeFromEdges(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsResizeFromEdges(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsResizeFromEdges = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsMoveFromTitleBarOnly(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushboolean(L, io.ConfigWindowsMoveFromTitleBarOnly);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsMoveFromTitleBarOnly(lua_State* L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsMoveFromTitleBarOnly = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsMemoryCompactTimer(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushnumber(L, io.ConfigWindowsMemoryCompactTimer);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsMemoryCompactTimer(lua_State* L)
{
    double t = luaL_optnumber(L, 2, -1.0f);

    ImGuiIO& io = getIO(L);
    io.ConfigWindowsMemoryCompactTimer = t;
    return 0;
}

int ImGui_impl_IO_GetBackendPlatformName(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.BackendPlatformName);
    return 1;
}

int ImGui_impl_IO_GetBackendRendererName(lua_State* L)
{
    ImGuiIO& io = getIO(L);
    lua_pushstring(L, io.BackendRendererName);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

struct FontData
{
    void* data;
    size_t size;

    FontData(void* p_data, size_t p_size)
    {
        data = p_data;
        size = p_size;
    }
};

G_FILE* GFileOpen(const char* filename, const char* mode)
{
    return g_fopen(filename, mode);
}

bool GFileClose(G_FILE* f)
{
    return g_fclose(f) == 0;
}

size_t GFileGetSize(G_FILE* f)
{
    long off = 0;
    long sz = 0;
    return ((off = g_ftell(f)) != -1 && !g_fseek(f, 0, SEEK_END) && (sz = g_ftell(f)) != -1 && !g_fseek(f, off, SEEK_SET)) ? (size_t)sz : (size_t)-1;
}

size_t GFileRead(void* data, ImU64 sz, ImU64 count, G_FILE* f)
{
    return g_fread(data, (size_t)sz, (size_t)count, f);
}

size_t GFileWrite(const void* data, ImU64 sz, ImU64 count, G_FILE* f)
{
    return g_fwrite(data, (size_t)sz, (size_t)count, f);
}

void* GFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size, int padding_bytes)
{
    IM_ASSERT(filename && mode);
    if (out_file_size)
       * out_file_size = 0;

    G_FILE* f;
    if ((f = GFileOpen(filename, mode)) == NULL)
    {
        return NULL;
    }

    size_t file_size = (size_t)GFileGetSize(f);
    if (file_size == (size_t)-1)
    {
        GFileClose(f);
        return NULL;
    }

    void* file_data = IM_ALLOC(file_size + padding_bytes);
    if (file_data == NULL)
    {
        GFileClose(f);
        return NULL;
    }
    if (GFileRead(file_data, 1, file_size, f) != file_size)
    {
        GFileClose(f);
        IM_FREE(file_data);
        return NULL;
    }
    if (padding_bytes > 0)
        memset((void*)(((char*)file_data) + file_size), 0, (size_t)padding_bytes);

    GFileClose(f);
    if (out_file_size)
       * out_file_size = file_size;

    return file_data;
}

FontData getFontData(lua_State* L, const char* filename)
{
    size_t data_size = 0;
    void* data = GFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        std::string str = "Cant load '";
        str.append(filename);
        str.append("' font! File not found.");
        lua_pushstring(L, str.c_str());
        lua_error(L);
    }

    return FontData(data, data_size);
}
/*
void _addFonts(lua_State* L, int idx)
{
    int len = luaL_getn(L, idx);
    lua_pushvalue(L, idx);
    for (int i = 1; i <= len; i++)
    {
        lua_rawgeti(L, 4, i); // { fontPath, fontSize, OversampleH, OversampleV, GlyphExtraSpacingX, GlyphExtraSpacingY }

        lua_rawgeti(L, -1, 1);
        const char* filename = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2); // font size
        double size_pixel = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        int oversampleH = 1;
        lua_rawgeti(L, -1, 3); // optional OversampleH
        if (!lua_isnil(L, -1)) oversampleH = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        int oversampleV = 1;
        lua_rawgeti(L, -1, 4); // optional OversampleV
        if (!lua_isnil(L, -1)) oversampleV = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        double spacingX = 0.0f;
        lua_rawgeti(L, -1, 5); // optional GlyphExtraSpacingX
        if (!lua_isnil(L, -1)) spacingX = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        double spacingY = 0.0f;
        lua_rawgeti(L, -1, 6); // optional GlyphExtraSpacingY
        if (!lua_isnil(L, -1)) spacingY = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        GAddFontFromFileTTF(L, filename, size_pixel, oversampleH, oversampleV, spacingX, spacingY);

        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}
*/


/// FONTS API

int ImGui_impl_Fonts_PushFont(lua_State* L)
{
    Binder binder(L);
    ImFont* font = static_cast<ImFont*>(binder.getInstance(FONT_CLASS_NAME, 2));
    LUA_ASSERT(L, font, "Font is nil");
    ImGui::PushFont(font);
    return 0;
}

int ImGui_impl_Fonts_PopFont(lua_State* _UNUSED(L))
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
void addConfCustomRanges(ImFontGlyphRangesBuilder &builder, ImFontAtlas* atlas, int value)
{

}

static void loadFontConfig(lua_State* L, int index, ImFontConfig &config, ImFontAtlas* atlas)
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
        readConfTable(L, "customRanges", builder, atlas, addConfCustomRanges);

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

    FontData font_data = getFontData(L, file_name);
    return atlas->AddFontFromMemoryTTF(font_data.data, font_data.size, size_pixels, &font_cfg);
}

int ImGui_impl_FontAtlas_AddFont(lua_State* L)
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
    binder.pushInstance(FONT_CLASS_NAME, font);
    return 1;
}

int ImGui_impl_FontAtlas_AddFonts(lua_State* L)
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

int ImGui_impl_FontAtlas_GetFontByIndex(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    int index = 0;
    if (lua_gettop(L) > 1 && !lua_isnil(L, 2))
    {
        index = luaL_checkinteger(L, 2);
    }
    int fonts_count = atlas->Fonts.Size;
    LUA_ASSERT(L, index >= 0 && index < fonts_count, "Font index is out of bounds!");
    ImFont* font = atlas->Fonts[index];
    LUA_ASSERT(L, font, "Font is nil");
    Binder binder(L);
    binder.pushInstance(FONT_CLASS_NAME, font);
    return 1;
}

int ImGui_impl_FontAtlas_GetCurrentFont(lua_State* L)
{
    Binder binder(L);
    binder.pushInstance(FONT_CLASS_NAME, ImGui::GetFont());
    return 1;
}

int ImGui_impl_FontAtlas_AddDefaultFont(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->AddFontDefault();
    return 0;
}

int ImGui_impl_FontAtlas_BuildFont(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->Build();
    return 0;
}

int ImGui_impl_FontAtlas_Bake(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);

    unsigned char* pixels;
    int width, height;
    atlas->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_id texture = gtexture_create(width, height, GTEXTURE_RGBA, GTEXTURE_UNSIGNED_BYTE, GTEXTURE_CLAMP, GTEXTURE_LINEAR, pixels, NULL, 0);
    atlas->TexID = (void *)texture;

    return 0;
}

int ImGui_impl_FontAtlas_ClearInputData(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearInputData();
    return 0;
}

int ImGui_impl_FontAtlas_ClearTexData(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearTexData();
    return 0;
}

int ImGui_impl_FontAtlas_ClearFonts(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->ClearFonts();
    return 0;
}

int ImGui_impl_FontAtlas_Clear(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    atlas->Clear();
    return 0;
}

int ImGui_impl_FontAtlas_IsBuilt(lua_State* L)
{
    ImFontAtlas* atlas = getFontAtlas(L);
    lua_pushboolean(L, atlas->IsBuilt());
    return 1;
}

int ImGui_impl_FontAtlas_AddCustomRectRegular(lua_State* L)
{
    int width  = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);
    ImFontAtlas* atlas = getFontAtlas(L);
    lua_pushinteger(L, atlas->AddCustomRectRegular(width, height));
    return 1;
}

int ImGui_impl_FontAtlas_AddCustomRectFontGlyph(lua_State* L)
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

int ImGui_impl_FontAtlas_GetCustomRectByIndex(lua_State* L)
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
    binder.pushInstance(FONT_CLASS_NAME, rect->Font);
    lua_pushboolean(L, rect->IsPacked());
    return 10;
}

////////////////////////////////////////////////////////////////////////////////
///
/// DRAW LIST
///
////////////////////////////////////////////////////////////////////////////////

int ImGui_impl_GetWindowDrawList(lua_State* L)
{
    // TODO: add assertion 'called before "ImGui:newFrame()"

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    binder.pushInstance(DRAW_LIST_CLASS_NAME, draw_list);
    return 1;
}

int ImGui_impl_GetBackgroundDrawList(lua_State* L)
{
    // TODO: add assertion 'called before "ImGui:newFrame()"

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    binder.pushInstance(DRAW_LIST_CLASS_NAME, draw_list);
    return 1;
}

int ImGui_impl_GetForegroundDrawList(lua_State* L)
{
    // TODO: add assertion 'called before "ImGui:newFrame()"

    Binder binder(L);
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    binder.pushInstance(DRAW_LIST_CLASS_NAME, draw_list);
    return 1;
}

ImDrawList* getDrawList(lua_State* L)
{
    Binder binder(L);
    return static_cast<ImDrawList*>(binder.getInstance(DRAW_LIST_CLASS_NAME, 1));
}

int ImGui_impl_DrawList_PushClipRect(lua_State* L)
{
    ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = luaL_optboolean(L, 6, 0) > 0;

    ImDrawList* list = getDrawList(L);
    list->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int ImGui_impl_DrawList_PushClipRectFullScreen(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PushClipRectFullScreen();
    return 0;
}

int ImGui_impl_DrawList_PopClipRect(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PopClipRect();
    return 0;
}

int ImGui_impl_DrawList_PushTextureID(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImTextureID texture_id = getTexture(L, 2).texture;
    list->PushTextureID(texture_id);
    return 0;
}

int ImGui_impl_DrawList_PopTextureID(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PopTextureID();
    return 0;
}

int ImGui_impl_DrawList_GetClipRectMin(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImVec2 min = list->GetClipRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_DrawList_GetClipRectMax(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    ImVec2 max = list->GetClipRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_DrawList_AddLine(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    double thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList* list = getDrawList(L);
    list->AddLine(p1, p2, col, thickness);
    return 0;
}

int ImGui_impl_DrawList_AddRect(lua_State* L)
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

int ImGui_impl_DrawList_AddRectFilled(lua_State* L)
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

int ImGui_impl_DrawList_AddRectFilledMultiColor(lua_State* L)
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

int ImGui_impl_DrawList_AddQuad(lua_State* L)
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

int ImGui_impl_DrawList_AddQuadFilled(lua_State* L)
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

int ImGui_impl_DrawList_AddTriangle(lua_State* L)
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

int ImGui_impl_DrawList_AddTriangleFilled(lua_State* L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddTriangleFilled(p1, p2, p3, col);

    return  0;
}

int ImGui_impl_DrawList_AddCircle(lua_State* L)
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

int ImGui_impl_DrawList_AddCircleFilled(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList* list = getDrawList(L);
    list->AddCircleFilled(center, radius, col, num_segments);

    return 0;
}

int ImGui_impl_DrawList_AddNgon(lua_State* L)
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

int ImGui_impl_DrawList_AddNgonFilled(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList* list = getDrawList(L);
    list->AddNgonFilled(center, radius, col, num_segments);

    return 0;
}

int ImGui_impl_DrawList_AddText(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 4), luaL_optnumber(L, 5, 1.0f));
    const char* text_begin = luaL_checkstring(L, 6);
    const char* text_end = luaL_optstring(L, 7, NULL);

    ImDrawList* list = getDrawList(L);
    list->AddText(pos, col, text_begin, text_end);

    return 0;
}

int ImGui_impl_DrawList_AddFontText(lua_State* L)
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

int ImGui_impl_DrawList_AddPolyline(lua_State* L)
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

int ImGui_impl_DrawList_AddConvexPolyFilled(lua_State* L)
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

int ImGui_impl_DrawList_AddBezierCurve(lua_State* L)
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

int ImGui_impl_DrawList_AddImage(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 1.0f));
    ImVec2 uv_min = ImVec2(luaL_optnumber(L,  9, 0.0f), luaL_optnumber(L, 10, 0.0f));
    ImVec2 uv_max = ImVec2(luaL_optnumber(L, 11, 1.0f), luaL_optnumber(L, 12, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddImage(data.texture, p_min, p_max, uv_min * data.uv, uv_max * data.uv, col);
    return 0;
}

int ImGui_impl_DrawList_AddImageQuad(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 7), luaL_checknumber(L, 8));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 9), luaL_checknumber(L, 10));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 11, 0xffffff), luaL_optnumber(L, 12, 1.0f));
    ImVec2 uv1 = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));
    ImVec2 uv2 = ImVec2(luaL_optnumber(L, 15, 1.0f), luaL_optnumber(L, 16, 0.0f));
    ImVec2 uv3 = ImVec2(luaL_optnumber(L, 17, 1.0f), luaL_optnumber(L, 18, 1.0f));
    ImVec2 uv4 = ImVec2(luaL_optnumber(L, 19, 0.0f), luaL_optnumber(L, 20, 1.0f));

    uv1 *= data.uv;
    uv2 *= data.uv;
    uv3 *= data.uv;
    uv4 *= data.uv;

    ImDrawList* list = getDrawList(L);
    list->AddImageQuad(data.texture, p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);
    return 0;
}

int ImGui_impl_DrawList_AddImageRounded(lua_State* L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 7), luaL_optnumber(L, 8, 1.0f));
    double rounding = luaL_checknumber(L, 9);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 10, ImDrawCornerFlags_All);
    ImVec2 uv_min = ImVec2(luaL_optnumber(L, 11, 0.0f), luaL_optnumber(L, 12, 0.0f));
    ImVec2 uv_max = ImVec2(luaL_optnumber(L, 13, 1.0f), luaL_optnumber(L, 14, 1.0f));

    ImDrawList* list = getDrawList(L);
    list->AddImageRounded(data.texture, p_min, p_max, uv_min * data.uv, uv_max * data.uv, col, rounding, rounding_corners);
    return 0;
}

int ImGui_impl_DrawList_PathClear(lua_State* L)
{
    ImDrawList* list = getDrawList(L);
    list->PathClear();
    return 0;
}

int ImGui_impl_DrawList_PathLineTo(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList* list = getDrawList(L);
    list->PathLineTo(pos);
    return 0;
}

int ImGui_impl_DrawList_PathLineToMergeDuplicate(lua_State* L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList* list = getDrawList(L);
    list->PathLineToMergeDuplicate(pos);
    return 0;
}

int ImGui_impl_DrawList_PathFillConvex(lua_State* L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    ImDrawList* list = getDrawList(L);
    list->PathFillConvex(color);
    return 0;

}

int ImGui_impl_DrawList_PathStroke(lua_State* L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    bool closed = lua_toboolean(L, 4) > 0;
    float thickness = luaL_optnumber(L, 3, 1.0f);
    ImDrawList* list = getDrawList(L);
    list->PathStroke(color, closed, thickness);
    return 0;
}

int ImGui_impl_DrawList_PathArcTo(lua_State* L)
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

int ImGui_impl_DrawList_PathArcToFast(lua_State* L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    double radius = luaL_checknumber(L, 4);
    int a_min = luaL_checkinteger(L, 5);
    int a_max = luaL_checkinteger(L, 6);
    ImDrawList* list = getDrawList(L);
    list->PathArcToFast(center, radius, a_min, a_max);
    return 0;

}

int ImGui_impl_DrawList_PathBezierCurveTo(lua_State* L)
{
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    int num_segments = luaL_optinteger(L, 8, 0);
    ImDrawList* list = getDrawList(L);
    list->PathBezierCurveTo(p2, p3, p4, num_segments);
    return 0;
}

int ImGui_impl_DrawList_PathRect(lua_State* L)
{
    ImVec2 rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    double rounding = luaL_optnumber(L, 6, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 7, ImDrawCornerFlags_All);
    ImDrawList* list = getDrawList(L);
    list->PathRect(rect_min, rect_max, rounding, rounding_corners);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
/*
int ImGui_my_Test2(lua_State* L)
{
    float value = luaL_optnumber(L, 2, NULL);

    LUA_ASSERT(L, value < 0, "bad argument #2");

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

int ImGui_my_ShowLuaStyleEditor(lua_State* _UNUSED(L))
{

    ImGui::Begin("Style editor [CUSTOM]", NULL);

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

        static int output_dest = 0;

        // Save/Revert button
        if (ImGui::Button("Save Ref"))
           * ref = ref_saved_style = style;
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

    return 0;
}

int loader(lua_State* L)
{
    Binder binder(L);

    const luaL_Reg imguiStylesFunctionList[] =
    {
        {"setColor", ImGui_impl_Style_SetColor},
        {"getColor", ImGui_impl_Style_GetColor},
        {"setAlpha", ImGui_impl_Style_SetAlpha},
        {"getAlpha", ImGui_impl_Style_GetAlpha},
        {"setWindowRounding", ImGui_impl_Style_SetWindowRounding},
        {"getWindowRounding", ImGui_impl_Style_GetWindowRounding},
        {"setWindowBorderSize", ImGui_impl_Style_SetWindowBorderSize},
        {"getWindowBorderSize", ImGui_impl_Style_GetWindowBorderSize},
        {"setChildRounding", ImGui_impl_Style_SetChildRounding},
        {"getChildRounding", ImGui_impl_Style_GetChildRounding},
        {"setChildBorderSize", ImGui_impl_Style_SetChildBorderSize},
        {"getChildBorderSize", ImGui_impl_Style_GetChildBorderSize},
        {"setPopupRounding", ImGui_impl_Style_SetPopupRounding},
        {"getPopupRounding", ImGui_impl_Style_GetPopupRounding},
        {"setPopupBorderSize", ImGui_impl_Style_SetPopupBorderSize},
        {"getPopupBorderSize", ImGui_impl_Style_GetPopupBorderSize},
        {"setFrameRounding", ImGui_impl_Style_SetFrameRounding},
        {"getFrameRounding", ImGui_impl_Style_GetFrameRounding},
        {"setFrameBorderSize", ImGui_impl_Style_SetFrameBorderSize},
        {"getFrameBorderSize", ImGui_impl_Style_GetFrameBorderSize},
        {"setIndentSpacing", ImGui_impl_Style_SetIndentSpacing},
        {"getIndentSpacing", ImGui_impl_Style_GetIndentSpacing},
        {"setColumnsMinSpacing", ImGui_impl_Style_SetColumnsMinSpacing},
        {"getColumnsMinSpacing", ImGui_impl_Style_GetColumnsMinSpacing},
        {"setScrollbarSize", ImGui_impl_Style_SetScrollbarSize},
        {"getScrollbarSize", ImGui_impl_Style_GetScrollbarSize},
        {"setScrollbarRounding", ImGui_impl_Style_SetScrollbarRounding},
        {"getScrollbarRounding", ImGui_impl_Style_GetScrollbarRounding},
        {"setGrabMinSize", ImGui_impl_Style_SetGrabMinSize},
        {"getGrabMinSize", ImGui_impl_Style_GetGrabMinSize},
        {"setGrabRounding", ImGui_impl_Style_SetGrabRounding},
        {"getGrabRounding", ImGui_impl_Style_GetGrabRounding},
        {"setLogSliderDeadzone", ImGui_impl_Style_SetLogSliderDeadzone},
        {"getLogSliderDeadzone", ImGui_impl_Style_GetLogSliderDeadzone},
        {"setTabRounding", ImGui_impl_Style_SetTabRounding},
        {"getTabRounding", ImGui_impl_Style_GetTabRounding},
        {"setTabBorderSize", ImGui_impl_Style_SetTabBorderSize},
        {"getTabBorderSize", ImGui_impl_Style_GetTabBorderSize},
        {"setTabMinWidthForUnselectedCloseButton", ImGui_impl_Style_SetTabMinWidthForUnselectedCloseButton},
        {"getTabMinWidthForUnselectedCloseButton", ImGui_impl_Style_GetTabMinWidthForUnselectedCloseButton},
        {"setMouseCursorScale", ImGui_impl_Style_SetMouseCursorScale},
        {"getMouseCursorScale", ImGui_impl_Style_GetMouseCursorScale},
        {"setCurveTessellationTol", ImGui_impl_Style_SetCurveTessellationTol},
        {"getCurveTessellationTol", ImGui_impl_Style_GetCurveTessellationTol},
        {"setCircleSegmentMaxError", ImGui_impl_Style_SetCircleSegmentMaxError},
        {"getCircleSegmentMaxError", ImGui_impl_Style_GetCircleSegmentMaxError},
        {"setWindowPadding", ImGui_impl_Style_SetWindowPadding},
        {"getWindowPadding", ImGui_impl_Style_GetWindowPadding},
        {"setWindowMinSize", ImGui_impl_Style_SetWindowMinSize},
        {"getWindowMinSize", ImGui_impl_Style_GetWindowMinSize},
        {"setWindowTitleAlign", ImGui_impl_Style_SetWindowTitleAlign},
        {"getWindowTitleAlign", ImGui_impl_Style_GetWindowTitleAlign},
        {"setFramePadding", ImGui_impl_Style_SetFramePadding},
        {"getFramePadding", ImGui_impl_Style_GetFramePadding},
        {"setItemSpacing", ImGui_impl_Style_SetItemSpacing},
        {"getItemSpacing", ImGui_impl_Style_GetItemSpacing},
        {"setItemInnerSpacing", ImGui_impl_Style_SetItemInnerSpacing},
        {"getItemInnerSpacing", ImGui_impl_Style_GetItemInnerSpacing},
        {"setTouchExtraPadding", ImGui_impl_Style_SetTouchExtraPadding},
        {"getTouchExtraPadding", ImGui_impl_Style_GetTouchExtraPadding},
        {"setButtonTextAlign", ImGui_impl_Style_SetButtonTextAlign},
        {"getButtonTextAlign", ImGui_impl_Style_GetButtonTextAlign},
        {"setSelectableTextAlign", ImGui_impl_Style_SetSelectableTextAlign},
        {"getSelectableTextAlign", ImGui_impl_Style_GetSelectableTextAlign},
        {"setDisplayWindowPadding", ImGui_impl_Style_SetDisplayWindowPadding},
        {"getDisplayWindowPadding", ImGui_impl_Style_GetDisplayWindowPadding},
        {"setDisplaySafeAreaPadding", ImGui_impl_Style_SetDisplaySafeAreaPadding},
        {"getDisplaySafeAreaPadding", ImGui_impl_Style_GetDisplaySafeAreaPadding},
        {"setWindowMenuButtonPosition", ImGui_impl_Style_SetWindowMenuButtonPosition},
        {"getWindowMenuButtonPosition", ImGui_impl_Style_GetWindowMenuButtonPosition},
        {"setColorButtonPosition", ImGui_impl_Style_SetColorButtonPosition},
        {"getColorButtonPosition", ImGui_impl_Style_GetColorButtonPosition},
        {"setAntiAliasedLines", ImGui_impl_Style_SetAntiAliasedLines},
        {"getAntiAliasedLines", ImGui_impl_Style_GetAntiAliasedLines},
        {"setAntiAliasedLinesUseTex", ImGui_impl_Style_SetAntiAliasedLinesUseTex},
        {"getAntiAliasedLinesUseTex", ImGui_impl_Style_GetAntiAliasedLinesUseTex},
        {"setAntiAliasedFill", ImGui_impl_Style_SetAntiAliasedFill},
        {"getAntiAliasedFill", ImGui_impl_Style_GetAntiAliasedFill},

        {NULL, NULL},
    };
    binder.createClass(STYLES_CLASS_NAME, 0, NULL, NULL, imguiStylesFunctionList);

    const luaL_Reg imguiDrawListFunctionList[] =
    {
        {"pushClipRect", ImGui_impl_DrawList_PushClipRect},
        {"pushClipRectFullScreen", ImGui_impl_DrawList_PushClipRectFullScreen},
        {"popClipRect", ImGui_impl_DrawList_PopClipRect},
        {"pushTextureID", ImGui_impl_DrawList_PushTextureID},
        {"popTextureID", ImGui_impl_DrawList_PopTextureID},
        {"getClipRectMin", ImGui_impl_DrawList_GetClipRectMin},
        {"getClipRectMax", ImGui_impl_DrawList_GetClipRectMax},
        {"addLine", ImGui_impl_DrawList_AddLine},
        {"addRect", ImGui_impl_DrawList_AddRect},
        {"addRectFilled", ImGui_impl_DrawList_AddRectFilled},
        {"addRectFilledMultiColor", ImGui_impl_DrawList_AddRectFilledMultiColor},
        {"addQuad", ImGui_impl_DrawList_AddQuad},
        {"addQuadFilled", ImGui_impl_DrawList_AddQuadFilled},
        {"addTriangle", ImGui_impl_DrawList_AddTriangle},
        {"addTriangleFilled", ImGui_impl_DrawList_AddTriangleFilled},
        {"addCircle", ImGui_impl_DrawList_AddCircle},
        {"addCircleFilled", ImGui_impl_DrawList_AddCircleFilled},
        {"addNgon", ImGui_impl_DrawList_AddNgon},
        {"addNgonFilled", ImGui_impl_DrawList_AddNgonFilled},
        {"addText", ImGui_impl_DrawList_AddText},
        {"addFontText", ImGui_impl_DrawList_AddFontText},
        {"addPolyline", ImGui_impl_DrawList_AddPolyline},
        {"addConvexPolyFilled", ImGui_impl_DrawList_AddConvexPolyFilled},
        {"addBezierCurve", ImGui_impl_DrawList_AddBezierCurve},

        {"addImage", ImGui_impl_DrawList_AddImage},
        {"addImageQuad", ImGui_impl_DrawList_AddImageQuad},
        {"addImageRounded", ImGui_impl_DrawList_AddImageRounded},
        {"pathClear", ImGui_impl_DrawList_PathClear},
        {"pathLineTo", ImGui_impl_DrawList_PathLineTo},
        {"pathLineToMergeDuplicate", ImGui_impl_DrawList_PathLineToMergeDuplicate},
        {"pathFillConvex", ImGui_impl_DrawList_PathFillConvex},
        {"pathStroke", ImGui_impl_DrawList_PathStroke},
        {"pathArcTo", ImGui_impl_DrawList_PathArcTo},
        {"pathArcToFast", ImGui_impl_DrawList_PathArcToFast},
        {"pathBezierCurveTo", ImGui_impl_DrawList_PathBezierCurveTo},
        {"pathRect", ImGui_impl_DrawList_PathRect},
        {NULL, NULL}
    };
    binder.createClass(DRAW_LIST_CLASS_NAME, 0, NULL, NULL, imguiDrawListFunctionList);

    const luaL_Reg imguiIoFunctionList[] =
    {
        {"setFontDefault", ImGui_impl_IO_SetFontDefault},
        {"getFonts", ImGui_impl_IO_GetFonts},
        {"getDeltaTime", ImGui_impl_IO_GetDeltaTime},
        {"isMouseDown", ImGui_impl_IO_isMouseDown},
        {"getMouseWheel", ImGui_impl_IO_GetMouseWheel},
        {"getMouseWheelH", ImGui_impl_IO_GetMouseWheelH},
        {"isKeyCtrl", ImGui_impl_IO_isKeyCtrl},
        {"isKeyShift", ImGui_impl_IO_isKeyShift},
        {"isKeyAlt", ImGui_impl_IO_isKeyAlt},
        {"isKeySuper", ImGui_impl_IO_isKeySuper},
        {"getKeysDown", ImGui_impl_IO_GetKeysDown},
        {"wantCaptureMouse", ImGui_impl_IO_WantCaptureMouse},
        {"wantCaptureKeyboard", ImGui_impl_IO_WantCaptureKeyboard},
        {"wantTextInput", ImGui_impl_IO_WantTextInput},
        {"wantSetMousePos", ImGui_impl_IO_WantSetMousePos},
        {"wantSaveIniSettings", ImGui_impl_IO_WantSaveIniSettings},
        {"isNavActive", ImGui_impl_IO_IsNavActive},
        {"isNavVisible", ImGui_impl_IO_IsNavVisible},
        {"getFramerate", ImGui_impl_IO_GetFramerate},
        {"getMetricsRenderVertices", ImGui_impl_IO_GetMetricsRenderVertices},
        {"getMetricsRenderIndices", ImGui_impl_IO_GetMetricsRenderIndices},
        {"getMetricsRenderWindows", ImGui_impl_IO_GetMetricsRenderWindows},
        {"getMetricsActiveWindows", ImGui_impl_IO_GetMetricsActiveWindows},
        {"getMetricsActiveAllocations", ImGui_impl_IO_GetMetricsActiveAllocations},
        {"getMouseDelta", ImGui_impl_IO_GetMouseDelta},
        {"getMouseDownSec", ImGui_impl_IO_GetMouseDownSec},
        {"setDisplaySize", ImGui_impl_IO_SetDisplaySize},
        {"getDisplaySize", ImGui_impl_IO_GetDisplaySize},

#ifdef IMGUI_HAS_DOCK
        {"setConfigDockingNoSplit", ImGui_impl_IO_GetConfigDockingNoSplit},
        {"setConfigDockingNoSplit", ImGui_impl_IO_SetConfigDockingNoSplit},
        {"setConfigDockingWithShift", ImGui_impl_IO_GetConfigDockingWithShift},
        {"setConfigDockingWithShift", ImGui_impl_IO_SetConfigDockingWithShift},
        {"setConfigDockingAlwaysTabBar", ImGui_impl_IO_GetConfigDockingAlwaysTabBar},
        {"setConfigDockingAlwaysTabBar", ImGui_impl_IO_SetConfigDockingAlwaysTabBar},
        {"setConfigDockingTransparentPayload", ImGui_impl_IO_GetConfigDockingTransparentPayload},
        {"setConfigDockingTransparentPayload", ImGui_impl_IO_SetConfigDockingTransparentPayload},
#endif
        {"getConfigFlags", ImGui_impl_IO_GetConfigFlags},
        {"setConfigFlags", ImGui_impl_IO_SetConfigFlags},
        {"addConfigFlags", ImGui_impl_IO_AddConfigFlags},
        {"getBackendFlags", ImGui_impl_IO_GetBackendFlags},
        {"setBackendFlags", ImGui_impl_IO_SetBackendFlags},
        {"getIniSavingRate", ImGui_impl_IO_GetIniSavingRate},
        {"setIniSavingRate", ImGui_impl_IO_SetIniSavingRate},
        {"getIniFilename", ImGui_impl_IO_GetIniFilename},
        {"setIniFilename", ImGui_impl_IO_SetIniFilename},
        {"getLogFilename", ImGui_impl_IO_GetLogFilename},
        {"setLogFilename", ImGui_impl_IO_SetLogFilename},
        {"getMouseDoubleClickTime", ImGui_impl_IO_GetMouseDoubleClickTime},
        {"setMouseDoubleClickTime", ImGui_impl_IO_SetMouseDoubleClickTime},
        {"getMouseDragThreshold", ImGui_impl_IO_GetMouseDragThreshold},
        {"setMouseDragThreshold", ImGui_impl_IO_SetMouseDragThreshold},
        {"getMouseDrawCursor", ImGui_impl_IO_GetMouseDrawCursor},
        {"setMouseDrawCursor", ImGui_impl_IO_SetMouseDrawCursor},
        {"getMouseDoubleClickMaxDist", ImGui_impl_IO_GetMouseDoubleClickMaxDist},
        {"setMouseDoubleClickMaxDist", ImGui_impl_IO_SetMouseDoubleClickMaxDist},
        {"getKeyMapValue", ImGui_impl_IO_GetKeyMapValue},
        {"setKeyMapValue", ImGui_impl_IO_SetKeyMapValue},
        {"getKeyRepeatDelay", ImGui_impl_IO_GetKeyRepeatDelay},
        {"setKeyRepeatDelay", ImGui_impl_IO_SetKeyRepeatDelay},
        {"getKeyRepeatRate", ImGui_impl_IO_GetKeyRepeatRate},
        {"setKeyRepeatRate", ImGui_impl_IO_SetKeyRepeatRate},
        {"getFontGlobalScale", ImGui_impl_IO_GetFontGlobalScale},
        {"setFontGlobalScale", ImGui_impl_IO_SetFontGlobalScale},
        {"getFontAllowUserScaling", ImGui_impl_IO_GetFontAllowUserScaling},
        {"setFontAllowUserScaling", ImGui_impl_IO_SetFontAllowUserScaling},
        {"getDisplayFramebufferScale", ImGui_impl_IO_GetDisplayFramebufferScale},
        {"setDisplayFramebufferScale", ImGui_impl_IO_SetDisplayFramebufferScale},
        {"getConfigMacOSXBehaviors", ImGui_impl_IO_GetConfigMacOSXBehaviors},
        {"setConfigMacOSXBehaviors", ImGui_impl_IO_SetConfigMacOSXBehaviors},
        {"getConfigInputTextCursorBlink", ImGui_impl_IO_GetConfigInputTextCursorBlink},
        {"setConfigInputTextCursorBlink", ImGui_impl_IO_SetConfigInputTextCursorBlink},
        {"getConfigWindowsResizeFromEdges", ImGui_impl_IO_GetConfigWindowsResizeFromEdges},
        {"setConfigWindowsResizeFromEdges", ImGui_impl_IO_SetConfigWindowsResizeFromEdges},
        {"getConfigWindowsMoveFromTitleBarOnly", ImGui_impl_IO_GetConfigWindowsMoveFromTitleBarOnly},
        {"setConfigWindowsMoveFromTitleBarOnly", ImGui_impl_IO_SetConfigWindowsMoveFromTitleBarOnly},
        {"getConfigWindowsMemoryCompactTimer", ImGui_impl_IO_GetConfigWindowsMemoryCompactTimer},
        {"setConfigWindowsMemoryCompactTimer", ImGui_impl_IO_SetConfigWindowsMemoryCompactTimer},

        {"getBackendPlatformName", ImGui_impl_IO_GetBackendPlatformName},
        {"getBackendRendererName", ImGui_impl_IO_GetBackendRendererName},

        {NULL, NULL}
    };
    binder.createClass(IO_CLASS_NAME, 0, NULL, NULL, imguiIoFunctionList);

    const luaL_Reg imguiFontAtlasFunctionList[] =
    {
        {"addFont", ImGui_impl_FontAtlas_AddFont},
        {"addFonts", ImGui_impl_FontAtlas_AddFonts},
        {"getFont", ImGui_impl_FontAtlas_GetFontByIndex},
        {"getCurrentFont", ImGui_impl_FontAtlas_GetCurrentFont},
        {"addDefaultFont", ImGui_impl_FontAtlas_AddDefaultFont},
        {"build", ImGui_impl_FontAtlas_BuildFont},
        {"bake", ImGui_impl_FontAtlas_Bake},
        {"clearInputData", ImGui_impl_FontAtlas_ClearInputData},
        {"clearTexData", ImGui_impl_FontAtlas_ClearTexData},
        {"clearFonts", ImGui_impl_FontAtlas_ClearFonts},
        {"clear", ImGui_impl_FontAtlas_Clear},
        {"isBuilt", ImGui_impl_FontAtlas_IsBuilt},
        {"addCustomRectRegular", ImGui_impl_FontAtlas_AddCustomRectRegular},
        {"addCustomRectFontGlyph", ImGui_impl_FontAtlas_AddCustomRectFontGlyph},
        {"getCustomRectByIndex", ImGui_impl_FontAtlas_GetCustomRectByIndex},
        {NULL, NULL}
    };
    binder.createClass(FONT_ATLAS_CLASS_NAME, 0, NULL, NULL, imguiFontAtlasFunctionList);

    const luaL_Reg imguiFontFunctionList[] = {
        {NULL, NULL}
    };
    binder.createClass(FONT_CLASS_NAME, 0, NULL, NULL, imguiFontFunctionList);

    const luaL_Reg imguiFunctionList[] =
    {
        // Fonts API
        {"pushFont", ImGui_impl_Fonts_PushFont},
        {"popFont", ImGui_impl_Fonts_PopFont},

        {"setStyleColor", ImGui_impl_Style_old_SetColor}, // Backward capability
        // Draw list
        {"getStyle", ImGui_impl_GetStyle},
        {"getWindowDrawList", ImGui_impl_GetWindowDrawList},
        {"getBackgroundDrawList", ImGui_impl_GetBackgroundDrawList},
        {"getForegroundDrawList", ImGui_impl_GetForegroundDrawList},
        {"getIO", ImGui_impl_GetIO},

        /////////////////////////////////////////////////////////////////////////////// Inputs +
        /// Mouse
        {"onMouseHover", ImGui_impl_MouseHover},
        {"onMouseMove", ImGui_impl_MouseMove},
        {"onMouseDown", ImGui_impl_MouseDown},
        {"onMouseUp", ImGui_impl_MouseUp},
        {"onMouseWheel", ImGui_impl_MouseWheel},

        /// Touch TODO

        /// Keyboard
        {"onKeyUp", ImGui_impl_KeyUp},
        {"onKeyDown", ImGui_impl_KeyDown},
        {"onKeyChar", ImGui_impl_KeyChar},

        /////////////////////////////////////////////////////////////////////////////// Inputs -

        // Colors TODO
        {"colorConvertHEXtoRGB", ImGui_impl_ColorConvertHEXtoRGB},
        {"colorConvertRGBtoHEX", ImGui_impl_ColorConvertRGBtoHEX},
        {"colorConvertRGBtoHSV", ImGui_impl_ColorConvertRGBtoHSV},
        {"colorConvertHSVtoRGB", ImGui_impl_ColorConvertHSVtoRGB},

        // Style themes
        {"setDarkStyle", ImGui_impl_StyleDark},
        {"setLightStyle", ImGui_impl_StyleLight},
        {"setClassicStyle", ImGui_impl_StyleClassic},

        // Childs
        {"beginChild", ImGui_impl_BeginChild},
        {"endChild", ImGui_impl_EndChild},

        {"isWindowAppearing", ImGui_impl_IsWindowAppearing},
        {"isWindowCollapsed", ImGui_impl_IsWindowCollapsed},
        {"isWindowFocused", ImGui_impl_IsWindowFocused},
        {"isWindowHovered", ImGui_impl_IsWindowHovered},
        {"getWindowPos", ImGui_impl_GetWindowPos},
        {"getWindowSize", ImGui_impl_GetWindowSize},
        {"getWindowWidth", ImGui_impl_GetWindowWidth},
        {"getWindowHeight", ImGui_impl_GetWindowHeight},

        {"setNextWindowPos", ImGui_impl_SetNextWindowPos},
        {"setNextWindowSize", ImGui_impl_SetNextWindowSize},
        {"setNextWindowSizeConstraints", ImGui_impl_SetNextWindowSizeConstraints},
        {"setNextWindowContentSize", ImGui_impl_SetNextWindowContentSize},
        {"setNextWindowCollapsed", ImGui_impl_SetNextWindowCollapsed},
        {"setNextWindowFocus", ImGui_impl_SetNextWindowFocus},
        {"setNextWindowBgAlpha", ImGui_impl_SetNextWindowBgAlpha},
        {"setWindowPos", ImGui_impl_SetWindowPos},
        {"setWindowSize", ImGui_impl_SetWindowSize},
        {"setWindowCollapsed", ImGui_impl_SetWindowCollapsed},
        {"setWindowFocus", ImGui_impl_SetWindowFocus},
        {"setWindowFontScale", ImGui_impl_SetWindowFontScale},

        {"getContentRegionMax", ImGui_impl_GetContentRegionMax},
        {"getContentRegionAvail", ImGui_impl_GetContentRegionAvail},
        {"getWindowContentRegionMin", ImGui_impl_GetWindowContentRegionMin},
        {"getWindowContentRegionMax", ImGui_impl_GetWindowContentRegionMax},
        {"getWindowContentRegionWidth", ImGui_impl_GetWindowContentRegionWidth},

        {"getScrollX", ImGui_impl_GetScrollX},
        {"getScrollY", ImGui_impl_GetScrollY},
        {"getScrollMaxX", ImGui_impl_GetScrollMaxX},
        {"getScrollMaxY", ImGui_impl_GetScrollMaxY},
        {"setScrollX", ImGui_impl_SetScrollX},
        {"setScrollY", ImGui_impl_SetScrollY},
        {"setScrollHereX", ImGui_impl_SetScrollHereX},
        {"setScrollHereY", ImGui_impl_SetScrollHereY},
        {"setScrollFromPosX", ImGui_impl_SetScrollFromPosX},
        {"setScrollFromPosY", ImGui_impl_SetScrollFromPosY},

        {"pushStyleColor", ImGui_impl_PushStyleColor},
        {"popStyleColor", ImGui_impl_PopStyleColor},
        {"pushStyleVar", ImGui_impl_PushStyleVar},
        {"popStyleVar", ImGui_impl_PopStyleVar},
        {"getFontSize", ImGui_impl_GetFontSize},

        {"pushItemWidth", ImGui_impl_PushItemWidth},
        {"popItemWidth", ImGui_impl_PopItemWidth},
        {"setNextItemWidth", ImGui_impl_SetNextItemWidth},
        {"calcItemWidth", ImGui_impl_CalcItemWidth},
        {"pushTextWrapPos", ImGui_impl_PushTextWrapPos},
        {"popTextWrapPos", ImGui_impl_PopTextWrapPos},
        {"pushAllowKeyboardFocus", ImGui_impl_PushAllowKeyboardFocus},
        {"popAllowKeyboardFocus", ImGui_impl_PopAllowKeyboardFocus},
        {"pushButtonRepeat", ImGui_impl_PushButtonRepeat},
        {"popButtonRepeat", ImGui_impl_PopButtonRepeat},

        {"separator", ImGui_impl_Separator},
        {"sameLine", ImGui_impl_SameLine},
        {"newLine", ImGui_impl_NewLine},
        {"spacing", ImGui_impl_Spacing},
        {"dummy", ImGui_impl_Dummy},
        {"indent", ImGui_impl_Indent},
        {"unindent", ImGui_impl_Unindent},
        {"beginGroup", ImGui_impl_BeginGroup},
        {"endGroup", ImGui_impl_EndGroup},

        {"getCursorPos", ImGui_impl_GetCursorPos},
        {"getCursorPosX", ImGui_impl_GetCursorPosX},
        {"getCursorPosY", ImGui_impl_GetCursorPosY},
        {"setCursorPos", ImGui_impl_SetCursorPos},
        {"setCursorPosX", ImGui_impl_SetCursorPosX},
        {"setCursorPosY", ImGui_impl_SetCursorPosY},
        {"getCursorStartPos", ImGui_impl_GetCursorStartPos},
        {"getCursorScreenPos", ImGui_impl_GetCursorScreenPos},
        {"alignTextToFramePadding", ImGui_impl_AlignTextToFramePadding},
        {"getTextLineHeight", ImGui_impl_GetTextLineHeight},
        {"getTextLineHeightWithSpacing", ImGui_impl_GetTextLineHeightWithSpacing},
        {"getFrameHeight", ImGui_impl_GetFrameHeight},
        {"getFrameHeightWithSpacing", ImGui_impl_GetFrameHeightWithSpacing},

        {"pushID", ImGui_impl_PushID},
        {"popID", ImGui_impl_PopID},
        {"getID", ImGui_impl_GetID},

        {"textUnformatted", ImGui_impl_TextUnformatted},
        {"text", ImGui_impl_Text},
        {"textColored", ImGui_impl_TextColored},
        {"textDisabled", ImGui_impl_TextDisabled},
        {"textWrapped", ImGui_impl_TextWrapped},
        {"labelText", ImGui_impl_LabelText},
        {"bulletText", ImGui_impl_BulletText},

        {"button", ImGui_impl_Button},
        {"smallButton", ImGui_impl_SmallButton},
        {"invisibleButton", ImGui_impl_InvisibleButton},
        {"arrowButton", ImGui_impl_ArrowButton},
        {"image", ImGui_impl_Image},
        {"imageFilled", ImGui_impl_ImageFilled},
        {"imageButton", ImGui_impl_ImageButton},
        {"imageButtonWithText", ImGui_impl_ImageButtonWithText},
        {"checkbox", ImGui_impl_Checkbox},
        {"checkboxFlags", ImGui_impl_CheckboxFlags},
        {"radioButton", ImGui_impl_RadioButton},
        {"progressBar", ImGui_impl_ProgressBar},
        {"bullet", ImGui_impl_Bullet},
        {"beginCombo", ImGui_impl_BeginCombo},
        {"endCombo", ImGui_impl_EndCombo},
        {"combo", ImGui_impl_Combo},

        {"dragFloat", ImGui_impl_DragFloat},
        {"dragFloat2", ImGui_impl_DragFloat2},
        {"dragFloat3", ImGui_impl_DragFloat3},
        {"dragFloat4", ImGui_impl_DragFloat4},
        {"dragFloatRange2", ImGui_impl_DragFloatRange2},

        {"dragInt", ImGui_impl_DragInt},
        {"dragInt2", ImGui_impl_DragInt2},
        {"dragInt3", ImGui_impl_DragInt3},
        {"dragInt4", ImGui_impl_DragInt4},
        {"dragIntRange2", ImGui_impl_DragIntRange2},
        {"dragScalar", ImGui_impl_DragScalar},

        {"sliderFloat", ImGui_impl_SliderFloat},
        {"sliderFloat2", ImGui_impl_SliderFloat2},
        {"sliderFloat3", ImGui_impl_SliderFloat3},
        {"sliderFloat4", ImGui_impl_SliderFloat4},
        {"sliderAngle", ImGui_impl_SliderAngle},
        {"sliderInt", ImGui_impl_SliderInt},
        {"sliderInt2", ImGui_impl_SliderInt2},
        {"sliderInt3", ImGui_impl_SliderInt3},
        {"sliderInt4", ImGui_impl_SliderInt4},
        {"sliderScalar", ImGui_impl_SliderScalar},
        {"vSliderFloat", ImGui_impl_VSliderFloat},
        {"vSliderInt", ImGui_impl_VSliderInt},
        {"vSliderScalar", ImGui_impl_VSliderScalar},

        {"filledSliderFloat", ImGui_impl_FilledSliderFloat},
        {"filledSliderFloat2", ImGui_impl_FilledSliderFloat2},
        {"filledSliderFloat3", ImGui_impl_FilledSliderFloat3},
        {"filledSliderFloat4", ImGui_impl_FilledSliderFloat4},
        {"filledSliderAngle", ImGui_impl_FilledSliderAngle},
        {"filledSliderInt", ImGui_impl_FilledSliderInt},
        {"filledSliderInt2", ImGui_impl_FilledSliderInt2},
        {"filledSliderInt3", ImGui_impl_FilledSliderInt3},
        {"filledSliderInt4", ImGui_impl_FilledSliderInt4},
        {"filledSliderScalar", ImGui_impl_FilledSliderScalar},
        {"vFilledSliderFloat", ImGui_impl_VFilledSliderFloat},
        {"vFilledSliderInt", ImGui_impl_VFilledSliderInt},
        {"vFilledSliderScalar", ImGui_impl_VFilledSliderScalar},

        {"inputText", ImGui_impl_InputText},
        {"inputTextMultiline", ImGui_impl_InputTextMultiline},
        {"inputTextWithHint", ImGui_impl_InputTextWithHint},
        {"inputFloat", ImGui_impl_InputFloat},
        {"inputFloat2", ImGui_impl_InputFloat2},
        {"inputFloat3", ImGui_impl_InputFloat3},
        {"inputFloat4", ImGui_impl_InputFloat4},
        {"inputInt", ImGui_impl_InputInt},
        {"inputInt2", ImGui_impl_InputInt2},
        {"inputInt3", ImGui_impl_InputInt3},
        {"inputInt4", ImGui_impl_InputInt4},
        {"inputDouble", ImGui_impl_InputDouble},
        {"inputScalar", ImGui_impl_InputScalar},

        {"colorEdit3", ImGui_impl_ColorEdit3},
        {"colorEdit4", ImGui_impl_ColorEdit4},
        {"colorPicker3", ImGui_impl_ColorPicker3},
        {"colorPicker4", ImGui_impl_ColorPicker4},
        {"colorButton", ImGui_impl_ColorButton},
        {"setColorEditOptions", ImGui_impl_SetColorEditOptions},

        {"treeNode", ImGui_impl_TreeNode},
        {"treeNodeEx", ImGui_impl_TreeNodeEx},
        {"treePush", ImGui_impl_TreePush},
        {"treePop", ImGui_impl_TreePop},
        {"getTreeNodeToLabelSpacing", ImGui_impl_GetTreeNodeToLabelSpacing},
        {"collapsingHeader", ImGui_impl_CollapsingHeader},
        {"setNextItemOpen", ImGui_impl_SetNextItemOpen},
        {"selectable", ImGui_impl_Selectable},

        {"listBox", ImGui_impl_ListBox},
        {"listBoxHeader", ImGui_impl_ListBoxHeader},
        {"listBoxHeader2", ImGui_impl_ListBoxHeader2},
        {"listBoxFooter", ImGui_impl_ListBoxFooter},
        {"plotLines", ImGui_impl_PlotLines},
        {"plotHistogram", ImGui_impl_PlotHistogram},
        {"value", ImGui_impl_Value},

        {"beginMenuBar", ImGui_impl_BeginMenuBar },
        {"endMenuBar", ImGui_impl_EndMenuBar },
        {"beginMainMenuBar", ImGui_impl_BeginMainMenuBar },
        {"endMainMenuBar", ImGui_impl_EndMainMenuBar },
        {"beginMenu", ImGui_impl_BeginMenu },
        {"endMenu", ImGui_impl_EndMenu },
        {"menuItem", ImGui_impl_MenuItem },
        {"menuItemWithShortcut", ImGui_impl_MenuItemWithShortcut },
        {"beginTooltip", ImGui_impl_BeginTooltip },
        {"endTooltip", ImGui_impl_EndTooltip },
        {"setTooltip", ImGui_impl_SetTooltip },
        {"beginPopup", ImGui_impl_BeginPopup},
        {"beginPopupModal", ImGui_impl_BeginPopupModal },
        {"endPopup", ImGui_impl_EndPopup },
        {"openPopup", ImGui_impl_OpenPopup },
        {"openPopupContextItem", ImGui_impl_OpenPopupContextItem },
        {"closeCurrentPopup", ImGui_impl_CloseCurrentPopup },
        {"beginPopupContextItem", ImGui_impl_BeginPopupContextItem },
        {"beginPopupContextWindow", ImGui_impl_BeginPopupContextWindow },
        {"beginPopupContextVoid", ImGui_impl_BeginPopupContextVoid },
        {"isPopupOpen", ImGui_impl_IsPopupOpen },

        {"columns", ImGui_impl_Columns},
        {"nextColumn", ImGui_impl_NextColumn},
        {"getColumnIndex", ImGui_impl_GetColumnIndex},
        {"getColumnWidth", ImGui_impl_GetColumnWidth},
        {"setColumnWidth", ImGui_impl_SetColumnWidth},
        {"getColumnOffset", ImGui_impl_GetColumnOffset},
        {"setColumnOffset", ImGui_impl_SetColumnOffset},
        {"getColumnsCount", ImGui_impl_GetColumnsCount},

        {"beginTabBar", ImGui_impl_BeginTabBar},
        {"endTabBar", ImGui_impl_EndTabBar},
        {"beginTabItem", ImGui_impl_BeginTabItem},
        {"endTabItem", ImGui_impl_EndTabItem},
        {"setTabItemClosed", ImGui_impl_SetTabItemClosed},

        {"logToTTY", ImGui_impl_LogToTTY},
        {"logToFile", ImGui_impl_LogToFile},
        {"logToClipboard", ImGui_impl_LogToClipboard},
        {"logFinish", ImGui_impl_LogFinish},
        {"logButtons", ImGui_impl_LogButtons},
        {"logText", ImGui_impl_LogText},

        {"pushClipRect", ImGui_impl_PushClipRect},
        {"popClipRect", ImGui_impl_PopClipRect},

        {"setItemDefaultFocus", ImGui_impl_SetItemDefaultFocus},
        {"setKeyboardFocusHere", ImGui_impl_SetKeyboardFocusHere},

        {"isItemHovered", ImGui_impl_IsItemHovered},
        {"isItemActive", ImGui_impl_IsItemActive},
        {"isItemFocused", ImGui_impl_IsItemFocused},
        {"isItemClicked", ImGui_impl_IsItemClicked},
        {"isItemVisible", ImGui_impl_IsItemVisible},
        {"isItemEdited", ImGui_impl_IsItemEdited},
        {"isItemActivated", ImGui_impl_IsItemActivated},
        {"isItemDeactivated", ImGui_impl_IsItemDeactivated},
        {"isItemDeactivatedAfterEdit", ImGui_impl_IsItemDeactivatedAfterEdit},
        {"isItemToggledOpen", ImGui_impl_IsItemToggledOpen},
        {"isAnyItemHovered", ImGui_impl_IsAnyItemHovered},
        {"isAnyItemActive", ImGui_impl_IsAnyItemActive},
        {"isAnyItemFocused", ImGui_impl_IsAnyItemFocused},
        {"getItemRectMin", ImGui_impl_GetItemRectMin},
        {"getItemRectMax", ImGui_impl_GetItemRectMax},
        {"getItemRectSize", ImGui_impl_GetItemRectSize},
        {"setItemAllowOverlap", ImGui_impl_SetItemAllowOverlap},

        // Miscellaneous Utilities
        {"isRectVisible", ImGui_impl_IsRectVisible},
        {"getTime", ImGui_impl_GetTime},
        {"getFrameCount", ImGui_impl_GetFrameCount},
        {"getStyleColorName", ImGui_impl_GetStyleColorName},
        {"calcListClipping", ImGui_impl_CalcListClipping},
        {"beginChildFrame", ImGui_impl_BeginChildFrame},
        {"endChildFrame", ImGui_impl_EndChildFrame},

        // Text Utilities
        {"calcTextSize", ImGui_impl_CalcTextSize},

        // Inputs Utilities: Keyboard
        {"getKeyIndex", ImGui_impl_GetKeyIndex},
        {"isKeyDown", ImGui_impl_IsKeyDown},
        {"isKeyPressed", ImGui_impl_IsKeyPressed},
        {"isKeyReleased", ImGui_impl_IsKeyReleased},
        {"getKeyPressedAmount", ImGui_impl_GetKeyPressedAmount},
        {"captureKeyboardFromApp", ImGui_impl_CaptureKeyboardFromApp},

        // Inputs Utilities: Mouse
        {"isMouseDown", ImGui_impl_IsMouseDown},
        {"isMouseClicked", ImGui_impl_IsMouseClicked},
        {"isMouseReleased", ImGui_impl_IsMouseReleased},
        {"isMouseDoubleClicked", ImGui_impl_IsMouseDoubleClicked},
        {"isMouseHoveringRect", ImGui_impl_IsMouseHoveringRect},
        {"isMousePosValid", ImGui_impl_IsMousePosValid},
        {"isAnyMouseDown", ImGui_impl_IsAnyMouseDown},
        {"getMousePos", ImGui_impl_GetMousePos},
        {"getMousePosOnOpeningCurrentPopup", ImGui_impl_GetMousePosOnOpeningCurrentPopup},
        {"isMouseDragging", ImGui_impl_IsMouseDragging},
        {"getMouseDragDelta", ImGui_impl_GetMouseDragDelta},
        {"resetMouseDragDelta", ImGui_impl_ResetMouseDragDelta},
        {"getMouseCursor", ImGui_impl_GetMouseCursor},
        {"setMouseCursor", ImGui_impl_SetMouseCursor},
        {"captureMouseFromApp", ImGui_impl_CaptureMouseFromApp},

        // Windows
        {"beginWindow", ImGui_impl_Begin},
        {"endWindow", ImGui_impl_End},

        // Render
        {"newFrame", ImGui_impl_NewFrame},
        {"render", ImGui_impl_Render},
        {"endFrame", ImGui_impl_EndFrame},

        // Demos
        {"showUserGuide", ImGui_impl_ShowUserGuide},
        {"showDemoWindow", ImGui_impl_ShowDemoWindow},
        {"showAboutWindow", ImGui_impl_ShowAboutWindow},
        {"showStyleEditor", ImGui_impl_ShowStyleEditor},
        {"showFontSelector", ImGui_impl_ShowFontSelector},
        {"showMetricsWindow", ImGui_impl_ShowMetricsWindow},
        {"showStyleSelector", ImGui_impl_ShowStyleSelector},
        {"showLuaStyleEditor", ImGui_my_ShowLuaStyleEditor},

        // Drag & Drop
        {"beginDragDropSource", ImGui_impl_BeginDragDropSource},
        {"setDragDropPayload", ImGui_impl_SetDragDropPayload},
        {"endDragDropSource", ImGui_impl_EndDragDropSource},
        {"beginDragDropTarget", ImGui_impl_BeginDragDropTarget},
        {"acceptDragDropPayload", ImGui_impl_AcceptDragDropPayload},
        {"endDragDropTarget", ImGui_impl_EndDragDropTarget},
        {"getDragDropPayload", ImGui_impl_GetDragDropPayload},

#ifdef IMGUI_HAS_DOCK
        {"dockSpace", ImGui_impl_DockSpace},
        {"dockSpaceOverViewport", ImGui_impl_DockSpaceOverViewport},
        {"setNextWindowDockID", ImGui_impl_SetNextWindowDockID},
        {"getWindowDockID", ImGui_impl_GetWindowDockID},
        {"isWindowDocked", ImGui_impl_IsWindowDocked},

        {"dockBuilderDockWindow", ImGui_impl_DockBuilderDockWindow},
        //{"dockBuilderGetNode", ImGui_impl_DockBuilderGetNode},
        {"dockBuilderSetNodePos", ImGui_impl_DockBuilderSetNodePos},
        {"dockBuilderSetNodeSize", ImGui_impl_DockBuilderSetNodeSize},
        {"dockBuilderAddNode", ImGui_impl_DockBuilderAddNode},
        {"dockBuilderRemoveNode", ImGui_impl_DockBuilderRemoveNode},
        {"dockBuilderRemoveNodeChildNodes", ImGui_impl_DockBuilderRemoveNodeChildNodes},
        {"dockBuilderRemoveNodeDockedWindows", ImGui_impl_DockBuilderRemoveNodeDockedWindows},
        {"dockBuilderSplitNode", ImGui_impl_DockBuilderSplitNode},
        {"dockBuilderCopyNode", ImGui_impl_DockBuilderCopyNode},
        {"dockBuilderCopyWindowSettings", ImGui_impl_DockBuilderCopyWindowSettings},
        {"dockBuilderCopyDockSpace", ImGui_impl_DockBuilderCopyDockSpace},
        {"dockBuilderFinish", ImGui_impl_DockBuilderFinish},
#endif

        {NULL, NULL}
    };
    binder.createClass(CLASS_NAME, "Sprite", initImGui, destroyImGui, imguiFunctionList);
    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    BindEnums(L);

    lua_getglobal(L, CLASS_NAME);
    lua_pushstring(L, ImGui::GetVersion());
    lua_setfield(L, -2, "_VERSION");
    lua_pop(L, 1);

    return 1;
}

static void g_initializePlugin(lua_State* L)
{
    ::L = L;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, PLUGIN_NAME);

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* _UNUSED(L)) {  }

REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", Imgui)
