
#include "PDS.h"

SDL_Window *gWindow;
SDL_GLContext gGlcontext;

void azelInit();
void resetEngine();
void readInputsFromSMPC();
void updateInputs();
void loopIteration();

extern bool bContinue;

extern "C" {
    int PDSmain(int argc, char* argv[]);
}

int PDSmain(int argc, char* argv[])
{
    azelSdl2_Init();

    checkGL();

    azelInit();
    resetEngine();

    //...
    readInputsFromSMPC();
    updateInputs();
    readInputsFromSMPC();
    updateInputs();

    u32 frameCounter = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopIteration, 0, 1);
#else
    do
    {
        loopIteration();
    } while (bContinue);
#endif
    return 0;
}

void drawDebugLine(sVec3_FP const&, sVec3_FP const&, sFColor const&)
{
    
}

void drawDebugFilledQuad(sVec3_FP const&, sVec3_FP const&, sVec3_FP const&, sVec3_FP const&, sFColor const&)
{
}

void addObjectToDrawList(sProcessed3dModel*)
{
}

void addBillBoardToDrawList(unsigned char*, unsigned int)
{}

void addObjectToDrawList(unsigned char*, unsigned int)
{}

void registerModelAndCharacter(unsigned char*, unsigned char*)
{}

void flushObjectsToDrawList() {}
