/*
 * Audio Overload SDK
 *
 * Debug GUI for Dreamcast
 *
 * Author: Nmlgc
 */

#include <imgui.h>
#include "ao.h"
#include "debug.h"
#include "eng_protos.h"
#include "dc_hw.h"

extern "C" void dc_debug(void)
{
	static DebugMemoryState memory_ctx_sat;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Dreamcast", NULL, ImGuiWindowFlags_NoMove);

	if(ImGui::CollapsingHeader("Memory", NULL, true, true)) {
		static DebugMemoryState memory_state;
		debug_memory("DC RAM", &memory_state, dc_ram, sizeof(dc_ram));
	}
	ImGui::End();
}
