#define _UNUSED(n)

//#include "glog.h"

//#define IM_ASSERT( exp ) \
//    ( (exp) ? (void)0 : glog_v("Failed assertion at %s:%d %s\n",  __FILE__,__LINE__, #exp))

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
#include "texturebase.h"
#include "bitmapdata.h"

#include "imgui_src/imgui.h"


////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
////////////////////////////////////////////////////////////////////////////////

struct MyTextureData
{
    void *texture;
    float width;
    float height;
};

struct VColor {
    uint8_t r,g,b,a;
};

struct GColor {
    int hex; // 0xffffff
    float alpha; // [0..1]

    GColor()
    {
        hex = 0;
        alpha = 0;
    }

    GColor(int _hex, float _alpha = 1.0f)
    {
        hex = _hex;
        alpha = _alpha;
    }

    static ImVec4 toVec4(int hex, float alpha = 1.0f)
    {
        float s = 1.0f / 255.0f;
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

    static GColor toHex(float _r, float _g, float _b, float _a = 1.0f)
    {
        int r = _r * 255;
        int g = _g * 255;
        int b = _b * 255;

        int hex = (r << IM_COL32_B_SHIFT) + (g << IM_COL32_G_SHIFT) + (b << IM_COL32_R_SHIFT);

        return GColor(hex, _a);
    }

    static GColor toHex(ImVec4 color)
    {
        return GColor::toHex(color.x, color.y, color.y, color.w);
    }

    static ImU32 toU32(float _r, float _g, float _b, float _a = 1.0f)
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

    static ImU32 toU32(int hex, float alpha = 1.0f)
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

double getfield(lua_State *L, const char *key)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);  // get table[key]
    double result = lua_tonumber(L, -1);
    lua_pop(L, 1);  // remove number
    return result;
}

int getKeyboardModifiers(lua_State *L)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, "getKeyboardModifiers");
    lua_getglobal(L, "application");
    lua_call(L,1,1);
    int mod = luaL_checkinteger(L, -1);
    lua_pop(L, 2);

    return mod;
}

float getApplicationProperty(lua_State *L, const char *name)
{
    lua_getglobal(L, "application");
    lua_getfield(L, -1, name);
    lua_getglobal(L, "application");
    lua_call(L,1,1);
    float value = luaL_checknumber(L, -1);
    lua_pop(L, 2);

    return value;
}

MyTextureData getTexture(lua_State *L, int idx = 1)
{
    MyTextureData data;
    Binder binder(L);

    if (binder.isInstanceOf("TextureBase", idx))
    {
        TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", idx));

        TextureData* gdata = textureBase->data;
        data.texture = (void*)gdata->gid;
        data.width = (float)gdata->width / (float)gdata->exwidth;
        data.height = (float)gdata->width / (float)gdata->exheight;
    }
    else if (binder.isInstanceOf("TextureRegion", idx))
    {
        BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", idx));

        TextureData* gdata = bitmapData->texture()->data;
        data.texture = (void*)gdata->gid;
        data.width = (float)gdata->width / (float)gdata->exwidth;
        data.height = (float)gdata->width / (float)gdata->exheight;
    }
    else
    {
        lua_pushstring(L, "Type mismatch. 'TextureBase' or 'TextureRegion' expected.");
        lua_error(L);
    }
    return data;
}

int convertGiderosMouseButton(lua_State *L, int button)
{
    if (button <= 0)
    {
        lua_pushstring(L, "Button index must be > 0");
        lua_error(L);
    }

    return log2(button);
}

void LUAError(lua_State *L, const char *message)
{
    lua_pushstring(L, message);
    lua_error(L);
}

int luaL_optboolean(lua_State *L, int narg, int def)
{
    return lua_isboolean(L, narg) ? lua_toboolean(L, narg) : def;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void BindEnums(lua_State *L)
{
    lua_getglobal(L, "ImGui");
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
//    lua_pushinteger(L, ImGuiMouseCursor_Hand);                          lua_setfield(L, -2, "MouseCursor_Hand");
//    lua_pushinteger(L, ImGuiMouseCursor_ResizeAll);                     lua_setfield(L, -2, "MouseCursor_ResizeAll");
//    lua_pushinteger(L, ImGuiMouseCursor_ResizeEW);                      lua_setfield(L, -2, "MouseCursor_ResizeEW");
//    lua_pushinteger(L, ImGuiMouseCursor_Arrow);                         lua_setfield(L, -2, "MouseCursor_Arrow");
//    lua_pushinteger(L, ImGuiMouseCursor_ResizeNS);                      lua_setfield(L, -2, "MouseCursor_ResizeNS");
//    lua_pushinteger(L, ImGuiMouseCursor_None);                          lua_setfield(L, -2, "MouseCursor_None");
//    lua_pushinteger(L, ImGuiMouseCursor_NotAllowed);                    lua_setfield(L, -2, "MouseCursor_NotAllowed");
//    lua_pushinteger(L, ImGuiMouseCursor_ResizeNWSE);                    lua_setfield(L, -2, "MouseCursor_ResizeNWSE");
//    lua_pushinteger(L, ImGuiMouseCursor_ResizeNESW);                    lua_setfield(L, -2, "MouseCursor_ResizeNESW");
//    lua_pushinteger(L, ImGuiMouseCursor_TextInput);                     lua_setfield(L, -2, "MouseCursor_TextInput");

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
    lua_pushinteger(L, ImGuiColorEditFlags__OptionsDefault);            lua_setfield(L, -2, "ColorEditFlags__OptionsDefault");
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

    lua_pop(L, 1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static lua_State *L;
static Application* application;
static char keyWeak = ' ';

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// GImGui
///
/////////////////////////////////////////////////////////////////////////////////////////////

class GImGui
{
public:
    GImGui(LuaApplication* application, lua_State *L);
    ~GImGui();

    SpriteProxy *proxy;

    void doDraw(const CurrentTransform&, float sx, float sy, float ex, float ey);
private:
    LuaApplication* application;
    VertexBuffer<Point2f> vertices;
    VertexBuffer<Point2f> texcoords;
    VertexBuffer<VColor> colors;
};

static void _Draw(void *c, const CurrentTransform&t, float sx, float sy,
        float ex, float ey) {
    ((GImGui *) c)->doDraw(t, sx, sy, ex, ey);
}

static void _Destroy(void *c) {
    delete ((GImGui *) c);
}

GImGui::GImGui(LuaApplication* application, lua_State *L)
{
    this->application = application;
    proxy = gtexture_get_spritefactory()->createProxy(application->getApplication(), this, _Draw, _Destroy);
}

GImGui::~GImGui()
{

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->TexID = 0;
    ImGui::DestroyContext();
}

void GImGui::doDraw(const CurrentTransform&, float _UNUSED(sx), float _UNUSED(sy), float _UNUSED(ex), float _UNUSED(ey))
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data) return;
    ShaderEngine *engine=gtexture_get_engine();
    ShaderProgram *shp=engine->getDefault(ShaderEngine::STDP_TEXTURECOLOR);
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
       for (size_t i=0;i<vtx_size;i++) {
           vertices[i].x=vtx_buffer[i].pos.x;
           vertices[i].y=vtx_buffer[i].pos.y;
           texcoords[i].x=vtx_buffer[i].uv.x;
           texcoords[i].y=vtx_buffer[i].uv.y;
           uint32_t c=vtx_buffer[i].col;

           uint32_t r=(c&0xFF) >> IM_COL32_R_SHIFT;
           uint32_t g=(c&0xFF00) >> IM_COL32_G_SHIFT;
           uint32_t b=(c&0xFF0000) >> IM_COL32_B_SHIFT;
           uint32_t a=(c&0xFF000000) >> IM_COL32_A_SHIFT;

           colors[i].r = (r * a) >> 8;
           colors[i].g = (g * a) >> 8;
           colors[i].b=(b*a)>>8;
           colors[i].a=a;
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
              shp->drawElements(ShaderProgram::Triangles, pcmd->ElemCount,ShaderProgram::DUSHORT, idx_buffer,true,NULL);
              engine->popClip();
         }
         idx_buffer += pcmd->ElemCount;
       }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

/// FONTS

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
        *out_file_size = 0;

    G_FILE *f;
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
        *out_file_size = file_size;

    return file_data;
}

void GAddFontFromFileTTF(lua_State *L, const char* filename, float size_pixels, int oversample_x = 3, int oversample_y = 1, float esx = 0.0f, float esy = 0.0f)
{   ImGuiIO& io = ImGui::GetIO();

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

    ImFontConfig config;
    config.OversampleH = oversample_x;
    config.OversampleV = oversample_y;
    config.GlyphExtraSpacing.x = esx;
    config.GlyphExtraSpacing.y = esy;
    sprintf(config.Name, "%s, %fpx", filename, size_pixels);

    io.Fonts->AddFontFromMemoryTTF(data, data_size, size_pixels, &config, NULL);
}

void _addFonts(lua_State *L, int idx)
{
    int len = luaL_getn(L, idx);
    lua_pushvalue(L, idx);
    for (int i = 1; i <= len; i++)
    {
        lua_rawgeti(L, 4, i); // { fontPath, fontSize, OversampleH, OversampleV, GlyphExtraSpacingX, GlyphExtraSpacingY }

        lua_rawgeti(L, -1, 1);
        const char *filename = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, -1, 2); // font size
        float size_pixel = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        int oversampleH = 1;
        lua_rawgeti(L, -1, 3); // optional OversampleH
        if (!lua_isnil(L, -1)) oversampleH = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        int oversampleV = 1;
        lua_rawgeti(L, -1, 4); // optional OversampleV
        if (!lua_isnil(L, -1)) oversampleV = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        float spacingX = 0.0f;
        lua_rawgeti(L, -1, 5); // optional GlyphExtraSpacingX
        if (!lua_isnil(L, -1)) spacingX = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        float spacingY = 0.0f;
        lua_rawgeti(L, -1, 6); // optional GlyphExtraSpacingY
        if (!lua_isnil(L, -1)) spacingY = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        GAddFontFromFileTTF(L, filename, size_pixel, oversampleH, oversampleV, spacingX, spacingY);

        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

int initImGui(lua_State *L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    ::application = application->getApplication();

    Binder binder(L);

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

    if (lua_type(L, 1) == LUA_TNUMBER && lua_type(L, 2) == LUA_TNUMBER)
    {
        swidth = lua_tointeger(L, 1);
        sheight = lua_tointeger(L, 2);
    }
    else
    {
        swidth = (int)getApplicationProperty(L, "getContentWidth");
        sheight = (int)getApplicationProperty(L, "getContentHeight");
    }

    io.DisplaySize.x = swidth;
    io.DisplaySize.y = sheight;

    // Load fonts
    io.Fonts->AddFontDefault();

    if (lua_type(L, 3) == LUA_TTABLE)
    {
        _addFonts(L, 3);
    }

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

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

    g_id texture = gtexture_create(width, height, GTEXTURE_RGBA, GTEXTURE_UNSIGNED_BYTE, GTEXTURE_CLAMP, GTEXTURE_NEAREST, pixels, NULL, 0);
    io.Fonts->TexID = (void *)texture;

    GImGui *imgui = new GImGui(application, L);
    binder.pushInstance("ImGui", imgui->proxy);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, imgui);
    lua_pop(L, 1);

    return 1;
}

int destroyImGui(lua_State *L)
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

/// IO
int ImGui_impl_IO_GetConfigFlags(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushinteger(L, io.ConfigFlags);
    return 1;
}

int ImGui_impl_IO_SetConfigFlags(lua_State *L)
{
    ImGuiConfigFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = flags;
    return 0;
}

int ImGui_impl_IO_GetBackendFlags(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushinteger(L, io.BackendFlags);
    return 1;
}

int ImGui_impl_IO_SetBackendFlags(lua_State *L)
{
    ImGuiBackendFlags flags = luaL_checkinteger(L, 2);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags = flags;
    return 0;
}

int ImGui_impl_IO_GetIniSavingRate(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.IniSavingRate);
    return 1;
}

int ImGui_impl_IO_SetIniSavingRate(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.IniSavingRate = luaL_optnumber(L, 2, 5.0f);
    return 1;
}

int ImGui_impl_IO_GetIniFilename(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushstring(L, io.IniFilename);
    return 1;
}

int ImGui_impl_IO_SetIniFilename(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = luaL_optstring(L, 2, "imgui.ini");
    return 0;
}

