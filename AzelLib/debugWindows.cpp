#include "PDS.h"
#include "debugWindows.h"
#include <imgui.h>

sDebugWindows gDebugWindows;

void drawDebugMenu()
{
    if (isShipping())
        return;

    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Objects", nullptr, &gDebugWindows.objects);
        ImGui::MenuItem("Sound Debugger", nullptr, &gDebugWindows.soundDebugger);
        ImGui::MenuItem("Collisions", nullptr, &gDebugWindows.collisions);
        ImGui::MenuItem("Input State", nullptr, &gDebugWindows.inputState);
        ImGui::MenuItem("VDP", nullptr, &gDebugWindows.vdp);
        ImGui::MenuItem("Config", nullptr, &gDebugWindows.config);
        ImGui::MenuItem("Final Composition", nullptr, &gDebugWindows.finalComposition);
        ImGui::MenuItem("Inventory", nullptr, &gDebugWindows.inventory);
        ImGui::MenuItem("Tasks", nullptr, &gDebugWindows.tasks);
        ImGui::MenuItem("Town", nullptr, &gDebugWindows.town);
        ImGui::MenuItem("Field", nullptr, &gDebugWindows.field);
        ImGui::MenuItem("Camera", nullptr, &gDebugWindows.camera);
        ImGui::MenuItem("Arachnoth", nullptr, &gDebugWindows.arachnoth);
        ImGui::Separator();
        ImGui::MenuItem("Default Log", nullptr, &gDebugWindows.defaultLog);
        ImGui::MenuItem("Task Log", nullptr, &gDebugWindows.taskLog);
        ImGui::MenuItem("Unimplemented Log", nullptr, &gDebugWindows.unimplementedLog);
        ImGui::MenuItem("Sound m68k", nullptr, &gDebugWindows.soundM68k);
        ImGui::MenuItem("Warning Log", nullptr, &gDebugWindows.warningLog);
        ImGui::EndMenu();
    }
}
