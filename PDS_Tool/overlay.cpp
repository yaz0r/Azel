#include "overlay.h"

#include "sh2dasm.h"
#include "json.hpp"
using json = nlohmann::json;

sOverlay::sOverlay()
{
    mem_edit.ReadOnly = true;
}

void SplitOperands(char* buffer, std::vector<std::string>& operandes)
{
    char* operandStart = buffer;
    char* operandEnd = buffer;
    while((*operandEnd != '\0'))
    {
        while ((*operandEnd != ' ') && (*operandEnd != ',') && (*operandEnd != '\0'))
        {
            if (*operandEnd == '(')
            {
                operandEnd++;
                while (*operandEnd != ')')
                {
                    assert(*operandEnd != '(');
                    operandEnd++;
                }
            }
            else
            {
                operandEnd++;
            }
        }

        operandes.push_back(std::string(operandStart, operandEnd - operandStart));

        while ((*operandEnd == ' ') || (*operandEnd == ','))
        {
            operandEnd++;
        }

        operandStart = operandEnd;
    }
}

void sOverlay::DrawAsm()
{
    ImGui::BeginChild("##scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
    const int line_total_count = (int)((m_dataSize + 2 - 1) / 2);
    ImGuiListClipper clipper(line_total_count, ImGui::GetTextLineHeight());
    const size_t visible_start_addr = clipper.DisplayStart * 2;
    const size_t visible_end_addr = clipper.DisplayEnd * 2;

    for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) // display only visible lines
    {
        size_t addr = (size_t)(line_i * 2);
        ImGui::Text("0x%08X", m_base + addr);

        char buffer[1024];
        DasmSH2(buffer, m_base + addr, READ_BE_U16(m_data + addr));

        std::vector<std::string> operandes;
        SplitOperands(buffer, operandes);

        s_addressProperties* properties = NULL;
        if (m_mainMap.count(m_base + addr))
        {
            properties = &m_mainMap[m_base + addr];
        }

        // print mnemonic
        ImGui::SameLine();
        ImGui::Text("%s", operandes[0].c_str());
        if(operandes.size() > 1)
        {
            ImGui::SameLine();  ImGui::SetCursorPosX(150);

            for (int i = 1; i < operandes.size(); i++)
            {
                if (i == 1)
                {
                    if (strchr(operandes[1].c_str(), ',') == NULL)
                    {
                        if (strstr(operandes[1].c_str(), "@($") == operandes[1].c_str())
                        {
                            int symAddress = 0;
                            if (sscanf(operandes[1].c_str(), "@($%X)", &symAddress))
                            {
                                assert(symAddress >= m_base);
                                assert(symAddress < m_base + m_dataSize);

                                u32 value = READ_BE_U32(m_data + symAddress - m_base);
                                char buffer[1024];
                                sprintf(buffer, "#$%08X", value);
                                operandes[1] = std::string(buffer);
                            }
                        }

                        if (properties && properties->m_operandIsOffset)
                        {
                            if (const char* startOfOffset = strstr(operandes[1].c_str(), "$"))
                            {
                                int symAddress = 0;
                                if (sscanf(startOfOffset, "$%X", &symAddress))
                                {
                                    if (m_stringMap.count(symAddress))
                                    {
                                        operandes[1].erase(startOfOffset - operandes[1].c_str());
                                        operandes[1] += m_stringMap[symAddress];
                                        ImGui::TextColored(ImVec4(0, 0, 1, 1), operandes[i].c_str());
                                    }
                                    else
                                    {
                                        ImGui::TextColored(ImVec4(1, 1, 0, 1), operandes[i].c_str());
                                    }

                                }
                            }
                            else
                            {
                                ImGui::Text("%s", operandes[i].c_str());
                            }
                        }
                        else
                        {
                            ImGui::Text("%s", operandes[i].c_str());
                        }
                    }
                    else
                    {
                        ImGui::Text("%s", operandes[i].c_str());
                    }
                }
                else
                {
                    ImGui::Text("%s", operandes[i].c_str());
                }
                
                if (i != operandes.size() - 1)
                    ImGui::SameLine();
            }
        }
    }

    clipper.End();
    ImGui::EndChild();
}

void sOverlay::SetName(u32 address, const std::string& name)
{
    m_stringMap[address] = name;
}

void sOverlay::MarkOperandOffset(u32 address)
{
    m_mainMap[address].m_operandIsOffset = true;
}

void sOverlay::ImportJSON(const json& inputJson)
{
    const json& mainJson = inputJson["main"];
    for (auto& el : mainJson.items()) {
        std::string key = el.key();

        u32 address;
        if (sscanf(key.c_str(), "0x%X", &address) != 1)
            assert(0);
        if (address < m_base)
            continue;
        if (address > m_base + m_dataSize)
            continue;

        const json& value = el.value();
        if (value.count("name"))
        {
            SetName(address, value["name"]);
        }
        if (value.count("operands"))
        {
            if (value["operands"]["0"]["type"] == std::string("offset"))
            {
                MarkOperandOffset(address);
            }
        }
    }
}
