#include "PDS.h"
#include "pcmPlayer.h"

SoLoud::handle playPCMFile(const char* filename, s32 sampleRate)
{
    FILE* fHandle = fopen(filename, "rb");
    if (!fHandle)
    {
        printf("Failed to play PCM %s\n", filename);
        return 0;
    }

    fseek(fHandle, 0, SEEK_END);
    int size = ftell(fHandle) / 2;
    fseek(fHandle, 0, SEEK_SET);
    s16* buffer = new s16[size];
    fread(buffer, 2, size, fHandle);
    fclose(fHandle);

    for (int i = 0; i < size; i++)
    {
        buffer[i] = READ_BE_S16(buffer + i);
    }

    SoLoud::Wav* newWav = new SoLoud::Wav();
    newWav->loadRawWave16(buffer, size, (float)sampleRate, 1);

    return gSoloud.play(*newWav);
}
