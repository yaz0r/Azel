#pragma once

#include "task.h"

struct s_fieldPaletteTaskWorkSub
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
};

struct sVdp2PlaneTask : public s_workAreaTemplate<sVdp2PlaneTask>
{
    s32 m0_scrollX;                              // 0x00
    s32 m4_scrollY;                              // 0x04
    u8 m8_pad[4];                                // 0x08
    sVec3_FP mC_cameraPosition;                  // 0x0C
    sVec3_FP m18_cameraRotation;                 // 0x18
    std::array<s16, 4> m24_vdp1Clipping;         // 0x24
    std::array<s16, 2> m2C_localCoordinates;     // 0x2C
    s16 m30_projParam0;                          // 0x30
    s16 m32_projParam1;                          // 0x32
    s32 m34_scrollValue;                         // 0x34
    s32 m38_groundY;                             // 0x38
    fixedPoint m3C_scale;                        // 0x3C
    s32 m40_waveSpeed;                           // 0x40
    s32 m44_waveFreq;                            // 0x44
    s32 m48_waveAmplitude;                       // 0x48
    s32 m4C_wavePhase;                           // 0x4C
    void* m50_lineScrollBuffer;                  // 0x50
    s32 m54_lsPhaseSpeed;                        // 0x54
    s32 m58_lsFreqPerLine;                       // 0x58
    s32 m5C_lsZoomAmplitude;                     // 0x5C
    s32 m60_lsScrollBaseSpeed;                   // 0x60
    s32 m64_lsScrollIncPerLine;                  // 0x64
    s32 m68_lsPhaseAccum;                        // 0x68
    s32 m6C_lsScrollBaseAccum;                   // 0x6C
    s8 m70_colorR;                               // 0x70
    s8 m71_colorG;                               // 0x71
    s8 m72_colorB;                               // 0x72
    u8 m73_pad;                                  // 0x73
    s8 m74_colorNBG;                             // 0x74
    s8 m75_colorRBG0;                            // 0x75
    s8 m76_animPhase;                            // 0x76
    s8 m77_animSpeed;                            // 0x77
    union {                                      // 0x78
        void* m78_ptr;
        s_fieldPaletteTaskWorkSub* m78_paletteWorkSub;
        s32* m78_skyRotationBuffer;
        u8* m78_auxBuffer;
    };
    // size 0x9C
};

void vdp2ApplyWaveDistortion(sVdp2PlaneTask* pThis);
void vdp2SetupRotationPass(sVdp2PlaneTask* pThis);
