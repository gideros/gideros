#include "imgui_src/imgui.h"

typedef int ImGuiImageScaleMode;

enum ImGuiWindowFlags_Extended
{
	ImGuiWindowFlags_FullScreen = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
};

enum ImGuiInputTextFlags_Extended
{
	ImGuiInputTextFlags_NoBackground = 1 << 20
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

enum ImGuiImageScaleMode_
{
	ImGuiImageScaleMode_LetterBox,
	ImGuiImageScaleMode_FitWidth ,
	ImGuiImageScaleMode_FitHeight,
	ImGuiImageScaleMode_Stretch
};


namespace ImGui
{
	IMGUI_API void FitImage(ImVec2& min, ImVec2& max, const ImVec2& rect_size,
							const ImVec2& texture_size, const ImVec2& anchor,
							ImGuiImageScaleMode fit_mode, bool keep_size);
	IMGUI_API void ScaledImage(const char* str_id, const ImVec2& texture_size, ImTextureID texture_id, const ImVec2& size,
							   ImGuiImageScaleMode fit_mode = ImGuiImageScaleMode_LetterBox, bool keep_size = false, const ImVec2& anchor = ImVec2(0.5f, 0.5f),
							   const ImVec4& tint_col = ImVec4(1.0f,1.0f,1.0f,1.0f), const ImVec4& border_col = ImVec4(0.0f,0.0f,0.0f,0.0f), const ImVec4& bg_col = ImVec4(0.0f,0.0f,0.0f,0.0f),
							   const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f,1.0f));
	IMGUI_API bool ScaledImageButton(const char* str_id, const ImVec2& texture_size, ImTextureID texture_id, const ImVec2& size,
									 ImGuiImageScaleMode fit_mode = ImGuiImageScaleMode_LetterBox, bool keep_size = false, ImGuiButtonFlags flags = 0, const ImVec2& anchor = ImVec2(0.5f, 0.5f),
									 const ImVec2& clipOffset = ImVec2(0.0f, 0.0f),
									 const ImVec4& tint_col = ImVec4(1.0f,1.0f,1.0f,1.0f), const ImVec4& border_col = ImVec4(0.0f,0.0f,0.0f,0.0f), const ImVec4& bg_col = ImVec4(0.0f,0.0f,0.0f,0.0f),
									 const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f,1.0f));
	IMGUI_API bool ScaledImageButtonWithText(const ImVec2& texture_size, ImTextureID texture_id, const char* label, const ImVec2& image_size,
											 const ImVec2& button_size = ImVec2(0.0f, 0.0f), ImGuiButtonFlags flags = 0,
											 ImGuiImageScaleMode fit_mode = ImGuiImageScaleMode_LetterBox, bool keep_size = false, const ImVec2& anchor = ImVec2(0.5f, 0.5f), ImGuiDir image_side = ImGuiDir_Left,
											 const ImVec2& clipOffset = ImVec2(0.0f, 0.0f),
											 const ImVec4& tint_col = ImVec4(1.0f,1.0f,1.0f,1.0f), const ImVec4& border_col = ImVec4(0.0f,0.0f,0.0f,0.0f), const ImVec4& bg_col = ImVec4(0.0f,0.0f,0.0f,0.0f),
											 const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f,1.0f));

	IMGUI_API bool FilledSliderScalar(const char* label, bool mirror, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
	IMGUI_API bool FilledSliderScalarN(const char* label, bool mirror, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
	IMGUI_API bool FilledSliderFloat(const char* label, bool mirror, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);	 // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display. Use power!=1.0 for power curve sliders
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
}