int ImGui_impl_IO_GetLogFilename(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushstring(L, io.LogFilename);
    return 1;
}

int ImGui_impl_IO_SetLogFilename(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.LogFilename = luaL_optstring(L, 2, "imgui_log.txt");
    return 0;
}

int ImGui_impl_IO_GetMouseDoubleClickTime(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseDoubleClickTime);
    return 1;
}

int ImGui_impl_IO_SetMouseDoubleClickTime(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDoubleClickTime = luaL_optnumber(L, 2, 0.30f);
    return 0;
}

int ImGui_impl_IO_GetMouseDragThreshold(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseDragThreshold);
    return 1;
}

int ImGui_impl_IO_SetMouseDragThreshold(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDragThreshold = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int ImGui_impl_IO_GetMouseDoubleClickMaxDist(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseDoubleClickMaxDist);
    return 1;
}

int ImGui_impl_IO_SetMouseDoubleClickMaxDist(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDoubleClickMaxDist = luaL_optnumber(L, 2, 6.0f);
    return 0;
}

int ImGui_impl_IO_GetKeyMapValue(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > ImGuiKey_COUNT)
    {
        lua_pushstring(L, "KeyMap index is out of bounds!");
        lua_error(L);
    }
    lua_pushinteger(L, io.KeyMap[index]);
    return 1;
}

int ImGui_impl_IO_SetKeyMapValue(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > ImGuiKey_COUNT)
    {
        lua_pushstring(L, "KeyMap index is out of bounds!");
        lua_error(L);
    }

    io.KeyMap[index] = luaL_checkinteger(L, 3);
    return 0;
}

int ImGui_impl_IO_GetKeyRepeatDelay(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.KeyRepeatDelay);
    return 1;
}

int ImGui_impl_IO_SetKeyRepeatDelay(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyRepeatDelay = luaL_optnumber(L, 2, 0.25f);
    return 0;
}

int ImGui_impl_IO_GetKeyRepeatRate(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.KeyRepeatRate);
    return 1;
}

int ImGui_impl_IO_SetKeyRepeatRate(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyRepeatRate = luaL_optnumber(L, 2, 0.05f);
    return 0;
}

int ImGui_impl_IO_GetFontGlobalScale(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.FontGlobalScale);
    return 1;
}

int ImGui_impl_IO_SetFontGlobalScale(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = luaL_optnumber(L, 2, 1.0f);
    return 0;
}

int ImGui_impl_IO_GetFontAllowUserScaling(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.FontAllowUserScaling);
    return 1;
}

int ImGui_impl_IO_SetFontAllowUserScaling(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.FontAllowUserScaling = lua_toboolean(L, 2) > 0;
    return 0;
}

int ImGui_impl_IO_GetDisplayFramebufferScale(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.DisplayFramebufferScale.x);
    lua_pushnumber(L, io.DisplayFramebufferScale.y);
    return 2;
}

int ImGui_impl_IO_SetDisplayFramebufferScale(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 scale = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    io.DisplayFramebufferScale = scale;
    return 0;
}

int ImGui_impl_IO_GetConfigMacOSXBehaviors(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.ConfigMacOSXBehaviors);
    return 1;
}

int ImGui_impl_IO_SetConfigMacOSXBehaviors(lua_State *L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigMacOSXBehaviors = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigInputTextCursorBlink(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int ImGui_impl_IO_SetConfigInputTextCursorBlink(lua_State *L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigInputTextCursorBlink = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsResizeFromEdges(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.ConfigInputTextCursorBlink);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsResizeFromEdges(lua_State *L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsResizeFromEdges = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsMoveFromTitleBarOnly(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.ConfigWindowsMoveFromTitleBarOnly);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsMoveFromTitleBarOnly(lua_State *L)
{
    bool flag = lua_toboolean(L, 2) > 0;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = flag;
    return 0;
}

int ImGui_impl_IO_GetConfigWindowsMemoryCompactTimer(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.ConfigWindowsMemoryCompactTimer);
    return 1;
}

int ImGui_impl_IO_SetConfigWindowsMemoryCompactTimer(lua_State *L)
{
    float t = luaL_optnumber(L, 2, -1.0f);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMemoryCompactTimer = t;
    return 0;
}

int ImGui_impl_IO_GetBackendPlatformName(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushstring(L, io.BackendPlatformName);
    return 1;
}

int ImGui_impl_IO_GetBackendRendererName(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushstring(L, io.BackendRendererName);
    return 1;
}

int ImGui_impl_IO_isMouseDown(lua_State *L)
{
    int button = convertGiderosMouseButton(L, luaL_checkinteger(L, 2));
    if (button < 0 || button > 5)
    {
        lua_pushstring(L, "Button index is out of bounds!");
        lua_error(L);
    }
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.MouseDown[button]);
    return  1;
}

int ImGui_impl_IO_GetMouseWheel(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseWheel);
    return 1;
}

int ImGui_impl_IO_GetMouseWheelH(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseWheelH);
    return 1;
}

int ImGui_impl_IO_isKeyCtrl(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.KeyCtrl);
    return 1;
}

int ImGui_impl_IO_isKeyShift(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.KeyShift);
    return 1;
}

int ImGui_impl_IO_isKeyAlt(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.KeyAlt);
    return 1;
}

int ImGui_impl_IO_isKeySuper(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.KeySuper);
    return 1;
}

int ImGui_impl_IO_GetKeysDown(lua_State *L)
{
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > 512)
    {
        lua_pushstring(L, "KeyDown index is out of bounds!");
        lua_error(L);
    }
    ImGuiIO& io = ImGui::GetIO();
    lua_pushboolean(L, io.KeysDown[index]);
    return 1;
}

/// SOME IO FLAGS / VALUES

int ImGui_impl_WantCaptureMouse(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.WantCaptureMouse);
    return 1;
}

int ImGui_impl_WantCaptureKeyboard(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.WantCaptureKeyboard);
    return 1;
}

int ImGui_impl_WantTextInput(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.WantTextInput);
    return 1;
}

int ImGui_impl_WantSetMousePos(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.WantSetMousePos);
    return 1;
}

int ImGui_impl_WantSaveIniSettings(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.WantSaveIniSettings);
    return 1;
}

int ImGui_impl_NavActive(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.NavActive);
    return 1;
}

int ImGui_impl_NavVisible(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushboolean(L, io.NavVisible);
    return 1;
}

int ImGui_impl_Framerate(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushnumber(L, io.Framerate);
    return 1;
}

int ImGui_impl_MetricsRenderVertices(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushinteger(L, io.MetricsRenderVertices);
    return 1;
}

int ImGui_impl_MetricsRenderIndices(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushinteger(L, io.MetricsRenderIndices);
    return 1;
}

int ImGui_impl_MetricsRenderWindows(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushinteger(L, io.MetricsRenderWindows);
    return 1;
}

int ImGui_impl_MetricsActiveWindows(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();

    lua_pushinteger(L, io.MetricsActiveWindows);
    return 1;
}

int ImGui_impl_MetricsActiveAllocations(lua_State *L)
    {
    ImGuiIO& io = ImGui::GetIO();

    lua_pushinteger(L, io.MetricsActiveAllocations);
    return 1;
     }

int ImGui_impl_MouseDelta(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    lua_pushnumber(L, io.MouseDelta.x);
    lua_pushnumber(L, io.MouseDelta.y);
    return 2;
}

int ImGui_impl_MouseDownSec(lua_State *L)
    {
    ImGuiIO& io = ImGui::GetIO();
    int button = convertGiderosMouseButton(L, lua_tointeger(L, 2));

    lua_pushnumber(L, io.MouseDownDuration[button]);
    return 1;
    }


/// FONTS?

int ImGui_addFonts(lua_State *L)
{
    _addFonts(L, 2);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Build();

    return 0;
}


/// MOUSE INPUTS

int ImGui_impl_MouseHover(lua_State *L)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = getfield(L, "x");
    io.MousePos.y = getfield(L, "y");

    return 0;
}

int ImGui_impl_MouseMove(lua_State *L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = getfield(L, "x");
    io.MousePos.y = getfield(L, "y");
    io.MouseDown[button] = true;

    return 0;
}

int ImGui_impl_MouseDown(lua_State *L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = getfield(L, "x");
    io.MousePos.y = getfield(L, "y");
    io.MouseDown[button] = true;

    return 0;
}

int ImGui_impl_MouseUp(lua_State *L)
{
    int button = convertGiderosMouseButton(L, (int)getfield(L, "button"));

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = getfield(L, "x");
    io.MousePos.y = getfield(L, "y");
    io.MouseDown[button] = false;

    return 0;
}

int ImGui_impl_MouseWheel(lua_State *L)
{
    float wheel = getfield(L, "wheel");

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += wheel < 0 ? -1.0f : 1.0f;
    io.MousePos.x = getfield(L, "x");
    io.MousePos.y = getfield(L, "y");

    return 0;
}

/// KEYBOARD INPUTS

int ImGui_impl_KeyUp(lua_State *L)
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

int ImGui_impl_KeyDown(lua_State *L)
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

int ImGui_impl_KeyChar(lua_State *L)
{
    lua_pushstring(L, "text");
    lua_gettable(L, -2);
    const char *text = lua_tostring(L, -1);
    lua_pop(L, 1);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharactersUTF8(text);

    return 0;
}

/// DRAWING STUFF

int ImGui_impl_NewFrame(lua_State *L)
{
    double deltaTime = getfield(L, "deltaTime");

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;

    ImGui::NewFrame();

    return 0;
}

int ImGui_impl_Render(lua_State *L)
{
    ImGui::Render();
    return 0;
}

int ImGui_impl_EndFrame(lua_State *L)
{
    ImGui::EndFrame();
    return 0;
}

// Windows
int ImGui_impl_Begin(lua_State *L)
{
    const char *name = luaL_checkstring(L, 2);
    bool p_open = lua_toboolean(L, 3) > 0;
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::Begin(name, &p_open, flags);

    lua_pushboolean(L, p_open);
    lua_pushboolean(L, result);

    return 2;
}

int ImGui_impl_End(lua_State *L)
{
    ImGui::End();
    return 0;
}

// Child Windows
int ImGui_impl_BeginChild(lua_State *L)
{
    float w = luaL_optnumber(L, 3, 0);
    float h = luaL_optnumber(L, 4, 0);
    bool border = luaL_optboolean(L, 5, 0);
    ImGuiWindowFlags flags = luaL_optinteger(L, 6, 0);
    bool result;

    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char *str_id = luaL_checkstring(L, 2);
        result = ImGui::BeginChild(str_id, ImVec2(w, h), border, flags);
    }
    else
    {
        ImGuiID id = luaL_checkinteger(L, 2);
        result = ImGui::BeginChild(id, ImVec2(w, h), border, flags);
    }

    lua_pushboolean(L, result);

    return 1;
}

int ImGui_impl_EndChild(lua_State *L)
{
    ImGui::EndChild();
    return 0;
}

// Windows Utilities
int ImGui_impl_IsWindowAppearing(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsWindowAppearing());
    return 1;
}

int ImGui_impl_IsWindowCollapsed(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsWindowCollapsed());
    return 1;
}

int ImGui_impl_IsWindowFocused(lua_State *L)
{
    ImGuiFocusedFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowFocused(flag));
    return 1;
}

int ImGui_impl_IsWindowHovered(lua_State *L)
{
    ImGuiHoveredFlags flag = luaL_optinteger(L, 2, 0);
    lua_pushboolean(L, ImGui::IsWindowHovered(flag));
    return 1;
}

int ImGui_impl_GetWindowPos(lua_State *L)
{
    ImVec2 pos = ImGui::GetWindowPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return  2;
}

int ImGui_impl_GetWindowSize(lua_State *L)
{
    ImVec2 size = ImGui::GetWindowSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return  2;
}

int ImGui_impl_GetWindowWidth(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetWindowWidth());
    return  1;
}

int ImGui_impl_GetWindowHeight(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetWindowHeight());
    return  1;
}

int ImGui_impl_SetNextWindowPos(lua_State *L)
{
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    const ImVec2& pos = ImVec2(x, y);
    ImGuiCond cond = luaL_optinteger(L, 4, 0);
    float px = luaL_optnumber(L, 5, 0);
    float py = luaL_optnumber(L, 6, 0);
    const ImVec2& pivot = ImVec2(px, py);

    ImGui::SetNextWindowPos(pos, cond, pivot);

    return 0;
}

