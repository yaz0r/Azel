#include "PDS.h"

bool debugEnabled = false; // watchdog bit 1

int enableDebugTask;

char pauseEngine[3];
sPortData2 PortData2;

int drawMenuString(char* text)
{
    int result = renderVdp2String(text);

    vdp2DebugPrintPosition.X = 0;
    vdp2DebugPrintPosition.Y += 2;
    if (vdp2DebugPrintPosition.Y > 0x3F)
        vdp2DebugPrintPosition.Y = 0;

    return result;
}

void unpauseGame()
{
    pauseEngine[0] = 0;
    pauseEngine[1] = 0;
    pauseEngine[2] = 0;
}

void resetEngine()
{
    // Stuff

    unpauseGame();

    // Stuff

    resetTasks();

    // Stuff

    resetMenu(menuUnk0, 0, 0, 1);

    // Stuff

    startInitialTask();
}

void initInitialTaskStatsAndDebug()
{
    initialTaskStatus.m_showWarningTask = 1;
    enableDebugTask = 0;

    //iniitInitialTaskStatsAndDebugSub();
}

void azelInit()
{
    // stuff
    initInitialTaskStatsAndDebug();

    // stuff
}

int main(int argc, char* argv[])
{
    azelInit();
    resetEngine();

    do 
    {
        //updateDebug();

        //copySMPCOutputStatus();

        //updateInputs();

        //updateInputDebug();

        runTasks();

        //waitForSh2Completion();

        //mergeDebugStats();

        //updateSound();

        //lastUpdateFunction();

    } while (1);
    return 0;
}