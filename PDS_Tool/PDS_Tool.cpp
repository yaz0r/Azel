// PDS_Tool.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS 1

#pragma comment(lib, "Opengl32.lib")

#include "PDS.h"
#include "overlay.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#ifdef _WIN32
#pragma comment(lib, "Opengl32.lib")
#endif

SDL_Window *gWindow;
SDL_GLContext gGlcontext;

std::vector<uint8_t> common_bin;

bool loadBinary(const char* name, std::vector<uint8_t>& output)
{
    FILE* fHandle = fopen(name, "rb");
    assert(fHandle);

    fseek(fHandle, 0, SEEK_END);
    uint32_t binSize = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);

    output.resize(binSize);
    fread(&output[0], 1, binSize, fHandle);
    fclose(fHandle);

    return true;
}

uint32_t readU32(uint32_t EA, std::vector<uint8_t> file, uint32_t base)
{
    assert(EA >= base);
    EA -= base;

    uint32_t value = (file[EA] << 24) + (file[EA + 1] << 16) + (file[EA + 2] << 8) + (file[EA + 3]);

    return value;
}

uint16_t readU16(uint32_t EA, std::vector<uint8_t> file, uint32_t base)
{
    assert(EA >= base);
    EA -= base;

    uint16_t value = (file[EA + 0] << 8) + (file[EA + 1]);

    return value;
}

sDragonData3Sub readDragonData3Sub(uint32_t EA)
{
    sDragonData3Sub entry;

    for (int i = 0; i < 4; i++)
    {
        entry.m_m0[i] = readU32(EA, common_bin, 0x200000); EA += 4;
    }


    return entry;
}

std::vector<sDragonData3> processDragonData3Array(FILE* fOutput, uint32_t dragonData3_EA)
{
    std::vector<sDragonData3> result;

    for (int i = 0; i < 9; i++)
    {
        sDragonData3 entry;
        entry.m_m0 = readU32(dragonData3_EA, common_bin, 0x200000); dragonData3_EA += 4;
        entry.m_m4 = readU32(dragonData3_EA, common_bin, 0x200000); dragonData3_EA += 4;

        for (int j = 0; j < 7; j++)
        {
            entry.m_m8[j] = readDragonData3Sub(dragonData3_EA); dragonData3_EA += 0xC;
        }

        result.push_back(entry);
    }

    return result;
}

void processCommonBin()
{
    loadBinary("COMMON.DAT", common_bin);

    FILE* fOutput = fopen("..\\PDS\\common_generated.cpp", "w+");

    fprintf(fOutput, "#include \"PDS.h\"\n");

    // CosSinTable
    {
        uint32_t CosSinTableEA = 0x216E80;
        uint32_t CosSinTableSize = 4096 + 1024;

        std::vector<uint32_t> CosSinTable;
        CosSinTable.reserve(CosSinTableSize);

        for (int i = 0; i < CosSinTableSize; i++)
        {
            uint32_t value = readU32(CosSinTableEA + i * 4, common_bin, 0x200000);
            CosSinTable.push_back(value);
        }

        // dump
        fprintf(fOutput, "s32 CosSinTable[%d] = {\n", CosSinTableSize);
        for (int i = 0; i < CosSinTableSize; i++)
        {
            fprintf(fOutput, "\t(s32)(0x%08X),\n", CosSinTable[i]);
        }
        fprintf(fOutput, "};\n");
    }
    // atan
    {
        uint32_t atanTableEA = 0x21FE82;
        uint32_t atanTableSize = 2049;

        std::vector<uint16_t> atanTable;
        atanTable.reserve(atanTableSize);

        for (int i = 0; i < atanTableSize; i++)
        {
            uint16_t value = readU16(atanTableEA + i * 2, common_bin, 0x200000);
            atanTable.push_back(value);
        }

        // dump
        fprintf(fOutput, "u16 atanTable[%d] = {\n", atanTableSize);
        for (int i = 0; i < atanTableSize; i++)
        {
            fprintf(fOutput, "\t0x%08X,\n", atanTable[i]);
        }
        fprintf(fOutput, "};\n");
    }
    // resetVdp2StringsData
    {
        uint32_t EA = 0x210E98;
        uint32_t Size = 4106;

        std::vector<uint16_t> data;
        data.reserve(Size);

        for (int i = 0; i < Size; i++)
        {
            uint16_t value = readU16(EA + i * 2, common_bin, 0x200000);
            data.push_back(value);
        }

        // dump
        fprintf(fOutput, "u16 resetVdp2StringsData[%d] = {\n", Size);
        for (int i = 0; i < Size; i++)
        {
            fprintf(fOutput, "\t0x%08X,\n", data[i]);
        }
        fprintf(fOutput, "};\n");

    }
    processDragonData3Array(fOutput, 0x2065E8);

    fclose(fOutput);
}

