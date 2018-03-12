// PDS_Tool.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <vector>

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

    uint32_t value = (file[EA]<<24) + (file[EA+1] << 16) + (file[EA+2] << 8) + (file[EA+3]);

    return value;
}

void processCommonBin()
{
    std::vector<uint8_t> common_bin;
    loadBinary("COMMON.DAT", common_bin);

    FILE* fOutput = fopen("..\\PDS\\common_generated.cpp", "w+");

    fprintf(fOutput, "#include \"PDS.h\"\n");

    // CosSinTable
    {
        uint32_t CosSinTableEA = 0x216E80;
        uint32_t CosSinTableSize = 4096;

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

    fclose(fOutput);
}

int main(int argc, char* argv[])
{
    processCommonBin();
    return 0;
}