int ImGui_impl_SetNextWindowSize(lua_State *L)
{
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);
    const ImVec2& size = ImVec2(w, h);
    ImGuiCond cond = luaL_optinteger(L, 4, 0);

    ImGui::SetNextWindowSize(size, cond);

    return 0;
}

//int ImGui_impl_SetNextWindowSizeConstraints(lua_State *L)
//{
//    const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL
//    return 0;
//}

int ImGui_impl_SetNextWindowContentSize(lua_State *L)
{
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);
    const ImVec2& size = ImVec2(w, h);

    ImGui::SetNextWindowContentSize(size);

    return 0;
}

int ImGui_impl_SetNextWindowCollapsed(lua_State *L)
{
    // bool collapsed, ImGuiCond cond = 0
    bool collapsed = lua_toboolean(L, 2) > 0;
    ImGuiCond cond = luaL_optinteger(L, 3, 0);

    ImGui::SetNextWindowCollapsed(collapsed, cond);
    return 0;
}

int ImGui_impl_SetNextWindowFocus(lua_State *L)
{
    ImGui::SetNextWindowFocus();
    return 0;
}

int ImGui_impl_SetNextWindowBgAlpha(lua_State *L)
{
    float alpha = luaL_checknumber(L, 2);
    ImGui::SetNextWindowBgAlpha(alpha);
    return 0;
}

int ImGui_impl_SetWindowPos(lua_State *L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char *name = luaL_checkstring(L, 2);
        float x = luaL_checknumber(L, 3);
        float y = luaL_checknumber(L, 4);
        const ImVec2& pos = ImVec2(x, y);
        ImGuiCond cond = luaL_optinteger(L, 5, 0);

        ImGui::SetWindowPos(name, pos, cond);
    }
    else
    {
        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);
        const ImVec2& pos = ImVec2(x, y);
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowPos(pos, cond);
    }

    return  0;
}

int ImGui_impl_SetWindowSize(lua_State *L)
{
    if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char *name = luaL_checkstring(L, 2);
        float w = luaL_checknumber(L, 3);
        float h = luaL_checknumber(L, 4);
        const ImVec2& size = ImVec2(w, h);
        ImGuiCond cond = luaL_optinteger(L, 5, 0);

        ImGui::SetWindowSize(name, size, cond);
    }
    else
    {
        float w = luaL_checknumber(L, 2);
        float h = luaL_checknumber(L, 3);
        const ImVec2& size = ImVec2(w, h);
        ImGuiCond cond = luaL_optinteger(L, 4, 0);

        ImGui::SetWindowSize(size, cond);
    }

    return 0;
}

int ImGui_impl_SetWindowCollapsed(lua_State *L)
{
    if (lua_gettop(L) == 4)
    {
        const char *name = luaL_checkstring(L, 2);
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

int ImGui_impl_SetWindowFocus(lua_State *L)
{
    if (lua_gettop(L) == 2)
    {
        const char *name = luaL_checkstring(L, 2);
        ImGui::SetWindowFocus(name);
    }
    else
        ImGui::SetWindowFocus();

    return 0;
}

int ImGui_impl_SetWindowFontScale(lua_State *L)
{
    float scale = luaL_checknumber(L, 2);
    ImGui::SetWindowFontScale(scale);
    return 0;
}

// Content region
// - Those functions are bound to be redesigned soon (they are confusing, incomplete and return values in local window coordinates which increases confusion)
int ImGui_impl_GetContentRegionMax(lua_State *L)
{
    ImVec2 max = ImGui::GetContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetContentRegionAvail(lua_State *L)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    lua_pushnumber(L, avail.x);
    lua_pushnumber(L, avail.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionMin(lua_State *L)
{
    ImVec2 min = ImGui::GetWindowContentRegionMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionMax(lua_State *L)
{
    ImVec2 max = ImGui::GetWindowContentRegionMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetWindowContentRegionWidth(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetWindowContentRegionWidth());
    return 1;
}

// Windows Scrolling
int ImGui_impl_GetScrollX(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetScrollX());
    return 1;
}

int ImGui_impl_GetScrollY(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetScrollY());
    return 1;
}

int ImGui_impl_GetScrollMaxX(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxX());
    return 1;
}

int ImGui_impl_GetScrollMaxY(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetScrollMaxY());
    return 1;
}

int ImGui_impl_SetScrollX(lua_State *L)
{
    float scroll_x = luaL_checknumber(L, 2);
    ImGui::SetScrollX(scroll_x);
    return 0;
}

int ImGui_impl_SetScrollY(lua_State *L)
{
    float scroll_y = luaL_checknumber(L, 2);
    ImGui::SetScrollY(scroll_y);
    return 0;
}

int ImGui_impl_SetScrollHereX(lua_State *L)
{
    float center_x_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereX(center_x_ratio);
    return 0;
}

int ImGui_impl_SetScrollHereY(lua_State *L)
{
    float center_y_ratio = luaL_optnumber(L, 2, 0.5f);
    ImGui::SetScrollHereY(center_y_ratio);
    return 0;
}

int ImGui_impl_SetScrollFromPosX(lua_State *L)
{
    float local_x = luaL_checknumber(L, 2);
    float center_x_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosX(local_x, center_x_ratio);
    return 0;
}

int ImGui_impl_SetScrollFromPosY(lua_State *L)
{
    float local_y = luaL_checknumber(L, 2);
    float center_y_ratio = luaL_optnumber(L, 3, 0.5f);
    ImGui::SetScrollFromPosY(local_y, center_y_ratio);
    return 0;
}

// Parameters stacks (shared)
int ImGui_impl_PushStyleColor(lua_State *L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);

    ImGui::PushStyleColor(idx, GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)));

    return 0;
}

int ImGui_impl_PopStyleColor(lua_State *L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleColor(count);
    return 0;
}

int ImGui_impl_PushStyleVar(lua_State *L)
{
    ImGuiStyleVar idx = luaL_checkinteger(L, 2);

    if (lua_type(L, 4) != LUA_TNIL)
    {
        float vx = luaL_checknumber(L, 3);
        float vy = luaL_checknumber(L, 4);
        ImGui::PushStyleVar(idx, ImVec2(vx, vy));
    }
    else
    {
        float val = luaL_checknumber(L, 3);
        ImGui::PushStyleVar(idx, val);
    }
    return 0;
}

int ImGui_impl_PopStyleVar(lua_State *L)
{
    int count = luaL_optinteger(L, 2, 1);
    ImGui::PopStyleVar(count);
    return 0;
}

int ImGui_impl_GetStyleColor(lua_State *L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);
    const ImVec4 col = ImGui::GetStyleColorVec4(idx);
    GColor out = GColor::toHex(col);
    lua_pushnumber(L, out.hex);
    lua_pushnumber(L, out.alpha);
    return 2;
}

int ImGui_impl_GetFontSize(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetFontSize());
    return 1;
}

// Parameters stacks (current window)
int ImGui_impl_PushItemWidth(lua_State *L)
{
    float item_width = luaL_checknumber(L, 2);
    ImGui::PushItemWidth(item_width);
    return 0;
}

int ImGui_impl_PopItemWidth(lua_State *L)
{
    ImGui::PopItemWidth();
    return 0;
}

int ImGui_impl_SetNextItemWidth(lua_State *L)
{
    float item_width = luaL_checknumber(L, 2);
    ImGui::SetNextItemWidth(item_width);
    return 0;
}

int ImGui_impl_CalcItemWidth(lua_State *L)
{
    lua_pushnumber(L, ImGui::CalcItemWidth());
    return 1;
}

int ImGui_impl_PushTextWrapPos(lua_State *L)
{
    float wrap_local_pos_x = luaL_optnumber(L, 2, 0.0f);
    ImGui::PushTextWrapPos(wrap_local_pos_x);
    return 0;
}

int ImGui_impl_PopTextWrapPos(lua_State *L)
{
    ImGui::PopTextWrapPos();
    return 0;
}

int ImGui_impl_PushAllowKeyboardFocus(lua_State *L)
{
    bool allow_keyboard_focus = lua_toboolean(L, 2) > 0;
    ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
    return 0;
}

int ImGui_impl_PopAllowKeyboardFocus(lua_State *L)
{
    ImGui::PopAllowKeyboardFocus();
    return 0;
}

int ImGui_impl_PushButtonRepeat(lua_State *L)
{
    bool repeat = lua_toboolean(L, 2) > 0;
    ImGui::PushButtonRepeat(repeat);
    return 0;
}

int ImGui_impl_PopButtonRepeat(lua_State *L)
{
    ImGui::PopButtonRepeat();
    return 0;
}

// Cursor / Layout
int ImGui_impl_Separator(lua_State *L)
{
    ImGui::Separator();
    return 0;
}

int ImGui_impl_SameLine(lua_State *L)
{
    float offset_from_start_x = luaL_optnumber(L, 2, 0.0f);
    float spacing = luaL_optnumber(L, 3, -1.0f);
    ImGui::SameLine(offset_from_start_x, spacing);
    return 0;
}

int ImGui_impl_NewLine(lua_State *L)
{
    ImGui::NewLine();
    return 0;
}

int ImGui_impl_Spacing(lua_State *L)
{
    ImGui::Spacing();
    return 0;
}

int ImGui_impl_Dummy(lua_State *L)
{
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);

    ImGui::Dummy(ImVec2(w, h));
    return 0;
}

int ImGui_impl_Indent(lua_State *L)
{
    float indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Indent(indent_w);
    return 0;
}

int ImGui_impl_Unindent(lua_State *L)
{
    float indent_w = luaL_optnumber(L, 2, 0.0f);
    ImGui::Unindent(indent_w);
    return 0;
}

int ImGui_impl_BeginGroup(lua_State *L)
{
    ImGui::BeginGroup();
    return 0;
}

int ImGui_impl_EndGroup(lua_State *L)
{
    ImGui::EndGroup();
    return 0;
}

int ImGui_impl_GetCursorPos(lua_State *L)
{
    ImVec2 pos = ImGui::GetCursorPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_GetCursorPosX(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetCursorPosX());
    return 1;
}

int ImGui_impl_GetCursorPosY(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetCursorPosY());
    return 1;
}

//void SetCursorPos(const ImVec2& local_pos);

//void SetCursorPosX(float local_x);

//void SetCursorPosY(float local_y);

int ImGui_impl_GetCursorStartPos(lua_State *L)
{
    ImVec2 pos = ImGui::GetCursorStartPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_GetCursorScreenPos(lua_State *L)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

//void SetCursorScreenPos(const ImVec2& pos);

int ImGui_impl_AlignTextToFramePadding(lua_State *L)
{
    ImGui::AlignTextToFramePadding();
    return 0;
}

int ImGui_impl_GetTextLineHeight(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeight());
    return 1;
}

int ImGui_impl_GetTextLineHeightWithSpacing(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetTextLineHeightWithSpacing());
    return 1;
}

int ImGui_impl_GetFrameHeight(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetFrameHeight());
    return 1;
}

int ImGui_impl_GetFrameHeightWithSpacing(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetFrameHeightWithSpacing());
    return 1;
}

