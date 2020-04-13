/*
 * Audio Overload SDK
 *
 * Custom ImGui extensions
 *
 * Author: Nmlgc
 */

namespace ImGui
{
	// DragInt() that makes sure to also clamp the value from the input box.
	IMGUI_API bool DragIntClamp(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* display_format = "%.0f");
}
