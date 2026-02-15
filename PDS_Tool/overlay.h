#pragma once

#include "json.hpp"
using json = nlohmann::json;

#include "PDS.h"
#include "imgui_memory_editor.h"

class sOverlay : public sSaturnMemoryFile
{
public:
    sOverlay();

    void DrawAsm();

    void SetName(u32 address, const std::string& name);
    void MarkOperandOffset(u32 address);
    void ImportJSON(const json& inputJson);

    MemoryEditor mem_edit;

    struct s_addressProperties
    {
        bool m_operandIsOffset;
    };

    std::map<u32, std::string> m_stringMap;
    std::map<u32, s_addressProperties> m_mainMap;
};

