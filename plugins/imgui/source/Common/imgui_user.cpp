#ifndef IMGUI_DISABLE

#ifndef DPRINTF
#include "debugapi.h"
#define DPRINTF( format, ...) do { char buffer[1024]; sprintf(buffer, format, __VA_ARGS__); OutputDebugStringA( buffer ); } while(0)
#endif

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

    void FitImage(ImRect& bb, const ImVec2& rect_size,
                   const ImVec2& texture_size, const ImVec2& anchor,
                   ImGuiImageScaleMode fit_mode, bool keep_size)
    {
        ImVec2 scaled_texture_size;
        switch (fit_mode) {
        case ImGuiImageScaleMode_FitWidth:
            scaled_texture_size = texture_size * rect_size.x / texture_size.x;
            break;
        case ImGuiImageScaleMode_FitHeight:
            scaled_texture_size = texture_size * rect_size.y / texture_size.y;
            break;
        case ImGuiImageScaleMode_Stretch:
            scaled_texture_size = rect_size;
            break;
        default:
            scaled_texture_size = texture_size * ImMin(rect_size.x / texture_size.x, rect_size.y / texture_size.y);
            break;
        }

        if (keep_size)
        {
            bb.Min += anchor * (rect_size - texture_size);
            bb.Max = bb.Min + texture_size;
        }
        else
        {
            ImVec2 anchor_offset = anchor * (rect_size - scaled_texture_size);
            bb.Min += anchor_offset;
            bb.Max = bb.Min + scaled_texture_size;
        }
    }

    void ScaledImage(const ImVec2& texture_size, ImTextureID texture_id, const ImVec2& size,
                     ImGuiImageScaleMode fit_mode, bool keep_size, const ImVec2& anchor,
                     const ImVec4& tint_col, const ImVec4& border_col, const ImVec4& bg_col,
                     const ImVec2& uv0, const ImVec2& uv1)
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
            window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col));

        if (border_col.w > 0.0f)
        {
            ImVec2 backup_min = bb.Min;
            ImVec2 backup_max = bb.Max;
            window->DrawList->PushClipRect(bb.Min, bb.Max);
            FitImage(bb, size, texture_size, anchor, fit_mode, keep_size);
            window->DrawList->AddImage(texture_id, bb.Min + ImVec2(1, 1), bb.Max + ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
            window->DrawList->AddRect(backup_min, backup_max, GetColorU32(border_col));
            window->DrawList->PopClipRect();
        }
        else
        {
            window->DrawList->PushClipRect(bb.Min, bb.Max);
            FitImage(bb, size, texture_size, anchor, fit_mode, keep_size);
            window->DrawList->AddImage(texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
            window->DrawList->PopClipRect();
        }
    }

    bool ScaledImageButtonEx(const ImVec2& texture_size, ImTextureID texture_id, ImGuiID id, const ImVec2& size,
                             ImGuiImageScaleMode fit_mode, bool keep_size, ImGuiButtonFlags flags, const ImVec2& anchor,
                             const ImVec4& tint_col, const ImVec4& border_col, const ImVec4& bg_col,
                             const ImVec2& uv0, const ImVec2& uv1)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        const ImVec2 padding = g.Style.FramePadding;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
        ItemSize(bb);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderNavHighlight(bb, id);
        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
        if (border_col.w > 0.0f)
            window->DrawList->AddRect(bb.Min + padding, bb.Max - padding, GetColorU32(border_col));
        bb.Min += padding;
        bb.Max -= padding;
        window->DrawList->PushClipRect(bb.Min, bb.Max);
        FitImage(bb, size, texture_size, anchor, fit_mode, keep_size);
        window->DrawList->AddImage(texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        window->DrawList->PopClipRect();

        return pressed;
    }

    bool ScaledImageButton(const ImVec2& texture_size, ImTextureID texture_id, const ImVec2& size,
                           ImGuiImageScaleMode fit_mode, bool keep_size, ImGuiButtonFlags flags, const ImVec2& anchor,
                           const ImVec4& tint_col, const ImVec4& border_col, const ImVec4& bg_col,
                           const ImVec2& uv0, const ImVec2& uv1)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return false;

        PushID((void*)(intptr_t)texture_id);
        const ImGuiID id = window->GetID("#image");
        PopID();

        return ScaledImageButtonEx(texture_size, texture_id, id, size, fit_mode, keep_size, flags, anchor, tint_col, border_col, bg_col, uv0, uv1);
    }

    bool ScaledImageButtonWithText(const ImVec2& texture_size, ImTextureID texture_id, const char* label, const ImVec2& image_size,
                                   const ImVec2& button_size, ImGuiButtonFlags flags,
                                   ImGuiImageScaleMode fit_mode, bool keep_size, const ImVec2& anchor, ImGuiDir image_side,
                                   const ImVec4& tint_col, const ImVec4& border_col, const ImVec4& bg_col,
                                   const ImVec2& uv0, const ImVec2& uv1)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImVec2 padding = style.FramePadding;

        ImVec2 pos = window->DC.CursorPos;
        if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && padding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
            pos.y += window->DC.CurrLineTextBaseOffset - padding.y;

        ImVec2 size = CalcItemSize(button_size, label_size.x + padding.x * 2.0f, label_size.y + padding.y * 2.0f);

        ImRect bb(pos, pos + size);
        ItemSize(size, padding.y);
        if (!ItemAdd(bb, id))
            return false;

        if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
            flags |= ImGuiButtonFlags_Repeat;
        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderNavHighlight(bb, id);
        RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);


        if (g.LogEnabled)
            LogSetNextTextDecoration("[", "]");

        float image_width = ImMax(image_size.x, padding.x * 2.0f + 1.0f);
        float image_height = ImMax(image_size.y, padding.y * 2.0f + 1.0f);
        ImRect ibb;

        switch (image_side) {
        case ImGuiDir_Right:
            {
                ImVec2 backup_max = bb.Max;
                bb.Max.x = ImClamp(bb.Max.x - image_width, bb.Min.x, bb.Max.x);
                ibb = ImRect(ImVec2(bb.Max.x + padding.x, bb.Min.y + padding.y), backup_max - padding);
            }
            break;
        case ImGuiDir_Up:
            {
                ImVec2 backup_min = bb.Min;
                bb.Min.y = ImClamp(bb.Min.y + image_height, bb.Min.y, bb.Max.y);
                ibb = ImRect(backup_min + padding, ImVec2(bb.Max.x - padding.x, bb.Min.y - padding.y));
            }
            break;
        case ImGuiDir_Down:
            {
                ImVec2 backup_max = bb.Max;
                bb.Max.y = ImClamp(bb.Max.y - image_height, bb.Min.y, bb.Max.y);
                ibb = ImRect(ImVec2(bb.Min.x + padding.x, bb.Max.y + padding.y), backup_max - padding);
            }
            break;
        // Left align by default
        default:
            {
                ImVec2 backup_min = bb.Min;
                bb.Min.x = ImClamp(bb.Min.x + image_width, bb.Min.x, bb.Max.x);
                ibb = ImRect(backup_min + padding, ImVec2(bb.Min.x - padding.x, bb.Max.y - padding.y));
            }
            break;
        }
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(ibb.Min, ibb.Max, GetColorU32(bg_col));
        ImRect backup_ibb = ibb;
        window->DrawList->PushClipRect(ibb.Min, ibb.Max, true);
        FitImage(ibb, ibb.Max - ibb.Min, texture_size, anchor, fit_mode, keep_size);
        window->DrawList->AddImage(texture_id, ibb.Min, ibb.Max, uv0, uv1, GetColorU32(tint_col));
        window->DrawList->PopClipRect();

        if (border_col.w > 0.0f)
            window->DrawList->AddRect(backup_ibb.Min, backup_ibb.Max, GetColorU32(border_col));
        RenderTextClipped(bb.Min + padding, bb.Max - padding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

        return pressed;
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

}

#endif