int ImGui_impl_PushID(lua_State *L)
{
    if (lua_gettop(L) == 2)
    {
        const int arg_type = lua_type(L, 2);
        switch(arg_type)
        {
        case(LUA_TNIL):
            LUAError(L, "bad argument #2 to 'pushID' (string/number/table/function expected, got nil)");
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

int ImGui_impl_PopID(lua_State *L)
{
    ImGui::PopID();
    return 0;
}

int ImGui_impl_GetID(lua_State *L)
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

int ImGui_impl_TextUnformatted(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    const char* text_end = luaL_optstring(L, 3, NULL);
    ImGui::TextUnformatted(text, text_end);
    return 0;
}

int ImGui_impl_Text(lua_State *L)
{
    const char *text = luaL_checkstring(L, 2);
    ImGui::Text("%s", text);
    return 0;
}

int ImGui_impl_TextColored(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextColored(GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f)), "%s", text);
    return 0;
}

int ImGui_impl_TextDisabled(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextDisabled("%s", text);
    return 0;
}

int ImGui_impl_TextWrapped(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::TextWrapped("%s", text);
    return 0;
}

int ImGui_impl_LabelText(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImGui::LabelText(label, "%s", text);
    return 0;
}

int ImGui_impl_BulletText(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::BulletText("%s", text);
    return 0;
}

// Widgets: Main
int ImGui_impl_Button(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    float w = luaL_optnumber(L, 3, 0.0f);
    float h = luaL_optnumber(L, 4, 0.0f);
    const ImVec2& size = ImVec2(w, h);
    lua_pushboolean(L, ImGui::Button(label, size));
    return 1;
}

int ImGui_impl_SmallButton(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    lua_pushboolean(L, ImGui::SmallButton(label));
    return 1;
}

int ImGui_impl_InvisibleButton(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    float w = luaL_optnumber(L, 3, 0.0f);
    float h = luaL_optnumber(L, 4, 0.0f);
    const ImVec2& size = ImVec2(w, h);
    lua_pushboolean(L, ImGui::InvisibleButton(str_id, size));
    return 1;
}

int ImGui_impl_ArrowButton(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiDir dir = luaL_checknumber(L, 3);
    lua_pushboolean(L, ImGui::ArrowButton(str_id, dir));
    return 1;
}

int ImGui_impl_Image(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 5, 0xffffff), luaL_optnumber(L, 6, 1.0f));
    ImVec4 border = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 9, data.width), luaL_optnumber(L, 10, data.height));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 11, 0.0f), luaL_optnumber(L, 12, 0.0f));

    ImGui::Image(data.texture, size, uv0, uv1, tint, border);
    return 0;
}

int ImGui_impl_ImageButton(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    const ImVec2& size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    int frame_padding = luaL_optinteger(L, 5, -1);
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 0.0f));
    ImVec4 tint = GColor::toVec4(luaL_optinteger(L, 8, 0xffffff), luaL_optnumber(L, 9, 1.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 10, data.width), luaL_optnumber(L, 11, data.height));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 12, 0.0f), luaL_optnumber(L, 13, 0.0f));

    lua_pushboolean(L, ImGui::ImageButton(data.texture, size, uv0, uv1, frame_padding, bg_col, tint));
    return 1;
}

//ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)


int ImGui_impl_ImageButtonWithText(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    const char* label = luaL_checkstring(L, 3);
    ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    int frame_padding = luaL_optinteger(L, 6, -1);
    ImVec4 bg_col = GColor::toVec4(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, 0.0f));
    ImVec4 tint_col = GColor::toVec4(luaL_optinteger(L, 9, 0xffffff), luaL_optnumber(L, 10, 1.0f));
    const ImVec2& uv1 = ImVec2(luaL_optnumber(L, 11, data.width), luaL_optnumber(L, 12, data.height));
    const ImVec2& uv0 = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));
    lua_pushboolean(L, ImGui::ImageButtonWithText(data.texture, label, size, uv0, uv1, frame_padding, bg_col, tint_col));
    return 1;
}

