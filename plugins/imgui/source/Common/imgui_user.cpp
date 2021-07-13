#ifndef IMGUI_DISABLE

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui_src/imgui.h"
#include "imgui_src/imgui_internal.h"
//#include "imgui_src/imgui_widgets.cpp"
#include <math.h>

//=============== CODE BORROWED FROM IMGUI_WIDGETS.CPP
static const ImGuiDataTypeInfo GDataTypeInfo[] =
{
    { sizeof(char),             "S8",   "%d",   "%d"    },  // ImGuiDataType_S8
    { sizeof(unsigned char),    "U8",   "%u",   "%u"    },
    { sizeof(short),            "S16",  "%d",   "%d"    },  // ImGuiDataType_S16
    { sizeof(unsigned short),   "U16",  "%u",   "%u"    },
    { sizeof(int),              "S32",  "%d",   "%d"    },  // ImGuiDataType_S32
    { sizeof(unsigned int),     "U32",  "%u",   "%u"    },
#ifdef _MSC_VER
    { sizeof(ImS64),            "S64",  "%I64d","%I64d" },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%I64u","%I64u" },
#else
    { sizeof(ImS64),            "S64",  "%lld", "%lld"  },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%llu", "%llu"  },
#endif
    { sizeof(float),            "float", "%f",  "%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
    { sizeof(double),           "double","%f",  "%lf"   },  // ImGuiDataType_Double
};

// FIXME-LEGACY: Prior to 1.61 our DragInt() function internally used floats and because of this the compile-time default value for format was "%.0f".
// Even though we changed the compile-time default, we expect users to have carried %f around, which would break the display of DragInt() calls.
// To honor backward compatibility we are rewriting the format string, unless IMGUI_DISABLE_OBSOLETE_FUNCTIONS is enabled. What could possibly go wrong?!
static const char* PatchFormatStringFloatToInt(const char* fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
        return "%d";
    const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
    if (fmt_end > fmt_start && fmt_end[-1] == 'f')
    {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        if (fmt_start == fmt && fmt_end[0] == 0)
            return "%d";
        ImGuiContext& g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
        return g.TempBuffer;
#else
        IM_ASSERT(0 && "DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
    }
    return fmt;
}

//END OF BORROWED CODE


namespace ImGui
{
    ImVec2 GetItemSize(ImVec2 size, ImVec2 min, float defw, float defh)
    {
        ImVec2 out_size = CalcItemSize(size, min.x + defw, min.y + defh);
        size.x = ImMax(size.x, min.x);
        size.y = ImMax(size.y, min.y);
        return out_size;
    }

    void FitImage(ImVec2& Min, ImVec2& Max, const ImVec2& rect_size, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& anchor, ImVec2 padding)
    {
        ImVec2 scaled_texture_size = texture_size * ImMin((image_size.x - padding.x * 2.0f) / texture_size.x, (image_size.y - padding.y * 2.0f) / texture_size.y);
        ImVec2 anchor_offset = anchor * (rect_size - scaled_texture_size);
        Min += anchor_offset;
        Min.x -= padding.x * anchor.x * 2.0f;
        Min.y -= padding.y;
        Max = Min + scaled_texture_size;
    }

    void ScaledImage(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& button_size, const ImVec2& anchor, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col, const float frame_rounding)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        ImVec2 size = GetItemSize(button_size, image_size, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);

        ImRect bb(window->DC.CursorPos + style.FramePadding, window->DC.CursorPos + size - style.FramePadding);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2, 2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;


        if (border_col.w > 0.0f)
        {
            ImVec2 unit(1,1);
            window->DrawList->PushClipRect(bb.Min, bb.Max, true);
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), frame_rounding);
            //window->DrawList->PopClipRect();
            // @MultiPain +
            FitImage(bb.Min, bb.Max, size, image_size, texture_size, anchor, style.FramePadding);
            // @MultiPain -
            //window->DrawList->PushClipRect(bb.Min, bb.Max, true);
            window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
            window->DrawList->PopClipRect();
        }
        else
        {
            // @MultiPain +
            FitImage(bb.Min, bb.Max, size, image_size, texture_size, anchor, style.FramePadding);
            // @MultiPain -
            window->DrawList->PushClipRect(bb.Min, bb.Max, true);
            window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
            window->DrawList->PopClipRect();
        }
    }

    bool ScaledImageButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& button_size, const ImVec2& anchor, const ImVec2& uv0, const ImVec2& uv1, ImGuiButtonFlags flags, const ImVec4& tint_col, const ImVec4& bg_col)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 size = GetItemSize(button_size, image_size, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);
        ImVec2 padding = g.Style.FramePadding;

        ImRect bb(window->DC.CursorPos + padding, window->DC.CursorPos + size - padding);
        ItemSize(bb);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        RenderNavHighlight(bb, id);

        if (bg_col.w > 0.0f)
        {
            RenderFrame(bb.Min, bb.Max, GetColorU32(bg_col), true, g.Style.FrameRounding);
        }
        else
        {
            const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);
        }

        // @MultiPain +
        FitImage(bb.Min, bb.Max, size, image_size, texture_size, anchor, style.FramePadding);
        // @MultiPain -
        window->DrawList->PushClipRect(bb.Min, bb.Max, true);
        window->DrawList->AddImage(texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        window->DrawList->PopClipRect();

        return pressed;
    }

    bool ScaledImageButton(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& button_size, const ImVec4& tint_col, const ImVec4& bg_col, ImGuiButtonFlags flags, const ImVec2& anchor, const ImVec2& uv0, const ImVec2& uv1)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return false;

        const ImGuiID id = window->GetID(user_texture_id);

        return ScaledImageButtonEx(id, user_texture_id, image_size, texture_size, button_size, anchor, uv0, uv1, flags, tint_col, bg_col);
    }

    bool FilledSliderScalar(const char* label, bool mirror, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = CalcItemWidth();

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb))
            return false;

        // Default format string when passing NULL
        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
            format = PatchFormatStringFloatToInt(format);

        // Tabbing or CTRL-clicking on Slider turns it into an input box
        const bool hovered = ItemHoverable(frame_bb, id);
        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
        if (!temp_input_is_active)
        {
            bool focused = (GetItemStatusFlags() & ImGuiItemStatusFlags_Focused) != 0;
            const bool focus_requested = temp_input_allowed && focused;
            const bool clicked = (hovered && g.IO.MouseClicked[0]);
            if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
                if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
                {
                    temp_input_is_active = true;
                }
            }
        }

        if (temp_input_is_active)
        {
            // Only clamp CTRL+Click input when ImGuiSliderFlags_ClampInput is set
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
        }

        // Draw frame
        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

        // Slider behavior
        ImRect grab_bb;
        const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
        if (value_changed)
            MarkItemEdited(id);

        // Render grab
        if (grab_bb.Max.x > grab_bb.Min.x)
        {
            if (mirror)
            {
                window->DrawList->AddRectFilled(grab_bb.Min, ImVec2(frame_bb.Max.x - 2.0f, grab_bb.Max.y), GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
            }
            else
            {
                window->DrawList->AddRectFilled(ImVec2(frame_bb.Min.x + 2.0f, grab_bb.Min.y), grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
            }
        }

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
        RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

        if (label_size.x > 0.0f)
            RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
        return value_changed;
    }

    bool FilledSliderScalarN(const char* label, bool mirror, ImGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, ImGuiSliderFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        bool value_changed = false;
        BeginGroup();
        PushID(label);
        PushMultiItemsWidths(components, CalcItemWidth());
        size_t type_size = GDataTypeInfo[data_type].Size;
        for (int i = 0; i < components; i++)
        {
            PushID(i);
            if (i > 0)
                SameLine(0, g.Style.ItemInnerSpacing.x);
            value_changed |= FilledSliderScalar("", mirror, data_type, v, v_min, v_max, format, flags);
            PopID();
            PopItemWidth();
            v = (void*)((char*)v + type_size);
        }
        PopID();

        const char* label_end = FindRenderedTextEnd(label);
        if (label != label_end)
        {
            SameLine(0, g.Style.ItemInnerSpacing.x);
            TextEx(label, label_end);
        }

        EndGroup();
        return value_changed;
    }

    bool FilledSliderFloat(const char* label, bool mirror, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalar(label, mirror, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }

    bool FilledSliderFloat2(const char* label, bool mirror, float v[2], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_Float, v, 2, &v_min, &v_max, format, flags);
    }

    bool FilledSliderFloat3(const char* label, bool mirror, float v[3], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_Float, v, 3, &v_min, &v_max, format, flags);
    }

    bool FilledSliderFloat4(const char* label, bool mirror, float v[4], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_Float, v, 4, &v_min, &v_max, format, flags);
    }

    bool FilledSliderAngle(const char* label, bool mirror, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, ImGuiSliderFlags flags)
    {
        if (format == NULL)
            format = "%.0f deg";
        float v_deg = (*v_rad) * 360.0f / (2 * IM_PI);
        bool value_changed = FilledSliderFloat(label, mirror, &v_deg, v_degrees_min, v_degrees_max, format, flags);
        *v_rad = v_deg * (2 * IM_PI) / 360.0f;
        return value_changed;
    }

    bool FilledSliderInt(const char* label, bool mirror, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalar(label, mirror, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
    }

    bool FilledSliderInt2(const char* label, bool mirror, int v[2], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_S32, v, 2, &v_min, &v_max, format, flags);
    }

    bool FilledSliderInt3(const char* label, bool mirror, int v[3], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_S32, v, 3, &v_min, &v_max, format, flags);
    }

    bool FilledSliderInt4(const char* label, bool mirror, int v[4], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return FilledSliderScalarN(label, mirror, ImGuiDataType_S32, v, 4, &v_min, &v_max, format, flags);
    }

    bool VFilledSliderScalar(const char* label, bool mirror, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
        const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(frame_bb, id))
            return false;

        // Default format string when passing NULL
        if (format == NULL)
            format = DataTypeGetInfo(data_type)->PrintFmt;
        else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
            format = PatchFormatStringFloatToInt(format);

        const bool hovered = ItemHoverable(frame_bb, id);
        if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id)
        {
            SetActiveID(id, window);
            SetFocusID(id, window);
            FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
        }

        // Draw frame
        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

        // Slider behavior
        ImRect grab_bb;
        const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags | ImGuiSliderFlags_Vertical, &grab_bb);
        if (value_changed)
            MarkItemEdited(id);

        // Render grab
        if (grab_bb.Max.y > grab_bb.Min.y)
        {
            if (mirror)
            {
                window->DrawList->AddRectFilled(grab_bb.Min, ImVec2(frame_bb.Max.x - 2.0f, grab_bb.Max.y), GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
            }
            else
            {
                window->DrawList->AddRectFilled(ImVec2(frame_bb.Min.x + 2.0f, grab_bb.Min.y), grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
            }
        }

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        // For the vertical slider we allow centered text to overlap the frame padding
        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.0f));
        if (label_size.x > 0.0f)
            RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

        return value_changed;
    }

    bool VFilledSliderFloat(const char* label, bool mirror, const ImVec2& size, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return VFilledSliderScalar(label, mirror, size, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    }

    bool VFilledSliderInt(const char* label, bool mirror, const ImVec2& size, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
    {
        return VFilledSliderScalar(label, mirror, size, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
    }


    bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImVec2 size = imageSize;
        if (size.x<=0 && size.y<=0) {size.x=size.y=ImGui::GetTextLineHeightWithSpacing();}
        else {
            if (size.x<=0)          size.x=size.y;
            else if (size.y<=0)     size.y=size.x;
            size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;
        }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        const ImGuiID id = window->GetID(label);
        const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
        const bool hasText = textSize.x>0;

        const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
        const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
        const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
        ImVec2 start(0,0);
        start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
        const ImRect image_bb(start, start + size);
        start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
        ItemSize(bb);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered=false, held=false;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        // Render
        const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

        window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

        if (textSize.x>0) ImGui::RenderText(start,label);
        return pressed;
    }

    bool ScaledImageButtonWithText(ImTextureID texId, const char* label, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& button_size, const ImVec4& tint_col, const ImVec4& bg_col, ImGuiDir image_side, ImGuiButtonFlags flags, const ImVec2& uv0, const ImVec2& uv1)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        ImVec2 padding = style.FramePadding;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = CalcTextSize(label, NULL, true);

        ImVec2 pos = window->DC.CursorPos;
        if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && padding.y < window->DC.CurrLineTextBaseOffset)
            pos.y += window->DC.CurrLineTextBaseOffset - padding.y;
        ImVec2 size = CalcItemSize(button_size, label_size.x + padding.x * 2.0f, label_size.y + padding.y * 2.0f);

        ImRect bb(pos, pos + size);
        ItemSize(size, padding.y);
        if (!ItemAdd(bb, id))
            return false;

        ImRect image_bb(pos + padding, pos + image_size - padding);
        FitImage(image_bb.Min, image_bb.Max, size, image_size, texture_size, ImVec2(image_side == ImGuiDir_Left ? 0.0f : 1.0f, 0.5f), padding);

        if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
            flags |= ImGuiButtonFlags_Repeat;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        RenderNavHighlight(bb, id);

        if (bg_col.w > 0.0f)
        {
           RenderFrame(bb.Min, bb.Max, GetColorU32(bg_col), true, style.FrameRounding);
        }
        else
        {
            const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
        }

        window->DrawList->PushClipRect(bb.Min, bb.Max, true);
        window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));
        window->DrawList->PopClipRect();

        if (image_side == ImGuiDir_Right)
        {
            bb.Max.x = image_bb.Min.x;
            RenderTextClipped(bb.Min + padding, bb.Max - padding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
        }
        else
        {
            bb.Min.x = image_bb.Max.x;
            RenderTextClipped(bb.Min + padding, bb.Max - padding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
        }

        return pressed;
    }

    void ImageFilled(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col, const ImVec4& border_col)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2, 2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;

        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col), 0.0f);

        if (border_col.w > 0.0f)
        {
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
            window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
        }
        else
        {
            window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        }
    }

    void ScaledImageFilled(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& texture_size, const ImVec2& button_size, const ImVec2& anchor, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col, const ImVec4& border_col, const float frame_rounding)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        ImVec2 size = GetItemSize(button_size, image_size, style.FramePadding.x * 2.0f, style.FramePadding.y * 2.0f);

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2, 2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;

        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col), frame_rounding);

        if (border_col.w > 0.0f)
        {
            ImVec2 unit(1,1);
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), frame_rounding);
            // @MultiPain +
            FitImage(bb.Min, bb.Max, size + unit * 2, image_size, texture_size, anchor, style.FramePadding);
            // @MultiPain -
            window->DrawList->AddImage(user_texture_id, bb.Min - unit, bb.Max + unit, uv0, uv1, GetColorU32(tint_col));
        }
        else
        {
            // @MultiPain +
            FitImage(bb.Min, bb.Max, size, image_size, texture_size, anchor, style.FramePadding);
            // @MultiPain -
            window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        }
    }

}

#endif
