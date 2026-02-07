#include "PDS.h"
#include <cstdio>

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
void azelSdl2_Init();

extern bool bContinue;

int main(int argc, char* argv[])
{
    fprintf(stderr, "=== main() starting ===\n");
    fflush(stderr);

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

    fprintf(stderr, "=== Calling azelSdl2_Init() ===\n");
    fflush(stderr);
    azelSdl2_Init();
    fprintf(stderr, "=== azelSdl2_Init() done ===\n");
    fflush(stderr);

    azelInit();
    fprintf(stderr, "=== azelInit() done ===\n");
    fflush(stderr);
    resetEngine();
    fprintf(stderr, "=== resetEngine() done ===\n");
    fflush(stderr);

    //...
    readInputsFromSMPC();
    updateInputs();
    readInputsFromSMPC();
    updateInputs();

    fprintf(stderr, "=== Entering main loop ===\n");
    fflush(stderr);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopIteration, 0, 1);
#else
    do
    {
        loopIteration();
    } while (bContinue);
#endif

#ifndef SHIPPING_BUILD
    cleanupLoggers();
#endif
    return 0;
}
