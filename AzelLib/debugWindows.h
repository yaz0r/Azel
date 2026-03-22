#pragma once

// Central toggles for debug ImGui windows.
// All default to false so windows start hidden.
struct sDebugWindows
{
    bool objects = false;
    bool soundDebugger = false;
    bool collisions = false;
    bool inputState = false;
    bool vdp = false;
    bool config = false;
    bool finalComposition = false;
    bool inventory = false;
    bool tasks = false;
    bool defaultLog = false;
    bool taskLog = false;
    bool unimplementedLog = false;
    bool soundM68k = false;
    bool warningLog = false;
    bool town = false;
    bool field = false;
    bool arachnoth = false;
    bool camera = false;
    bool vdp1Vram = false;
};

extern sDebugWindows gDebugWindows;

void drawDebugMenu(); // call inside BeginMainMenuBar
