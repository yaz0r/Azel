/*
 * Audio Overload SDK
 *
 * Debug GUI
 *
 * Author: Nmlgc
 */

#include <imgui.h>
#include "imgui/examples/opengl_example/imgui_impl_glfw.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "ao.h"
#include "corlett.h"
#include "debug.h"

GLFWwindow *window;
bool show_test_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImColor(114, 144, 154);

/// Widgets
/// -------
void debug_memory(const char *view_id, DebugMemoryState *state, uint8 *mem_buf, int32 mem_size)
{
	const int ROWS_MAX = 64;
	IM_ASSERT(view_id);
	IM_ASSERT(state);

	auto& io = ImGui::GetIO();
	auto& style = ImGui::GetStyle();
	int byte_width = (int)ImGui::CalcTextSize("00").x;
	auto& x_spacing = style.ItemSpacing.x;
	auto item_spacing_half = ImVec2(style.ItemSpacing.x / 2, style.ItemSpacing.y);

	enum ByteColors : uint8 {
		Normal,
		Control,
		MAX
	};

	const ImVec4 BYTE_COLORS[MAX] {
		style.Colors[ImGuiCol_Text],
		style.Colors[ImGuiCol_TextDisabled],
	};

	ImGui::PushID(state);
	ImGui::PushItemWidth(ROWS_MAX * 2);
	ImGui::DragIntClamp("##rows", &state->rows, 0.2f, 1, ROWS_MAX, "%.0f bytes per line");
	if(state->offset > state->rows - 1) {
		state->offset = state->rows - 1;
	}
	if(state->rows > 1) {
		ImGui::SameLine();
		ImGui::DragIntClamp("##offset", &state->offset, 0.2f, 0, state->rows - 1, "Offset: %.0f");
	}
	ImGui::PopItemWidth();

	ImGui::BeginChild(view_id);
	auto mem_lines = (mem_size + state->offset + (state->rows - 1)) / state->rows;
	ImGuiListClipper clipper(mem_lines, ImGui::GetTextLineHeightWithSpacing());
	int32 byte = -state->offset + (clipper.DisplayStart * state->rows);
	auto line = clipper.DisplayStart;

	float vline_x[2];
	while(line < clipper.DisplayEnd && byte < mem_size) {
		ByteColors ascii_colors[ROWS_MAX + 1];
		char ascii[ROWS_MAX + 1];

		ImGui::Text("%08x", byte < 0 ? 0 : byte);
		ImGui::SameLine();

		vline_x[0] = ImGui::GetCursorScreenPos().x;
			
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_spacing);
		for(int column = 0; column < state->rows; byte++, column++) {
			ascii_colors[column] = Normal;
			if(byte < 0 || byte >= mem_size) {
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + byte_width + x_spacing);
				ascii[column] = ' ';
			} else {
				auto b = mem_buf[byte];
				if(b == 0) {
					ascii_colors[column] = Control;
					ascii[column] = '.';
				} else if(b < ' ') {
					ascii_colors[column] = Control;
					ascii[column] = '_';
				} else if(b > 127) {
					ascii_colors[column] = Control;
					ascii[column] = '?';
				} else {
					ascii[column] = b;
				}
				ImGui::Text("%02x", b);
				ImGui::SameLine();
			}
		}
		vline_x[1] = ImGui::GetCursorScreenPos().x;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_spacing);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing_half);
		for(int column = 0; column < state->rows; column++) {
			auto color = BYTE_COLORS[ascii_colors[column]];
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(&ascii[column], &ascii[column + 1]);
			if(column != state->rows - 1) {
				ImGui::SameLine();
			}
			ImGui::PopStyleColor();
		}
		ImGui::PopStyleVar();
		line++;
	}
	clipper.End();
	auto vline_y_end = ImGui::GetCursorScreenPos().y;
	for(int i = 0; i < countof(vline_x); i++) {
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(vline_x[i], 0.0f), ImVec2(vline_x[i], vline_y_end),
			ImColor(style.Colors[ImGuiCol_Column])
		);
	}
	ImGui::EndChild();
	ImGui::PopID();
}
/// -------

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

ao_bool debug_start(void)
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if(!glfwInit()) {
		return false;
	}
 
	window = glfwCreateWindow(1280, 720, "ImGui OpenGL2 example", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Setup ImGui binding
	ImGui_ImplGlfw_Init(window, true);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;

	return true;
}

ao_bool debug_frame(debug_hw_t *hw)
{
	auto& io = ImGui::GetIO();
	auto& style = ImGui::GetStyle();

	glfwPollEvents();
	ImGui_ImplGlfw_NewFrame();

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if(ImGui::Button("Test Window")) {
			show_test_window ^= 1;
		}
		if(ImGui::Button("Another Window")) {
			show_another_window ^= 1;
		}
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if(show_another_window) {
		ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	if(hw) {
		hw();
	}

	// Status bar
	auto status_bar_flags =
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoFocusOnAppearing;
	auto status_bar_pos = ImVec2(0.0f, io.DisplaySize.y - ImGui::GetWindowContentRegionMin().y);
	auto progress = (double)corlett_sample_count() / (double)corlett_sample_total();
	auto progress_x = progress * io.DisplaySize.x;

	// ... Well, creating a second window just for the progress bar seems
	// to be the cleanest way to work around the window padding. -.-
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowPos(ImVec2(0, status_bar_pos.y));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0.0f));
		ImGui::Begin("Progress Bar", NULL, status_bar_flags);
			ImGui::GetWindowDrawList()->AddRectFilled(
				status_bar_pos,
				ImVec2(progress_x, status_bar_pos.y + ImGui::GetWindowSize().y),
				ImColor(style.Colors[ImGuiCol_TitleBg])
			);
		ImGui::End();
	ImGui::PopStyleVar();

	char fps[32];
	sprintf(fps, "%.1f FPS", io.Framerate);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
	ImGui::SetNextWindowPos(status_bar_pos);
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0.0f));
	ImGui::Begin("Status Bar", NULL, status_bar_flags);
		ImGui::Columns(2);
			ImGui::SetColumnOffset(1, ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(fps).x);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
				ImGui::TextUnformatted(fps);
			ImGui::PopItemWidth();
		ImGui::Columns(1);
	ImGui::End();
	ImGui::PopStyleColor();

	// Rendering
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
	glfwSwapBuffers(window);
	
	return glfwWindowShouldClose(window);
}

ao_bool debug_stop(void)
{
	// Cleanup
	ImGui_ImplGlfw_Shutdown();
	glfwTerminate();

	return true;
}