std::vector<sOverlay*> gOverlays;
std::vector<sOverlay*> gGlobalOverlays;

sOverlay* loadOverlay(const std::string& filename, u32 baseAddress, u32 endAddress)
{
    FILE* fHandle = fopen(filename.c_str(), "rb");
    if (fHandle)
    {
        sOverlay* pNewOverlay = new sOverlay;
        pNewOverlay->m_name = filename;
        pNewOverlay->m_base = baseAddress;
        pNewOverlay->m_dataSize = endAddress - baseAddress;
        pNewOverlay->m_data = new u8[pNewOverlay->m_dataSize];
        memset(pNewOverlay->m_data, 0, pNewOverlay->m_dataSize);

        fseek(fHandle, 0, SEEK_END);
        int fileSize = ftell(fHandle);
        assert(fileSize <= pNewOverlay->m_dataSize);
        fseek(fHandle, 0, SEEK_SET);
        fread(pNewOverlay->m_data, 1, fileSize, fHandle);
        fclose(fHandle);

        gOverlays.push_back(pNewOverlay);
        return pNewOverlay;
    }
    return NULL;
}
void loadGlobalOverlay(const std::string& filename, u32 baseAddress, u32 endAddress)
{
    sOverlay* pOverlay = loadOverlay(filename, baseAddress, endAddress);
    if (pOverlay)
    {
        gGlobalOverlays.push_back(pOverlay);
    }
}

void loadOverlays()
{
    loadGlobalOverlay(std::string("COMMON.DAT"), 0x200000, 0x250000);
    loadGlobalOverlay(std::string("1ST_READ.PRG"), 0x6006000, 0x6054000);
    loadOverlay(std::string("FLD_A3.PRG"), 0x6054000, 0x60D0000);
}

void processOverlay(sOverlay* pOverlay)
{
    if (ImGui::Begin(pOverlay->m_name.c_str(), NULL, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Import JSON"))
                {
                    //pOverlay->ImportJSON();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        if (ImGui::BeginTabBar("Menu"))
        {
            if (ImGui::BeginTabItem("ASM"))
            {
                pOverlay->DrawAsm();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Hex"))
            {
                pOverlay->mem_edit.DrawContents(pOverlay->m_data, pOverlay->m_dataSize, pOverlay->m_base);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void PDSToolInit()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        assert(false);
    }

#ifdef USE_GL_ES3 
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif
    gWindow = SDL_CreateWindow("PDSTool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    assert(gWindow);

    gGlcontext = SDL_GL_CreateContext(gWindow);
    assert(gGlcontext);

#ifndef USE_GL_ES3
    gl3wInit();
#endif

    // Setup ImGui binding
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
#ifndef __EMSCRIPTEN__
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    printf("glsl_version: %s\n", glsl_version);

    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplSDL2_InitForOpenGL(gWindow, gGlcontext);
}

void PDSToolLoopStart()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        default:
            break;
        }
    }

    checkGL();
    ImGui_ImplOpenGL3_NewFrame();
    checkGL();
    ImGui_ImplSDL2_NewFrame(gWindow);
    ImGui::NewFrame();
    checkGL();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PDSToolLoopEnd()
{
    // End
    checkGL();

    //PDS_Logger.Draw("Logs");

    ImGui::Render();

    checkGL();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(gWindow, gGlcontext);
    }

    checkGL();

    glFlush();

    checkGL();

    int frameLimit = 60;
    {
        static Uint64 last_time = SDL_GetPerformanceCounter();
        Uint64 now = SDL_GetPerformanceCounter();

        float freq = SDL_GetPerformanceFrequency();
        float secs = (now - last_time) / freq;
        float timeToWait = ((1.f / frameLimit) - secs) * 1000;
        //timeToWait = 0;
        if (timeToWait > 0)
        {
            //SDL_Delay(timeToWait);
        }

        SDL_GL_SwapWindow(gWindow);

        last_time = SDL_GetPerformanceCounter();
    }

    checkGL();
}

int main(int argc, char* argv[])
{
    PDSToolInit();

    loadOverlays();
    {
        std::ifstream input("output.json");
        json mainJson;
        input >> mainJson;
        for (int i = 0; i < gOverlays.size(); i++)
        {
            gOverlays[i]->ImportJSON(mainJson);
        }
    }

    processCommonBin();

    do
    {
        PDSToolLoopStart();

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text(" %.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

        for (int i=0; i<gOverlays.size(); i++)
        {
            processOverlay(gOverlays[i]);
        }

        PDSToolLoopEnd();
    }while (1);

    return 0;
}

