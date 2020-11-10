#include "imgui_src/imgui.h"

enum ImGuiWindowFlags_Extended
{
    ImGuiWindowFlags_FullScreen = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
};

enum ImGuiInputTextFlags_Extended
{
    ImGuiInputTextFlags_NoBackground = 1 << 19
};

enum ImGuiGlyphRanges
{
    ImGuiGlyphRanges_Default,
    ImGuiGlyphRanges_Korean,
    ImGuiGlyphRanges_ChineseFull,
    ImGuiGlyphRanges_ChineseSimplifiedCommon,
    ImGuiGlyphRanges_Japanese,
    ImGuiGlyphRanges_Cyrillic,
    ImGuiGlyphRanges_Thai,
    ImGuiGlyphRanges_Vietnamese,
};

namespace ImGui
{
    IMGUI_API bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));

    IMGUI_API bool FilledSliderScalar(const char* label, bool mirror, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderScalarN(const char* label, bool mirror, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderFloat(const char* label, bool mirror, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display. Use power!=1.0 for power curve sliders
    IMGUI_API bool FilledSliderFloat2(const char* label, bool mirror, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderFloat3(const char* label, bool mirror, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderFloat4(const char* label, bool mirror, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderAngle(const char* label, bool mirror, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderInt(const char* label, bool mirror, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderInt2(const char* label, bool mirror, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderInt3(const char* label, bool mirror, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool FilledSliderInt4(const char* label, bool mirror, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool VFilledSliderFloat(const char* label, bool mirror, const ImVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    IMGUI_API bool VFilledSliderInt(const char* label, bool mirror, const ImVec2& size, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    IMGUI_API bool VFilledSliderScalar(const char* label, bool mirror, const ImVec2& size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);

    IMGUI_API void ImageFilled(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& bg_col = ImVec4(1,1,1,0), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
}
