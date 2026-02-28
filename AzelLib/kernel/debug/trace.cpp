#include "PDS.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>

FILE* fHandle = nullptr;
bool bTraceEnabled = false;

void startTrace(const char* name)
{
    //return;
    fHandle = nullptr;
    if (bTraceEnabled)
    {
        if (fHandle == nullptr)
        {
            fHandle = fopen(name, "r");
        }
    }
}

bool isTraceEnabled()
{
    return fHandle != nullptr;
}

static int counter = 0;

void readTraceLogU32(u32& value, const char* name)
{
    if(fHandle)
    {
        std::vector<char> buffer;
        char newChar;
        while (fread(&newChar, 1, 1, fHandle) && (newChar != '\n'))
        {
            buffer.push_back(newChar);
        }

        char internalFormat[1024];
        sprintf(internalFormat, "%d: %s: 0x%%08X", counter++, name);

        if (buffer.size())
        {
            int tempValue;
            if (sscanf(&buffer[0], internalFormat, &tempValue) != 1)
            {
                assert(0);
            }
            value = tempValue;
        }
    }
}

void readTraceLogU16(u16& value, const char* name)
{
    if (fHandle)
    {
        std::vector<char> buffer;
        char newChar;
        while (fread(&newChar, 1, 1, fHandle) && (newChar != '\n'))
        {
            buffer.push_back(newChar);
        }

        char internalFormat[1024];
        sprintf(internalFormat, "%d: %s: 0x%%04X", counter++, name);

        if (buffer.size())
        {
            int tempValue;
            if (sscanf(&buffer[0], internalFormat, &tempValue) != 1)
            {
                assert(0);
            }
            value = tempValue;
        }
    }
}

void readTraceLogU8(u8& value, const char* name)
{
    if (fHandle)
    {
        std::vector<char> buffer;
        char newChar;
        while (fread(&newChar, 1, 1, fHandle) && (newChar != '\n'))
        {
            buffer.push_back(newChar);
        }

        char internalFormat[1024];
        sprintf(internalFormat, "%d: %s: 0x%%04X", counter++, name);

        if (buffer.size())
        {
            int tempValue;
            if (sscanf(&buffer[0], internalFormat, &tempValue) != 1)
            {
                assert(0);
            }
            value = tempValue;
        }
    }
}

void readTraceLogS8(s8& value, const char* name)
{
    if(fHandle)
    {
        u8 temp;
        readTraceLogU8(temp, name);
        value = temp;
    }
}

void addTraceLog(const char* fmt, ...)
{
    if (fHandle)
    {
        char buffer[1024];
        sprintf(buffer, "%d: ", counter++);

        va_list args;
        va_start(args, fmt);
        vsprintf(buffer + strlen(buffer), fmt, args);
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
void addTraceLog(const fixedPoint& value, const char* name)
{
    addTraceLog("%s: 0x%08X\n", name, value.asS32());
}

void addTraceLog(const s8& value, const char* name)
{
    addTraceLog("%s: 0x%02X\n", name, value);
}

void addTraceLog(const s16& value, const char* name)
{
    addTraceLog("%s: 0x%04X\n", name, value);
}

void addTraceLog(const s32& value, const char* name)
{
    addTraceLog("%s: 0x%08X\n", name, value);
}

void addTraceLog(const sMatrix4x3& matrix, const char* name)
{
    addTraceLog("%s: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n", name,
        matrix.m[0][0].asS32(), matrix.m[0][1].asS32(), matrix.m[0][2].asS32(), matrix.m[0][3].asS32(),
        matrix.m[1][0].asS32(), matrix.m[1][1].asS32(), matrix.m[1][2].asS32(), matrix.m[1][3].asS32(),
        matrix.m[2][0].asS32(), matrix.m[2][1].asS32(), matrix.m[2][2].asS32(), matrix.m[2][3].asS32()
    );
}


