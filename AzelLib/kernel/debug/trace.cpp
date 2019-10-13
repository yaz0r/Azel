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
        assert(fHandle);
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

        sscanf(&buffer[0], fmt, &value);
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
        fread(buffer2, 1, strlen(buffer), fHandle);
        buffer2[strlen(buffer)] = 0;

        printf(buffer);
        assert(strcmp(buffer2, buffer) == 0);
    }
}
