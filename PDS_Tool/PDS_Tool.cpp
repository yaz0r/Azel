// PDS_Tool.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS 1

#pragma comment(lib, "Opengl32.lib")

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <vector>

#define HEADLESS_TOOL

#include "PDS.h"

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

int main(int argc, char* argv[])
{
    processCommonBin();
    return 0;
}

