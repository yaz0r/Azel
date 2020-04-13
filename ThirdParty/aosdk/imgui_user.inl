/*
 * Audio Overload SDK
 *
 * Custom ImGui extensions
 *
 * Author: Nmlgc
 */

bool ImGui::DragIntClamp(const char* label, int* v, float v_speed, int v_min, int v_max, const char* display_format)
{
	bool ret = ImGui::DragInt(label, v, v_speed, v_min, v_max, display_format);
	if(ret && v_min < v_max) {
		*v = ImClamp(*v, v_min, v_max);
	}
	return ret;
}