int ImGui_impl_Checkbox(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool v = lua_toboolean2(L, 3) > 0;
    bool result = ImGui::Checkbox(label, &v);
    lua_pushboolean(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_CheckboxFlags(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    unsigned int* flags = 0;
    unsigned int flags_value = 0;

    lua_pushboolean(L, ImGui::CheckboxFlags(label, flags, flags_value));
    return 1;
}

int ImGui_impl_RadioButton(lua_State *L)
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

int ImGui_impl_ProgressBar(lua_State *L)
{
    float fraction = luaL_checknumber(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, -1.0f), luaL_optnumber(L, 4, 0.0f));
    const char* overlay = luaL_optstring(L, 5, NULL);
    ImGui::ProgressBar(fraction, size, overlay);
    return  0;
}

int ImGui_impl_Bullet(lua_State *L)
{
    ImGui::Bullet();
    return 0;
}

// Widgets: Combo Box
int ImGui_impl_BeginCombo(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* preview_value = luaL_checkstring(L, 3);
    ImGuiComboFlags flags = luaL_optinteger(L, 4, 0);
    lua_pushboolean(L, ImGui::BeginCombo(label, preview_value, flags));
    return 1;
}

int ImGui_impl_EndCombo(lua_State *L)
{
    ImGui::EndCombo();
    return 0;
}

int ImGui_impl_Combo(lua_State *L)
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

    if (lua_type(L, 4) == LUA_TTABLE)
    {
        luaL_checktype(L, 4, LUA_TTABLE);
        size_t len = luaL_getn(L, 4);
        if (len == 0)
        {
            lua_pushnumber(L, -1);
            lua_pushboolean(L, false);
            return 2;
        }

        const char* items[len];
        lua_pushvalue(L, 4);
        for (int i = 0; i < len; i++)
        {
            lua_rawgeti(L, 4, i+1);
            const char* str = lua_tostring(L,-1);
            items[i] = str;
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        result = ImGui::Combo(label, &item_current, items, len, maxItems);
    }
    else if (lua_type(L, 4) == LUA_TSTRING)
    {
        const char* items = luaL_checkstring(L, 4);

        result = ImGui::Combo(label, &item_current, items, maxItems);
    }
    else
    {
        lua_pushstring(L, "Incorrect data type to #4");
        lua_error(L);
        return 0;
    }

    lua_pushinteger(L, item_current);
    lua_pushboolean(L, result);
    return 2;
}

// Widgets: Drags
int ImGui_impl_DragFloat(lua_State *L)
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

int ImGui_impl_DragFloat2(lua_State *L)
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
    bool result = ImGui::DragFloat2(label, &vec2f[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_DragFloat3(lua_State *L)
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
    bool result = ImGui::DragFloat3(label, &vec3f[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec3f[0]);
    lua_pushnumber(L, vec3f[1]);
    lua_pushnumber(L, vec3f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_DragFloat4(lua_State *L)
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
    bool result = ImGui::DragFloat4(label, &vec4f[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec4f[0]);
    lua_pushnumber(L, vec4f[1]);
    lua_pushnumber(L, vec4f[2]);
    lua_pushnumber(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_DragFloatRange2(lua_State *L)
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

int ImGui_impl_DragInt(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    int v = luaL_checkinteger(L, 3);
    float v_speed = luaL_optnumber(L, 4, 1.0f);
    int v_min = luaL_optinteger(L, 5, 0);
    int v_max = luaL_optinteger(L, 6, 0);
    const char* format = luaL_optstring(L, 7, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::DragInt(label, &v, v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_DragInt2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[2];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);

    float v_speed = luaL_optnumber(L, 5, 1.0f);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::DragInt(label, &vec2i[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_DragInt3(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 3);
    vec3i[1] = luaL_checkinteger(L, 4);
    vec3i[2] = luaL_checkinteger(L, 5);

    float v_speed = luaL_optnumber(L, 5, 1.0f);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, 0);

    bool result = ImGui::DragInt(label, &vec3i[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_DragInt4(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 3);
    vec4i[1] = luaL_checkinteger(L, 4);
    vec4i[2] = luaL_checkinteger(L, 5);
    vec4i[2] = luaL_checkinteger(L, 6);

    float v_speed = luaL_optnumber(L, 7, 1.0f);
    int v_min = luaL_optinteger(L, 8, 0);
    int v_max = luaL_optinteger(L, 9, 0);
    const char* format = luaL_optstring(L, 10, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, 0);

    bool result = ImGui::DragInt(label, &vec4i[0], v_speed, v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_DragIntRange2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    int v_current_min = luaL_checkinteger(L, 3);
    int v_current_max = luaL_checkinteger(L, 4);
    float v_speed = luaL_optnumber(L, 5, 1.0f);
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

int ImGui_impl_DragScalar(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    ImGuiDataType data_type = luaL_checkinteger(L, 3);
    double value = luaL_checknumber(L, 4);
    float v_speed = luaL_checknumber(L, 5);
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
int ImGui_impl_SliderFloat(lua_State *L)
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

int ImGui_impl_SliderFloat2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec2f[3];
    vec2f[0] = luaL_checknumber(L, 3);
    vec2f[1] = luaL_checknumber(L, 4);
    float v_min = luaL_checknumber(L, 5);
    float v_max = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::SliderFloat(label, &vec2f[0], v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_SliderFloat3(lua_State *L)
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

    bool result = ImGui::SliderFloat3(label, &vec3f[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3f[0]);
    lua_pushinteger(L, vec3f[1]);
    lua_pushinteger(L, vec3f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_SliderFloat4(lua_State *L)
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

    bool result = ImGui::SliderFloat3(label, &vec4f[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushinteger(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_SliderAngle(lua_State *L)
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

int ImGui_impl_SliderInt(lua_State *L)
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

int ImGui_impl_SliderInt2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[4];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);
    int v_min = luaL_optinteger(L, 5, 0);
    int v_max = luaL_optinteger(L, 6, 0);
    const char* format = luaL_optstring(L, 7, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 8, 0);

    bool result = ImGui::SliderInt2(label, &vec2i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_SliderInt3(lua_State *L)
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

    bool result = ImGui::SliderInt3(label, &vec3i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_SliderInt4(lua_State *L)
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

    bool result = ImGui::SliderInt4(label, &vec4i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_SliderScalar(lua_State *L)
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

int ImGui_impl_VSliderFloat(lua_State *L)
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

int ImGui_impl_VSliderInt(lua_State *L)
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

int ImGui_impl_VSliderScalar(lua_State *L)
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

int ImGui_impl_FilledSliderFloat(lua_State *L)
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

int ImGui_impl_FilledSliderFloat2(lua_State *L)
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

    bool result = ImGui::FilledSliderFloat(label, mirror, &vec2f[0], v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_FilledSliderFloat3(lua_State *L)
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

    bool result = ImGui::FilledSliderFloat3(label, mirror, &vec4f[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_FilledSliderFloat4(lua_State *L)
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

    bool result = ImGui::FilledSliderFloat3(label, mirror, &vec4f[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4f[0]);
    lua_pushinteger(L, vec4f[1]);
    lua_pushinteger(L, vec4f[2]);
    lua_pushinteger(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_FilledSliderAngle(lua_State *L)
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

int ImGui_impl_FilledSliderInt(lua_State *L)
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

int ImGui_impl_FilledSliderInt2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static int vec2i[4];
    vec2i[0] = luaL_checkinteger(L, 4);
    vec2i[1] = luaL_checkinteger(L, 5);
    int v_min = luaL_optinteger(L, 6, 0);
    int v_max = luaL_optinteger(L, 7, 0);
    const char* format = luaL_optstring(L, 8, "%d");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 9, NULL);

    bool result = ImGui::FilledSliderInt2(label, mirror, &vec2i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_FilledSliderInt3(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 4);
    vec3i[1] = luaL_checkinteger(L, 5);
    vec3i[2] = luaL_checkinteger(L, 6);
    int v_min = luaL_optinteger(L, 7, 0);
    int v_max = luaL_optinteger(L, 8, 0);
    const char* format = luaL_optstring(L, 9, NULL);
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, NULL);

    bool result = ImGui::FilledSliderInt3(label, mirror, &vec3i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_FilledSliderInt4(lua_State *L)
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
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 11, NULL);

    bool result = ImGui::FilledSliderInt4(label, mirror, &vec4i[0], v_min, v_max, format, sliderFlag);

    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_FilledSliderScalar(lua_State *L)
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

int ImGui_impl_VFilledSliderFloat(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool mirror = lua_toboolean(L, 3) > 0;
    const ImVec2 size = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    float v = luaL_checknumber(L, 6);
    float v_min = luaL_checknumber(L, 7);
    float v_max = luaL_checknumber(L, 8);
    const char* format = luaL_optstring(L, 9, "%.3f");
    ImGuiSliderFlags sliderFlag = luaL_optinteger(L, 10, NULL);

    bool result = ImGui::VFilledSliderFloat(label, mirror, size, &v, v_min, v_max, format, sliderFlag);

    lua_pushnumber(L, v);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_VFilledSliderInt(lua_State *L)
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

int ImGui_impl_VFilledSliderScalar(lua_State *L)
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
// Widgets: Input with Keyboard
int ImGui_impl_InputText(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    unsigned int size_t = luaL_checkinteger(L, 4);
    char buffer[size_t];
    sprintf(buffer, "%s", text);

    ImGuiInputTextFlags flags = luaL_optinteger(L, 5, 0);
    //ImGuiInputTextCallback callback = NULL;
    //void* user_data = NULL;

    bool result = ImGui::InputText(label, buffer, size_t, flags);

    lua_pushstring(L, &buffer[0]);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_InputTextMultiline(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    size_t buf_size = luaL_checkinteger(L, 4);
    char buffer[buf_size];
    sprintf(buffer, "%s", text);

    ImVec2 size = ImVec2(luaL_optnumber(L, 5, 0.0f), luaL_optnumber(L, 6, 0.0f));
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);
    // ImGuiInputTextCallback callback = NULL; void* user_data = NULL;

    bool result = ImGui::InputTextMultiline(label, buffer, buf_size, size, flags);
    lua_pushstring(L, &buffer[0]);
    lua_pushboolean(L, result);
    return 2;

}

int ImGui_impl_InputTextWithHint(lua_State *L)
{

    const char* label = luaL_checkstring(L, 2);
    const char* text = luaL_checkstring(L, 3);
    const char* hint = luaL_checkstring(L, 4);
    size_t buf_size = luaL_checkinteger(L, 5);
    char buffer[buf_size];
    sprintf(buffer, "%s", text);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);
    // ImGuiInputTextCallback callback = NULL; void* user_data = NULL;

    bool result = ImGui::InputTextWithHint(label, hint, buffer, buf_size, flags);
    lua_pushstring(L, &buffer[0]);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_InputFloat(lua_State *L)
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

int ImGui_impl_InputFloat2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec2f[2];
    vec2f[0] = luaL_checknumber(L, 3);
    vec2f[1] = luaL_checknumber(L, 4);
    const char* format = luaL_optstring(L, 5, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputFloat2(label, &vec2f[0], format, flags);
    lua_pushnumber(L, vec2f[0]);
    lua_pushnumber(L, vec2f[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_InputFloat3(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec3f[3];
    vec3f[0] = luaL_checknumber(L, 3);
    vec3f[1] = luaL_checknumber(L, 4);
    vec3f[2] = luaL_checknumber(L, 5);
    const char* format = luaL_optstring(L, 6, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputFloat3(label, &vec3f[0], format, flags);
    lua_pushnumber(L, vec3f[0]);
    lua_pushnumber(L, vec3f[1]);
    lua_pushnumber(L, vec3f[2]);
   lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_InputFloat4(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static float vec4f[4];
    vec4f[0] = luaL_checknumber(L, 3);
    vec4f[1] = luaL_checknumber(L, 4);
    vec4f[2] = luaL_checknumber(L, 5);
    vec4f[3] = luaL_checknumber(L, 6);
    const char* format = luaL_optstring(L, 7, "%.3f");
    ImGuiInputTextFlags flags = luaL_optinteger(L, 8, 0);

    bool result = ImGui::InputFloat4(label, &vec4f[0], format, flags);
    lua_pushnumber(L, vec4f[0]);
    lua_pushnumber(L, vec4f[1]);
    lua_pushnumber(L, vec4f[2]);
    lua_pushnumber(L, vec4f[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_InputInt(lua_State *L)
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

int ImGui_impl_InputInt2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec2i[2];
    vec2i[0] = luaL_checkinteger(L, 3);
    vec2i[1] = luaL_checkinteger(L, 4);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 5, 0);

    bool result = ImGui::InputInt2(label, &vec2i[0], flags);
    lua_pushinteger(L, vec2i[0]);
    lua_pushinteger(L, vec2i[1]);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_InputInt3(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec3i[3];
    vec3i[0] = luaL_checkinteger(L, 3);
    vec3i[1] = luaL_checkinteger(L, 4);
    vec3i[2] = luaL_checkinteger(L, 5);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 6, 0);

    bool result = ImGui::InputInt3(label, &vec3i[0], flags);
    lua_pushinteger(L, vec3i[0]);
    lua_pushinteger(L, vec3i[1]);
    lua_pushinteger(L, vec3i[2]);
    lua_pushboolean(L, result);
    return 4;
}

int ImGui_impl_InputInt4(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int vec4i[4];
    vec4i[0] = luaL_checkinteger(L, 3);
    vec4i[1] = luaL_checkinteger(L, 4);
    vec4i[2] = luaL_checkinteger(L, 5);
    vec4i[3] = luaL_checkinteger(L, 6);
    ImGuiInputTextFlags flags = luaL_optinteger(L, 7, 0);

    bool result = ImGui::InputInt4(label, &vec4i[0], flags);
    lua_pushinteger(L, vec4i[0]);
    lua_pushinteger(L, vec4i[1]);
    lua_pushinteger(L, vec4i[2]);
    lua_pushinteger(L, vec4i[3]);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_InputDouble(lua_State *L)
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

int ImGui_impl_InputScalar(lua_State *L)
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
int ImGui_impl_ColorEdit3(lua_State *L)
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

int ImGui_impl_ColorEdit4(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);

    bool result = ImGui::ColorEdit4(label, (float *)&col, flags);

    GColor conv = GColor::toHex(col);
    lua_pushnumber(L, conv.hex);
    lua_pushnumber(L, conv.alpha);
    lua_pushboolean(L, result);
    return 3;
}

int ImGui_impl_ColorPicker3(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::ColorPicker3(label, (float *)&col, flags);

    GColor conv = GColor::toHex(col);
    lua_pushnumber(L, conv.hex);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_ColorPicker4(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec4 col = GColor::toVec4(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    ImGuiColorEditFlags flags = luaL_optinteger(L, 5, 0);
    ImVec4 refCol = GColor::toVec4(luaL_optinteger(L, 6, 0xffffff), luaL_optnumber(L, 7, 1.0f));

    bool result = ImGui::ColorPicker4(label, (float *)&col, flags, (float *)&refCol);

    GColor conv1 = GColor::toHex(col);
    GColor conv2 = GColor::toHex(refCol);
    lua_pushnumber(L, conv1.hex);
    lua_pushnumber(L, conv1.alpha);
    lua_pushnumber(L, conv2.hex);
    lua_pushnumber(L, conv2.alpha);
    lua_pushboolean(L, result);
    return 5;
}

int ImGui_impl_ColorButton(lua_State *L)
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

int ImGui_impl_SetColorEditOptions(lua_State *L)
{
    ImGuiColorEditFlags flags = luaL_checkinteger(L, 2);
    ImGui::SetColorEditOptions(flags);
    return 0;
}

// Widgets: Trees
int ImGui_impl_TreeNode(lua_State *L)
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

int ImGui_impl_TreeNodeEx(lua_State *L)
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

int ImGui_impl_TreePush(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGui::TreePush(str_id);
    return 0;
}

int ImGui_impl_TreePop(lua_State *L)
{
    ImGui::TreePop();
    return 0;
}

int ImGui_impl_GetTreeNodeToLabelSpacing(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetTreeNodeToLabelSpacing());
    return 1;
}

// FIXME lua_type
int ImGui_impl_CollapsingHeader(lua_State *L)
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

int ImGui_impl_SetNextItemOpen(lua_State *L)
{
    bool is_open = lua_toboolean(L, 2);
    ImGuiCond cond = luaL_optinteger(L, 3, 0);
    ImGui::SetNextItemOpen(is_open, cond);
    return 0;
}

// Widgets: Selectables
int ImGui_impl_Selectable(lua_State *L)
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
int ImGui_impl_ListBox(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    static int current_item = luaL_checkinteger(L, 3);

    luaL_checktype(L, 4, LUA_TTABLE);
    int len = luaL_getn(L, 4);
    const char* items[len];
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
    return 2;
}

int ImGui_impl_ListBoxHeader(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    ImVec2 size = ImVec2(luaL_optnumber(L, 3, 0), luaL_optnumber(L, 4, 0));

    lua_pushboolean(L, ImGui::ListBoxHeader(label, size));
    return 1;
}

int ImGui_impl_ListBoxHeader2(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    int items_count = luaL_checkinteger(L, 3);

    lua_pushboolean(L, ImGui::ListBoxHeader(label, items_count));
    return 1;
}

int ImGui_impl_ListBoxFooter(lua_State *L)
{
    ImGui::ListBoxFooter();
    return 0;
}

// Widgets: Data Plotting
int ImGui_impl_PlotLines(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);

    luaL_checktype(L, 3, LUA_TTABLE);
    int len = luaL_getn(L, 3);
    float values[len];
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

    ImGui::PlotLines(label, (float*)&values, len, values_offset, overlay_text, scale_min, scale_max, graph_size, stride);

    return 0;
}

int ImGui_impl_PlotHistogram(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);

    luaL_checktype(L, 3, LUA_TTABLE);
    int len = luaL_getn(L, 3);
    float values[len];
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

    return 0;
}

// Widgets: Value() Helpers.
int ImGui_impl_Value(lua_State *L)
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
                float n = lua_tonumber(L, 3);
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
int ImGui_impl_BeginMenuBar(lua_State *L)
{
    lua_pushboolean(L, ImGui::BeginMenuBar());
    return 1;
}

int ImGui_impl_EndMenuBar(lua_State *L)
{
    ImGui::EndMenuBar();
    return 0;
}

int ImGui_impl_BeginMainMenuBar(lua_State *L)
{
    lua_pushboolean(L, ImGui::BeginMainMenuBar());
    return 1;
}

int ImGui_impl_EndMainMenuBar(lua_State *L)
{
    ImGui::EndMainMenuBar();
    return 0;
}

int ImGui_impl_BeginMenu(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool enabled = lua_toboolean(L, 3) > 0;
    lua_pushboolean(L, ImGui::BeginMenu(label, enabled));
    return 1;
}

int ImGui_impl_EndMenu(lua_State *L)
{
    ImGui::EndMenu();
    return 0;
}

int ImGui_impl_MenuItem(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    const char* shortcut = luaL_optstring(L, 3, NULL);
    int selected = luaL_optboolean(L, 4, 0);
    int enabled = luaL_optboolean(L, 5, 1);

    lua_pushboolean(L, ImGui::MenuItem(label, shortcut, selected, enabled));

    return 1;
}

int ImGui_impl_MenuItemWithShortcut(lua_State *L)
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
int ImGui_impl_BeginTooltip(lua_State *L)
{
    ImGui::BeginTooltip();
    return 0;
}

int ImGui_impl_EndTooltip(lua_State *L)
{
    ImGui::EndTooltip();
    return 0;
}

int ImGui_impl_SetTooltip(lua_State *L)
{
    const char* text = luaL_checkstring(L, 2);
    ImGui::SetTooltip("%s", text);
    return 0;
}

// Popups, Modals
int ImGui_impl_BeginPopup(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiWindowFlags flags = luaL_optinteger(L, 3, 0);
    lua_pushboolean(L, ImGui::BeginPopup(str_id, flags));
    return 1;
}

int ImGui_impl_BeginPopupModal(lua_State *L)
{
    const char* name = luaL_checkstring(L, 2);
    bool p_open = lua_tointeger(L, 3) > 0;
    ImGuiWindowFlags flags = luaL_optinteger(L, 4, 0);

    bool result = ImGui::BeginPopupModal(name, &p_open, flags);

    lua_pushboolean(L, p_open);
    lua_pushboolean(L, result);

    return 2;
}

int ImGui_impl_EndPopup(lua_State *L)
{
    ImGui::EndPopup();
    return 0;
}

// Popups: open/close functions
int ImGui_impl_OpenPopup(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 0);
    ImGui::OpenPopup(str_id, popup_flags);
    return 0;
}

int ImGui_impl_OpenPopupContextItem(lua_State *L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::OpenPopupContextItem(str_id, popup_flags));
    return 1;
}

int ImGui_impl_CloseCurrentPopup(lua_State *L)
{
    ImGui::CloseCurrentPopup();
    return 0;
}

// Popups: open+begin combined functions helpers
int ImGui_impl_BeginPopupContextItem(lua_State *L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextItem(str_id, popup_flags));
    return 1;
}

int ImGui_impl_BeginPopupContextWindow(lua_State *L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextWindow(str_id, popup_flags));
    return 1;
}

int ImGui_impl_BeginPopupContextVoid(lua_State *L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::BeginPopupContextVoid(str_id, popup_flags));
    return 1;
}

// Popups: test function
int ImGui_impl_IsPopupOpen(lua_State *L)
{
    const char* str_id = luaL_optstring(L, 2, NULL);
    ImGuiPopupFlags popup_flags = luaL_optinteger(L, 3, 1);

    lua_pushboolean(L, ImGui::IsPopupOpen(str_id, popup_flags));
    return 1;
}

// Columns
int ImGui_impl_Columns(lua_State *L)
{
    int count = luaL_optinteger(L, 2, 1);
    const char* id = luaL_optstring(L, 3, NULL);
    bool border = luaL_optboolean(L, 4, 1);

    ImGui::Columns(count, id, border);

    return 0;
}

int ImGui_impl_NextColumn(lua_State *L)
{
    ImGui::NextColumn();
    return 0;
}

int ImGui_impl_GetColumnIndex(lua_State *L)
{
    lua_pushinteger(L, ImGui::GetColumnIndex());
    return 1;
}

int ImGui_impl_GetColumnWidth(lua_State *L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnWidth(column_index));
    return 1;
}

int ImGui_impl_SetColumnWidth(lua_State *L)
{
    int column_index = luaL_checkinteger(L, 2);
    float width = luaL_checknumber(L, 3);
    ImGui::SetColumnWidth(column_index, width);
    return 0;
}

int ImGui_impl_GetColumnOffset(lua_State *L)
{
    int column_index = luaL_optinteger(L, 2, -1);
    lua_pushnumber(L, ImGui::GetColumnOffset(column_index));
    return 1;
}

int ImGui_impl_SetColumnOffset(lua_State *L)
{
    int column_index = luaL_checkinteger(L, 2);
    float offset = luaL_checknumber(L, 3);
    ImGui::SetColumnOffset(column_index, offset);
    return 0;
}

int ImGui_impl_GetColumnsCount(lua_State *L)
{
    lua_pushinteger(L, ImGui::GetColumnsCount());
    return 1;
}

// Tab Bars, Tabs
int ImGui_impl_BeginTabBar(lua_State *L)
{
    const char* str_id = luaL_checkstring(L, 2);
    ImGuiTabBarFlags flags = luaL_optinteger(L, 3, NULL);

    lua_pushboolean(L, ImGui::BeginTabBar(str_id, flags));
    return 1;
}

int ImGui_impl_EndTabBar(lua_State *L)
{
    ImGui::EndTabBar();
    return  0;
}

int ImGui_impl_BeginTabItem(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);
    bool p_open = lua_toboolean(L, 3) > 0;
    ImGuiTabItemFlags flags = luaL_optinteger(L, 4, NULL);

    bool result = ImGui::BeginTabItem(label, &p_open, flags);

    lua_pushboolean(L, p_open);
    lua_pushboolean(L, result);
    return 2;
}

int ImGui_impl_EndTabItem(lua_State *L)
{
    ImGui::EndTabItem();
    return 0;
}

int ImGui_impl_SetTabItemClosed(lua_State *L)
{
    const char* tab_or_docked_window_label = luaL_checkstring(L, 2);
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
    return 0;
}

// Logging/Capture
//void LogToTTY(int auto_open_depth = -1);
//void LogToFile(int auto_open_depth = -1, const char* filename = NULL);
//void LogToClipboard(int auto_open_depth = -1);
//void LogFinish();
//void LogButtons();
//void LogText(const char* fmt, ...) IM_FMTARGS(1);

// Drag and Drop
//bool BeginDragDropSource(ImGuiDragDropFlags flags = 0);
//bool SetDragDropPayload(const char* type, const void* data, size_t sz, ImGuiCond cond = 0);
//void EndDragDropSource();
//bool BeginDragDropTarget();
//const ImGuiPayload*   AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);
//void  EndDragDropTarget();
//const ImGuiPayload*   GetDragDropPayload();

// Clipping
int ImGui_impl_PushClipRect(lua_State *L)
{
    const ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    const ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = lua_toboolean(L, 6) > 0;
    ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int ImGui_impl_PopClipRect(lua_State *L)
{
    ImGui::PopClipRect();
    return 0;
}

// Focus, Activation
int ImGui_impl_SetItemDefaultFocus(lua_State *L)
{
    ImGui::SetItemDefaultFocus();
    return 0;
}

int ImGui_impl_SetKeyboardFocusHere(lua_State *L)
{
    int offset = luaL_optinteger(L, 2, NULL);
    ImGui::SetKeyboardFocusHere(offset);
    return 0;
}

// Item/Widgets Utilities
int ImGui_impl_IsItemHovered(lua_State *L)
{
    ImGuiHoveredFlags flags = luaL_optboolean(L, 2, NULL);
    lua_pushboolean(L, ImGui::IsItemHovered(flags));
    return 1;
}

int ImGui_impl_IsItemActive(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemActive());
    return 1;
}

int ImGui_impl_IsItemFocused(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemFocused());
    return 1;
}

int ImGui_impl_IsItemClicked(lua_State *L)
{
    ImGuiMouseButton mouse_button = convertGiderosMouseButton(L, luaL_optinteger(L, 2, 1));
    lua_pushboolean(L, ImGui::IsItemClicked(mouse_button));
    return 1;
}

int ImGui_impl_IsItemVisible(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemVisible());
    return 1;
}

int ImGui_impl_IsItemEdited(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemEdited());
    return 1;
}

int ImGui_impl_IsItemActivated(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemActivated());
    return 1;
}

int ImGui_impl_IsItemDeactivated(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivated());
    return 1;
}

int ImGui_impl_IsItemDeactivatedAfterEdit(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemDeactivatedAfterEdit());
    return 1;
}

int ImGui_impl_IsItemToggledOpen(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsItemToggledOpen());
    return 1;
}

int ImGui_impl_IsAnyItemHovered(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsAnyItemHovered());
    return 1;
}

int ImGui_impl_IsAnyItemActive(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsAnyItemActive());
    return 1;
}

int ImGui_impl_IsAnyItemFocused(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsAnyItemFocused());
    return 1;
}

int ImGui_impl_GetItemRectMin(lua_State *L)
{
    ImVec2 min = ImGui::GetItemRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_GetItemRectMax(lua_State *L)
{
    ImVec2 max = ImGui::GetItemRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_GetItemRectSize(lua_State *L)
{
    ImVec2 size = ImGui::GetItemRectSize();
    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);
    return 2;
}

int ImGui_impl_SetItemAllowOverlap(lua_State *L)
{
    ImGui::SetItemAllowOverlap();
    return 0;
}

// Miscellaneous Utilities
int ImGui_impl_IsRectVisible(lua_State *L)
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

int ImGui_impl_GetTime(lua_State *L)
{
    lua_pushnumber(L, ImGui::GetTime());
    return 1;
}

int ImGui_impl_GetFrameCount(lua_State *L)
{
    lua_pushinteger(L, ImGui::GetFrameCount());
    return 0;
}

int ImGui_impl_GetStyleColorName(lua_State *L)
{
    ImGuiCol idx = luaL_checkinteger(L, 2);
    lua_pushstring(L, ImGui::GetStyleColorName(idx));
    return 0;
}

int ImGui_impl_CalcListClipping(lua_State *L)
{
    //int items_count, float items_height, int* out_items_display_start, int* out_items_display_end
    int items_count = luaL_checkinteger(L, 2);
    float items_height = luaL_checknumber(L, 3);
    int out_items_display_start = luaL_checkinteger(L, 4);
    int out_items_display_end = luaL_checkinteger(L, 5);

    ImGui::CalcListClipping(items_count, items_height, &out_items_display_start, &out_items_display_end);
    lua_pushinteger(L, out_items_display_start);
    lua_pushinteger(L, out_items_display_end);
    return 2;
}

int ImGui_impl_BeginChildFrame(lua_State *L)
{
    ImGuiID id = luaL_checkinteger(L, 2);
    ImVec2 size = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImGuiWindowFlags flags = luaL_optinteger(L, 5, NULL);

    lua_pushboolean(L, ImGui::BeginChildFrame(id, size, flags));
    return 1;
}

int ImGui_impl_EndChildFrame(lua_State *L)
{
    ImGui::EndChildFrame();
    return 0;
}


// Text Utilities
int ImGui_impl_CalcTextSize(lua_State *L)
{
    //const char* text, const char* text_end = NULL, , float wrap_width = -1.0f);
    const char* text = luaL_checkstring(L, 2);
    const char* text_end = luaL_optstring(L, 3, NULL);
    bool hide_text_after_double_hash = luaL_optboolean(L, 4, 0);
    float wrap_width = luaL_optnumber(L, 5, NULL);

    ImVec2 size = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);

    lua_pushnumber(L, size.x);
    lua_pushnumber(L, size.y);

    return 2;
}
// Inputs Utilities: Keyboard
int ImGui_impl_GetKeyIndex(lua_State *L)
{
    ImGuiKey imgui_key = luaL_checkinteger(L, 2);
    lua_pushinteger(L, ImGui::GetKeyIndex(imgui_key));
    return  1;
}

int ImGui_impl_IsKeyDown(lua_State *L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyDown(user_key_index));
    return  1;
}

int ImGui_impl_IsKeyPressed(lua_State *L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    bool repeat = luaL_optboolean(L, 3, 1);

    lua_pushboolean(L, ImGui::IsKeyPressed(user_key_index, repeat));
    return 1;
}

int ImGui_impl_IsKeyReleased(lua_State *L)
{
    int user_key_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ImGui::IsKeyReleased(user_key_index));
    return 1;
}

//int  (int key_index, float repeat_delay, float rate);
int ImGui_impl_GetKeyPressedAmount(lua_State *L)
{
    int key_index = luaL_checkinteger(L, 2);
    float repeat_delay = luaL_checknumber(L, 3);
    float rate = luaL_checknumber(L, 4);
    lua_pushinteger(L, ImGui::GetKeyPressedAmount(key_index, repeat_delay, rate));
    return 1;
}

//void (bool want_capture_keyboard_value = true);
int ImGui_impl_CaptureKeyboardFromApp(lua_State *L)
{
    bool want_capture_keyboard_value = luaL_optboolean(L, 2, 1);
    ImGui::CaptureKeyboardFromApp(want_capture_keyboard_value);
    return 0;
}


// Inputs Utilities: Mouse
int ImGui_impl_IsMouseDown(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDown(button));
    return 1;
}

int ImGui_impl_IsMouseClicked(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    bool repeat = luaL_optboolean(L, 3, 0);
    lua_pushboolean(L, ImGui::IsMouseClicked(button, repeat));
    return 1;
}

int ImGui_impl_IsMouseReleased(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseReleased(button));
    return 1;
}

int ImGui_impl_IsMouseDoubleClicked(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    lua_pushboolean(L, ImGui::IsMouseDoubleClicked(button));
    return 1;
}

int ImGui_impl_IsMouseHoveringRect(lua_State *L)
{
    ImVec2 r_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 r_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool clip = luaL_optboolean(L, 6, 1);
    lua_pushboolean(L, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
    return 1;
}

int ImGui_impl_IsMousePosValid(lua_State *L)
{
    ImVec2 mouse_pos = ImVec2(luaL_optnumber(L, 2, -FLT_MAX), luaL_optnumber(L, 3, -FLT_MAX));
    lua_pushboolean(L, ImGui::IsMousePosValid(&mouse_pos));
    return 1;
}

int ImGui_impl_IsAnyMouseDown(lua_State *L)
{
    lua_pushboolean(L, ImGui::IsAnyMouseDown());
    return 1;
}

int ImGui_impl_GetMousePos(lua_State *L)
{
    ImVec2 pos = ImGui::GetMousePos();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return  2;
}

int ImGui_impl_GetMousePosOnOpeningCurrentPopup(lua_State *L)
{
    ImVec2 pos = ImGui::GetMousePosOnOpeningCurrentPopup();
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_IsMouseDragging(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, NULL);
    lua_pushboolean(L, ImGui::IsMouseDragging(button, lock_threshold));
    return 1;
}

int ImGui_impl_GetMouseDragDelta(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    float lock_threshold = luaL_optnumber(L, 3, NULL);
    ImVec2 pos = ImGui::GetMouseDragDelta(button, lock_threshold);
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    return 2;
}

int ImGui_impl_ResetMouseDragDelta(lua_State *L)
{
    ImGuiMouseButton button = convertGiderosMouseButton(L, lua_tointeger(L, 2));
    ImGui::ResetMouseDragDelta(button);
    return 0;
}

//ImGuiMouseCursor GetMouseCursor();
//void SetMouseCursor(ImGuiMouseCursor cursor_type);
//void CaptureMouseFromApp(bool want_capture_mouse_value = true);

/// STYLES

int ImGui_impl_StyleDark(lua_State *L)
{
    ImGui::StyleColorsDark();
    return 0;
}

int ImGui_impl_StyleLight(lua_State *L)
{
    ImGui::StyleColorsLight();
    return 0;
}

int ImGui_impl_StyleClassic(lua_State *L)
{
    ImGui::StyleColorsClassic();
    return 0;
}

/// DISPLAY SIZE

int ImGui_impl_SetDisplaySize(lua_State *L)
{
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize.x = w;
    io.DisplaySize.y = h;

    return 0;
}

int ImGui_impl_SetDisplayScale(lua_State *L)
{
    float sx = luaL_checknumber(L, 2);
    float sy = luaL_checknumber(L, 3);

    ImGuiIO& io = ImGui::GetIO();

    io.DisplayFramebufferScale.x = sx;
    io.DisplayFramebufferScale.y = sy;

    return 0;
}

/// Color Utilities
int ImGui_impl_ColorConvertRGBtoHSV(lua_State *L)
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

int ImGui_impl_ColorConvertHSVtoRGB(lua_State *L)
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

int ImGui_impl_ShowUserGuide(lua_State *L)
{
    ImGui::ShowUserGuide();

    return 0;
}

int ImGui_impl_ShowDemoWindow(lua_State *L)
{
    ImGui::ShowDemoWindow();

    return 0;
}

int ImGui_impl_ShowAboutWindow(lua_State *L)
{
    ImGui::ShowAboutWindow();

    return 0;
}

int ImGui_impl_ShowStyleEditor(lua_State *L)
{
    ImGui::ShowStyleEditor();

    return 0;
}

int ImGui_impl_ShowFontSelector(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);

    ImGui::ShowFontSelector(label);

    return 0;
}

int ImGui_impl_ShowMetricsWindow(lua_State *L)
{
    ImGui::ShowMetricsWindow();

    return 0;
}

int ImGui_impl_ShowStyleSelector(lua_State *L)
{
    const char* label = luaL_checkstring(L, 2);

    bool open = ImGui::ShowStyleSelector(label);

    lua_pushboolean(L, open);

    return 1;
}
////////////////////////////////////////////////////////////////////////////////
///
/// DRAW LIST
///
////////////////////////////////////////////////////////////////////////////////

int ImGui_impl_DrawList_PushClipRect(lua_State *L)
{
    ImVec2 clip_rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 clip_rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    bool intersect_with_current_clip_rect = luaL_optboolean(L, 6, 0) > 0;

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    return 0;
}

int ImGui_impl_DrawList_PushClipRectFullScreen(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PushClipRectFullScreen();
    return 0;
}

int ImGui_impl_DrawList_PopClipRect(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PopClipRect();
    return 0;
}

int ImGui_impl_DrawList_PushTextureID(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImTextureID texture_id = getTexture(L, 2).texture;
    list->PushTextureID(texture_id);
    return 0;
}

int ImGui_impl_DrawList_PopTextureID(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PopTextureID();
    return 0;
}

int ImGui_impl_DrawList_GetClipRectMin(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImVec2 min = list->GetClipRectMin();
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    return 2;
}

int ImGui_impl_DrawList_GetClipRectMax(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImVec2 max = list->GetClipRectMax();
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    return 2;
}

int ImGui_impl_DrawList_AddLine(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    float thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddLine(p1, p2, col, thickness);
    return 0;
}

int ImGui_impl_DrawList_AddRect(lua_State *L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    float rounding = luaL_optnumber(L, 8, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 9, ImDrawCornerFlags_All);
    float thickness = luaL_optnumber(L, 10, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRect(p_min, p_max, col, rounding, rounding_corners, thickness);

    return 0;
}

int ImGui_impl_DrawList_AddRectFilled(lua_State *L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    float rounding = luaL_optnumber(L, 8, 0.0f);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 9, ImDrawCornerFlags_All);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRectFilled(p_min, p_max, col, rounding, rounding_corners);

    return 0;
}

int ImGui_impl_DrawList_AddRectFilledMultiColor(lua_State *L)
{
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImU32 col_upr_left  = GColor::toU32(luaL_checkinteger(L, 6), luaL_optnumber(L, 7, 1.0f));
    ImU32 col_upr_right = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));
    ImU32 col_bot_right = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));
    ImU32 col_bot_left  = GColor::toU32(luaL_checkinteger(L, 12), luaL_optnumber(L, 13, 1.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRectFilledMultiColor(p_min, p_max, col_upr_left, col_upr_right, col_bot_right, col_bot_left);

    return 0;
}

int ImGui_impl_DrawList_AddQuad(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));
    float thickness = luaL_optnumber(L, 12, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddQuad(p1, p2, p3, p4, col, thickness);

    return  0;
}

int ImGui_impl_DrawList_AddQuadFilled(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, 1.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddQuadFilled(p1, p2, p3, p4, col);

    return  0;
}

int ImGui_impl_DrawList_AddTriangle(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));
    float thickness = luaL_optnumber(L, 10, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddTriangle(p1, p2, p3, col, thickness);

    return  0;
}

int ImGui_impl_DrawList_AddTriangleFilled(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 8), luaL_optnumber(L, 9, 1.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddTriangleFilled(p1, p2, p3, col);

    return  0;
}

int ImGui_impl_DrawList_AddCircle(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);
    float thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddCircle(center, radius, col, num_segments, thickness);

    return 0;
}

int ImGui_impl_DrawList_AddCircleFilled(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddCircleFilled(center, radius, col, num_segments);

    return 0;
}

int ImGui_impl_DrawList_AddNgon(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);
    float thickness = luaL_optnumber(L, 8, 1.0f);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddNgon(center, radius, col, num_segments, thickness);

    return 0;
}

int ImGui_impl_DrawList_AddNgonFilled(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 5), luaL_optnumber(L, 6, 1.0f));
    int num_segments = luaL_optinteger(L, 7, 12);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddNgonFilled(center, radius, col, num_segments);

    return 0;
}

int ImGui_impl_DrawList_AddText(lua_State *L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 4), luaL_optnumber(L, 5, 1.0f));
    const char* text_begin = luaL_checkstring(L, 6);
    const char* text_end = luaL_optstring(L, 7, NULL);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddText(pos, col, text_begin, text_end);

    return 0;
}

//int ImGui_impl_DrawList_AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL);

int ImGui_impl_DrawList_AddPolyline(lua_State *L)
{

    luaL_checktype(L, 2, LUA_TTABLE);
    int index = 0;
    int num_points = luaL_getn(L, 2);
    ImVec2 points[num_points];
    lua_pushvalue(L, 2);
    for (int i = 0; i < num_points; i+=2)
    {
        lua_rawgeti(L, 2, i+1);
        float x = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, 2, i+2);
        float y = luaL_checknumber(L, -1);
        points[index] = ImVec2(x,y);

        index ++;

        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    ImU32 col = GColor::toU32(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));
    bool closed = lua_toboolean(L, 5) > 0;
    float thickness = luaL_checknumber(L, 6);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddPolyline(points, index, col, closed, thickness);

    return  0;
}

