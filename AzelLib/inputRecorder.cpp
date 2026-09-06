#include "PDS.h"
#include "inputRecorder.h"

#include <cstdio>

void (*gOnInputFinalized)() = nullptr;

enum class eMode { Off, Recording, Playback };

static eMode gMode = eMode::Off;
static FILE* gFile = nullptr;
static u32 gFrame = 0;
static char gPath[256] = "input_recording.bin";

static constexpr u32 kMagic = 0x52534450; // 'PDSR'

using sInput = s_graphicEngineStatus_4514_inputStatus;

static void closeFile()
{
    if (gFile)
    {
        fclose(gFile);
        gFile = nullptr;
    }
}

static void stop()
{
    if (gMode == eMode::Recording && gFile)
    {
        fseek(gFile, sizeof(u32), SEEK_SET);
        fwrite(&gFrame, sizeof(u32), 1, gFile);
    }
    closeFile();
    gMode = eMode::Off;
}

void inputRecorder_startRecording(const char* path)
{
    if (path && path[0])
    {
        std::snprintf(gPath, sizeof(gPath), "%s", path);
    }
    closeFile();
    gFile = fopen(gPath, "wb");
    if (!gFile)
        return;

    const u32 header[2] = { kMagic, 0 }; // frameCount is patched in on stop
    fwrite(header, sizeof(header), 1, gFile);
    gFrame = 0;
    gMode = eMode::Recording;
}

void inputRecorder_startPlayback(const char* path)
{
    if (path && path[0])
    {
        std::snprintf(gPath, sizeof(gPath), "%s", path);
    }
    closeFile();
    gFile = fopen(gPath, "rb");
    if (!gFile)
        return;

    u32 header[2] = { 0, 0 };
    if ((fread(header, sizeof(header), 1, gFile) != 1) || (header[0] != kMagic))
    {
        closeFile();
        return;
    }
    gFrame = 0;
    gMode = eMode::Playback;
}

void updateInputRecorder()
{
    if (gMode != eMode::Off && gFile)
    {
        sInput& dev0 = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
        sInput& dev1 = graphicEngineStatus.m4514.m0_inputDevices[1].m0_current;

        if (gMode == eMode::Recording)
        {
            fwrite(&dev0, sizeof(sInput), 1, gFile);
            fwrite(&dev1, sizeof(sInput), 1, gFile);
            fflush(gFile);
            gFrame++;
        }
        else // Playback overwrites live input
        {
            if ((fread(&dev0, sizeof(sInput), 1, gFile) != 1) ||
                (fread(&dev1, sizeof(sInput), 1, gFile) != 1))
            {
                stop(); // end of recording
            }
            else
            {
                gFrame++;
            }
        }
    }

    if (gOnInputFinalized)
        gOnInputFinalized();
}
