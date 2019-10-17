#include "PDS.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>

FILE* fHandle = nullptr;

void startTrace(const char* name)
{
    fHandle = nullptr;
    if (fHandle == nullptr)
    {
        fHandle = fopen(name, "r");
    }
}

bool isTraceEnabled()
{
    return fHandle != nullptr;
}

void readTraceLog(const char* fmt, u32& value)
{
    if(fHandle)
    {
        std::vector<char> buffer;
        char newChar;
        while (fread(&newChar, 1, 1, fHandle) && (newChar != '\n'))
        {
            buffer.push_back(newChar);
        }

        if (buffer.size())
        {
            sscanf(&buffer[0], fmt, &value);
        }
    }
}

void addTraceLog(const char* fmt, ...)
{
    if (fHandle)
    {
        char buffer[1024];

        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        va_end(args);

        char buffer2[1024];
        if (fread(buffer2, 1, strlen(buffer), fHandle) != strlen(buffer))
            return;
        buffer2[strlen(buffer)] = 0;

        printf(buffer);
        assert(strcmp(buffer2, buffer) == 0);
    }
}

void addTraceLog(const sVec3_FP& vec, const char* name)
{
    addTraceLog("%s: 0x%08X 0x%08X 0x%08X\n", name, vec[0].asS32(), vec[1].asS32(), vec[2].asS32());
}
void addTraceLog(const sVec2_FP& vec, const char* name)
{
    addTraceLog("%s: 0x%08X 0x%08X\n", name, vec[0].asS32(), vec[1].asS32());
}
void addTraceLog(const sMatrix4x3& matrix, const char* name)
{
    addTraceLog("%s: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n", name,
        matrix.matrix[0].asS32(), matrix.matrix[1].asS32(), matrix.matrix[2].asS32(), matrix.matrix[3].asS32(),
        matrix.matrix[4].asS32(), matrix.matrix[5].asS32(), matrix.matrix[6].asS32(), matrix.matrix[7].asS32(),
        matrix.matrix[8].asS32(), matrix.matrix[9].asS32(), matrix.matrix[10].asS32(), matrix.matrix[11].asS32()
    );
}