int ImGui_impl_DrawList_AddConvexPolyFilled(lua_State *L)
{

    luaL_checktype(L, 2, LUA_TTABLE);
    int index = 0;
    int num_points = luaL_getn(L, 2);
    ImVec2 points[num_points];
    lua_pushvalue(L, 2);
    for (int i = 0; i < num_points; i+=2)
    {
        lua_rawgeti(L, 2, i+1);
        float x = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_rawgeti(L, 2, i+2);
        float y = luaL_checknumber(L, -1);
        points[index] = ImVec2(x,y);

        index ++;

        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    ImU32 col = GColor::toU32(luaL_checkinteger(L, 3), luaL_optnumber(L, 4, 1.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddConvexPolyFilled(points, index, col);

    return  0;
}

int ImGui_impl_DrawList_AddBezierCurve(lua_State *L)
{
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 8), luaL_checknumber(L, 9));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 10), luaL_optnumber(L, 11, NULL));
    float thickness = luaL_checknumber(L, 12);
    int num_segments = luaL_optinteger(L, 13, 0);

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddBezierCurve(p1, p2, p3, p4, col, thickness, num_segments);
    return 0;
}

int ImGui_impl_DrawList_AddImage(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 7, 0xffffff), luaL_optnumber(L, 8, NULL));
    ImVec2 uv_max = ImVec2(luaL_optnumber(L, 9, data.width), luaL_optnumber(L, 10, data.height));
    ImVec2 uv_min = ImVec2(luaL_optnumber(L, 11, 0.0f), luaL_optnumber(L, 12, 0.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddImage(data.texture, p_min, p_max, uv_min, uv_max, col);
    return 0;
}

int ImGui_impl_DrawList_AddImageQuad(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p1 = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 7), luaL_checknumber(L, 8));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 9), luaL_checknumber(L, 10));
    ImU32 col = GColor::toU32(luaL_optinteger(L, 11, 0xffffff), luaL_optnumber(L, 12, NULL));
    ImVec2 uv1 = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));
    ImVec2 uv2 = ImVec2(luaL_optnumber(L, 15, data.width), luaL_optnumber(L, 16, 0.0f));
    ImVec2 uv3 = ImVec2(luaL_optnumber(L, 17, data.width), luaL_optnumber(L, 18, data.height));
    ImVec2 uv4 = ImVec2(luaL_optnumber(L, 19, 0.0f), luaL_optnumber(L, 20, data.height));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddImageQuad(data.texture, p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);
    return 0;
}

