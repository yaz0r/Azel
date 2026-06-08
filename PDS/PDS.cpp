#include "PDS.h"

#include <cstring>

#ifdef _WIN32
#include "validation/validation.h"
#endif

#if (defined(__APPLE__) && (TARGET_OS_OSX))
#include <unistd.h> // for chdir
#include <libgen.h> // for dirname
#include <mach-o/dyld.h> // for _NSGetExecutablePath
#include <limits.h> // for PATH_MAX?
#endif

void azelInit();
void resetEngine();
void readInputsFromSMPC();
void updateInputs();
void loopIteration();
void azelSdl_Init();
void endOfFrame();

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
    
    azelSdl_Init();

    azelInit();

#ifdef _WIN32
    // Validation is opt-in: only enabled when --validation is passed on the command line.
    {
        bool validationRequested = false;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--validation") == 0)
                validationRequested = true;
        }
        enableValidation = enableValidation && validationRequested;
    }
    // Must run before resetEngine() so the validation hooks are armed when resetEngine() calls them.
    if (enableValidation) {
        extern float gVolume;
        gVolume = 0.f;
        validationInit();
    }
#endif

    resetEngine();
    endOfFrame();
    readInputsFromSMPC();
    updateInputs();
    readInputsFromSMPC();
    updateInputs();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopIteration, 0, 1);
#else
    while ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x100) == 0)
    {
        loopIteration();

        if (!bContinue)
            break;
    };
#endif
    return 0;
}
