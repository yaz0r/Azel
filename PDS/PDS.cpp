#include "PDS.h"

#if (defined(__APPLE__) && (TARGET_OS_OSX))
#include <unistd.h> // for chdir
#include <libgen.h> // for dirname
#include <mach-o/dyld.h> // for _NSGetExecutablePath
#include <limits.h> // for PATH_MAX?
#endif

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
#if (defined(__APPLE__) && (TARGET_OS_OSX))
    char path[PATH_MAX];
    uint32_t pathLen = sizeof(path);
    int err = _NSGetExecutablePath(path, &pathLen);
    assert(!err);
    
    // Switch to the directory of the actual binary
    chdir(dirname(path));
    // and then go up three directories to get to the folder of the .app bundle
    chdir("../Resources");
#endif
    
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
