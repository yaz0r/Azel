#include "PDS.h"

extern "C" {
#include "ao.h"
#include "eng_ssf/m68k.h"
#include "eng_ssf/scsp.h"
#include "eng_ssf/sat_hw.h"
}

void audioDebug()
{
    ImGui::Begin("Sound Debugger");
    {
        for (int i=0; i<32; i++)
        {
            int offsetStart = 0x500 + i * 8;

            int dataPresent = (m68k_read_memory_8(offsetStart + 0) >> 7) & 1;
            int dataID = (m68k_read_memory_8(offsetStart + 0) >> 4) & 0x7;
            int idNumber = (m68k_read_memory_8(offsetStart + 0)) & 0x7;
            int startAddress = m68k_read_memory_32(offsetStart + 0) & 0xFFFFF;
            int transferCompleted = (m68k_read_memory_8(offsetStart + 4) >> 7) & 1;
            int areaSize = m68k_read_memory_16(offsetStart + 4) & 0xFFFFF;
            
            if (!dataPresent)
            {
                ImGui::Text("Map Info %d", i);
                ImGui::Text("dataPresent: %d", dataPresent);
                ImGui::Text("dataID: %d", dataID);
                ImGui::Text("idNumber: %d", idNumber);
                ImGui::Text("startAddress: 0x%06X", startAddress);
                ImGui::Text("transferCompleted: %d", transferCompleted);
                ImGui::Text("areaSize: 0x%06X", areaSize);
                ImGui::Separator();
            }
        }
    }
    ImGui::End();
}

