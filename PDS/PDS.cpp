#include "PDS.h"

SDL_Window *gWindow;
SDL_GLContext gGlcontext;

void azelInit();
void resetEngine();
void readInputsFromSMPC();
void updateInputs();
void loopIteration();

extern bool bContinue;

int main(int argc, char* argv[])
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