int ImGui_impl_DrawList_AddImageRounded(lua_State *L)
{
    MyTextureData data = getTexture(L, 2);
    ImVec2 p_min = ImVec2(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ImVec2 p_max = ImVec2(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    ImU32 col = GColor::toU32(luaL_checkinteger(L, 7), luaL_optnumber(L, 8, NULL));
    float rounding = luaL_checknumber(L, 9);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 10, ImDrawCornerFlags_All);
    ImVec2 uv_max = ImVec2(luaL_optnumber(L, 11, data.width), luaL_optnumber(L, 12, data.height));
    ImVec2 uv_min = ImVec2(luaL_optnumber(L, 13, 0.0f), luaL_optnumber(L, 14, 0.0f));

    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddImageRounded(data.texture, p_min, p_max, uv_min, uv_max, col, rounding, rounding_corners);
    return 0;
}

int ImGui_impl_DrawList_PathClear(lua_State *L)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathClear();
    return 0;
}

int ImGui_impl_DrawList_PathLineTo(lua_State *L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathLineTo(pos);
    return 0;
}

int ImGui_impl_DrawList_PathLineToMergeDuplicate(lua_State *L)
{
    ImVec2 pos = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathLineToMergeDuplicate(pos);
    return 0;
}

int ImGui_impl_DrawList_PathFillConvex(lua_State *L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathFillConvex(color);
    return 0;

}

int ImGui_impl_DrawList_PathStroke(lua_State *L)
{
    ImU32 color = GColor::toU32(luaL_checkinteger(L, 2), luaL_optnumber(L, 3, 1.0f));
    bool closed = lua_toboolean(L, 4) > 0;
    float thickness = luaL_optnumber(L, 3, NULL);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathStroke(color, closed, thickness);
    return 0;
}

int ImGui_impl_DrawList_PathArcTo(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    float a_min = luaL_checknumber(L, 5);
    float a_max = luaL_checknumber(L, 6);
    int num_segments = luaL_optinteger(L, 7, NULL);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathArcTo(center, radius, a_min, a_max, num_segments);
    return 0;

}

int ImGui_impl_DrawList_PathArcToFast(lua_State *L)
{
    ImVec2 center = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    float radius = luaL_checknumber(L, 4);
    int a_min = luaL_checkinteger(L, 5);
    int a_max = luaL_checkinteger(L, 6);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathArcToFast(center, radius, a_min, a_max);
    return 0;

}

int ImGui_impl_DrawList_PathBezierCurveTo(lua_State *L)
{
    ImVec2 p2 = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 p3 = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    ImVec2 p4 = ImVec2(luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    int num_segments = luaL_optinteger(L, 8, NULL);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathBezierCurveTo(p2, p3, p4, num_segments);
    return 0;
}

int ImGui_impl_DrawList_PathRect(lua_State *L)
{
    ImVec2 rect_min = ImVec2(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    ImVec2 rect_max = ImVec2(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    float rounding = luaL_optnumber(L, 6, NULL);
    ImDrawCornerFlags rounding_corners = luaL_optinteger(L, 7, NULL);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PathRect(rect_min, rect_max, rounding, rounding_corners);
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int ImGui_my_test_key_table(lua_State *L)
{

    luaL_checktype(L, 2, LUA_TTABLE);
    size_t len = lua_objlen(L, 2);
    lua_pushvalue(L, 2);
    lua_pushnil(L);
    while (lua_next(L, -2))
    {
        lua_pushvalue(L, -2);
        int index = lua_tointeger(L, -1);
        const char *str = lua_tostring(L, -2);

        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    return 0;
}

int ImGui_my_test_n_table(lua_State *L)
{
    luaL_checktype(L, 2, LUA_TTABLE);
    size_t len = luaL_getn(L, 2);
    const char* items[len];
    lua_pushvalue(L, 2);
    for (int i = 0; i < len; i++)
    {
        lua_rawgeti(L, 2, i+1);

        const char* str = lua_tostring(L,-1);

        lua_getglobal(L, "print");
        lua_pushstring(L, str);
        lua_call(L, 1, 0);

        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return 0;
}

int loader(lua_State *L)
{
    Binder binder(L);
    const luaL_Reg functionList[] =
    {
        // Draw list
        {"drawListPushClipRect", ImGui_impl_DrawList_PushClipRect},
        {"drawListPushClipRectFullScreen", ImGui_impl_DrawList_PushClipRectFullScreen},
        {"drawListPopClipRect", ImGui_impl_DrawList_PopClipRect},
        {"drawListPushTextureID", ImGui_impl_DrawList_PushTextureID},
        {"drawListPopTextureID", ImGui_impl_DrawList_PopTextureID},
        {"drawListGetClipRectMin", ImGui_impl_DrawList_GetClipRectMin},
        {"drawListGetClipRectMax", ImGui_impl_DrawList_GetClipRectMax},
        {"drawListAddLine", ImGui_impl_DrawList_AddLine},
        {"drawListAddRect", ImGui_impl_DrawList_AddRect},
        {"drawListAddRectFilled", ImGui_impl_DrawList_AddRectFilled},
        {"drawListAddRectFilledMultiColor", ImGui_impl_DrawList_AddRectFilledMultiColor},
        {"drawListAddQuad", ImGui_impl_DrawList_AddQuad},
        {"drawListAddQuadFilled", ImGui_impl_DrawList_AddQuadFilled},
        {"drawListAddTriangle", ImGui_impl_DrawList_AddTriangle},
        {"drawListAddTriangleFilled", ImGui_impl_DrawList_AddTriangleFilled},
        {"drawListAddCircle", ImGui_impl_DrawList_AddCircle},
        {"drawListAddCircleFilled", ImGui_impl_DrawList_AddCircleFilled},
        {"drawListAddNgon", ImGui_impl_DrawList_AddNgon},
        {"drawListAddNgonFilled", ImGui_impl_DrawList_AddNgonFilled},
        {"drawListAddText", ImGui_impl_DrawList_AddText},
        //{"drawListAddFontText", ImGui_impl_DrawList_AddText},
        {"drawListAddPolyline", ImGui_impl_DrawList_AddPolyline},
        {"drawListAddConvexPolyFilled", ImGui_impl_DrawList_AddConvexPolyFilled},
        {"drawListAddBezierCurve", ImGui_impl_DrawList_AddBezierCurve},

        {"drawListAddImage", ImGui_impl_DrawList_AddImage},
        {"drawListAddImageQuad", ImGui_impl_DrawList_AddImageQuad},
        {"drawListAddImageRounded", ImGui_impl_DrawList_AddImageRounded},
        {"drawListPathClear", ImGui_impl_DrawList_PathClear},
        {"drawListPathLineTo", ImGui_impl_DrawList_PathLineTo},
        {"drawListPathLineToMergeDuplicate", ImGui_impl_DrawList_PathLineToMergeDuplicate},
        {"drawListPathFillConvex", ImGui_impl_DrawList_PathFillConvex},
        {"drawListPathStroke", ImGui_impl_DrawList_PathStroke},
        {"drawListPathArcTo", ImGui_impl_DrawList_PathArcTo},
        {"drawListPathArcToFast", ImGui_impl_DrawList_PathArcToFast},
        {"drawListPathBezierCurveTo", ImGui_impl_DrawList_PathBezierCurveTo},
        {"drawListPathRect", ImGui_impl_DrawList_PathRect},

        // Fonts
        {"addFonts", ImGui_addFonts},

        // Mouse inputs
        {"onMouseHover", ImGui_impl_MouseHover},
        {"onMouseMove", ImGui_impl_MouseMove},
        {"onMouseDown", ImGui_impl_MouseDown},
        {"onMouseUp", ImGui_impl_MouseUp},
        {"onMouseWheel", ImGui_impl_MouseWheel},

        // Touch inputs TODO

        // Keyboard
        {"onKeyUp", ImGui_impl_KeyUp},
        {"onKeyDown", ImGui_impl_KeyDown},
        {"onKeyChar", ImGui_impl_KeyChar},

        // Input flags
        {"wantCaptureMouse", ImGui_impl_WantCaptureMouse},
        {"wantCaptureKeyboard", ImGui_impl_WantCaptureKeyboard},
        {"wantTextInput", ImGui_impl_WantTextInput},

        // Colors
        {"colorConvertRGBtoHSV", ImGui_impl_ColorConvertRGBtoHSV},
        {"colorConvertHSVtoRGB", ImGui_impl_ColorConvertHSVtoRGB},

        // Styles
        {"setDarkStyle", ImGui_impl_StyleDark},
        {"setLightStyle", ImGui_impl_StyleLight},
        {"setClassicStyle", ImGui_impl_StyleClassic},

        {"ioGetConfigFlags", ImGui_impl_IO_GetConfigFlags},
        {"ioSetConfigFlags", ImGui_impl_IO_SetConfigFlags},
        {"ioGetBackendFlags", ImGui_impl_IO_GetBackendFlags},
        {"ioSetBackendFlags", ImGui_impl_IO_SetBackendFlags},
        {"ioGetIniSavingRate", ImGui_impl_IO_GetIniSavingRate},
        {"ioSetIniSavingRate", ImGui_impl_IO_SetIniSavingRate},
        {"ioGetIniFilename", ImGui_impl_IO_GetIniFilename},
        {"ioSetIniFilename", ImGui_impl_IO_SetIniFilename},
        {"ioGetLogFilename", ImGui_impl_IO_GetLogFilename},
        {"ioSetLogFilename", ImGui_impl_IO_SetLogFilename},
        {"ioGetMouseDoubleClickTime", ImGui_impl_IO_GetMouseDoubleClickTime},
        {"ioSetMouseDoubleClickTime", ImGui_impl_IO_SetMouseDoubleClickTime},
        {"ioGetMouseDragThreshold", ImGui_impl_IO_GetMouseDragThreshold},
        {"ioSetMouseDragThreshold", ImGui_impl_IO_SetMouseDragThreshold},
        {"ioGetMouseDoubleClickMaxDist", ImGui_impl_IO_GetMouseDoubleClickMaxDist},
        {"ioSetMouseDoubleClickMaxDist", ImGui_impl_IO_SetMouseDoubleClickMaxDist},
        {"ioGetKeyMapValue", ImGui_impl_IO_GetKeyMapValue},
        {"ioSetKeyMapValue", ImGui_impl_IO_SetKeyMapValue},
        {"ioGetKeyRepeatDelay", ImGui_impl_IO_GetKeyRepeatDelay},
        {"ioSetKeyRepeatDelay", ImGui_impl_IO_SetKeyRepeatDelay},
        {"ioGetKeyRepeatRate", ImGui_impl_IO_GetKeyRepeatRate},
        {"ioSetKeyRepeatRate", ImGui_impl_IO_SetKeyRepeatRate},
        {"ioGetFontGlobalScale", ImGui_impl_IO_GetFontGlobalScale},
        {"ioSetFontGlobalScale", ImGui_impl_IO_SetFontGlobalScale},
        {"ioGetFontAllowUserScaling", ImGui_impl_IO_GetFontAllowUserScaling},
        {"ioSetFontAllowUserScaling", ImGui_impl_IO_SetFontAllowUserScaling},
        {"ioGetDisplayFramebufferScale", ImGui_impl_IO_GetDisplayFramebufferScale},
        {"ioSetDisplayFramebufferScale", ImGui_impl_IO_SetDisplayFramebufferScale},
        {"ioGetConfigMacOSXBehaviors", ImGui_impl_IO_GetConfigMacOSXBehaviors},
        {"ioSetConfigMacOSXBehaviors", ImGui_impl_IO_SetConfigMacOSXBehaviors},
        {"ioGetConfigInputTextCursorBlink", ImGui_impl_IO_GetConfigInputTextCursorBlink},
        {"ioSetConfigInputTextCursorBlink", ImGui_impl_IO_SetConfigInputTextCursorBlink},
        {"ioGetConfigWindowsResizeFromEdges", ImGui_impl_IO_GetConfigWindowsResizeFromEdges},
        {"ioSetConfigWindowsResizeFromEdges", ImGui_impl_IO_SetConfigWindowsResizeFromEdges},
        {"ioGetConfigWindowsMoveFromTitleBarOnly", ImGui_impl_IO_GetConfigWindowsMoveFromTitleBarOnly},
        {"ioSetConfigWindowsMoveFromTitleBarOnly", ImGui_impl_IO_SetConfigWindowsMoveFromTitleBarOnly},
        {"ioGetConfigWindowsMemoryCompactTimer", ImGui_impl_IO_GetConfigWindowsMemoryCompactTimer},
        {"ioSetConfigWindowsMemoryCompactTimer", ImGui_impl_IO_SetConfigWindowsMemoryCompactTimer},

        {"ioGetBackendPlatformName", ImGui_impl_IO_GetBackendPlatformName},
        {"ioGetBackendRendererName", ImGui_impl_IO_GetBackendRendererName},
        {"ioIsMouseDown", ImGui_impl_IO_isMouseDown},
        {"ioGetMouseWheel", ImGui_impl_IO_GetMouseWheel},
        {"ioGetMouseWheelH", ImGui_impl_IO_GetMouseWheelH},
        {"ioIsKeyCtrl", ImGui_impl_IO_isKeyCtrl},
        {"ioIsKeyShift", ImGui_impl_IO_isKeyShift},
        {"ioIsKeyAlt", ImGui_impl_IO_isKeyAlt},
        {"ioIsKeySuper", ImGui_impl_IO_isKeySuper},
        {"ioGetKeysDown", ImGui_impl_IO_GetKeysDown},

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
        {"mouseDownSec", ImGui_impl_MouseDownSec},

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
        {"getStyleColor", ImGui_impl_GetStyleColor},
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

        // Windows
        {"beginWindow", ImGui_impl_Begin},
        {"endWindow", ImGui_impl_End},

        // Render
        {"newFrame", ImGui_impl_NewFrame},
        {"render", ImGui_impl_Render},
        {"endFrame", ImGui_impl_EndFrame},

        // Display size
        {"setDisplaySize", ImGui_impl_SetDisplaySize},
        {"setDisplayScale", ImGui_impl_SetDisplayScale},

        // Demos
        {"showUserGuide", ImGui_impl_ShowUserGuide},
        {"showDemoWindow", ImGui_impl_ShowDemoWindow},
        {"showAboutWindow", ImGui_impl_ShowAboutWindow},
        {"showStyleEditor", ImGui_impl_ShowStyleEditor},
        {"showFontSelector", ImGui_impl_ShowFontSelector},
        {"showMetricsWindow", ImGui_impl_ShowMetricsWindow},
        {"showStyleSelector", ImGui_impl_ShowStyleSelector},
        {NULL, NULL}
    };

    binder.createClass("ImGui", "Sprite", initImGui, destroyImGui, functionList);

    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    BindEnums(L);

    lua_getglobal(L, "ImGui");
    lua_pushstring(L, ImGui::GetVersion());
    lua_setfield(L, -2, "_VERSION");
    lua_pop(L, 1);

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
    ::L = L;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "ImGui");

    lua_pop(L, 2);

}

static void g_deinitializePlugin(lua_State *_UNUSED(L)) {  }

REGISTER_PLUGIN_NAMED("ImGui", "1.0.0", imgui)
