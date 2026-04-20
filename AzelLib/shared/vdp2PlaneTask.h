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

struct npcFileDeleter;

struct sVdp2PlaneTask : public s_workAreaTemplate<sVdp2PlaneTask>
{
    struct sLineScrollParams
    {
        void* m0_buffer;
        s32 m4_phaseSpeed;
        s32 m8_freqPerLine;
        s32 mC_zoomAmplitude;
        s32 m10_scrollBaseSpeed;
        s32 m14_scrollIncPerLine;
        s32 m18_phaseAccum;
        s32 m1C_scrollBaseAccum;
    };

    s32 m0_scrollX;                              // 0x00
    s32 m4_scrollY;                              // 0x04
    u8 m8_pad[4];                                // 0x08
    sVec3_FP mC_cameraPosition;                  // 0x0C
    sVec3_FP m18_cameraRotation;                 // 0x18
    std::array<s16, 4> m24_vdp1Clipping;         // 0x24
    std::array<s16, 2> m2C_vdp1LocalCoordinates;  // 0x2C
    std::array<s16, 2> m30_vdp1ProjectionParam;  // 0x30
    s32 m34;                                     // 0x34
    s32 m38;                                     // 0x38
    fixedPoint m3C_scale;                        // 0x3C
    s32 m40_waveSpeed;                           // 0x40
    s32 m44_waveFreq;                            // 0x44
    s32 m48;                       // 0x48
    s32 m4C_wavePhase;                           // 0x4C
    union {                                      // 0x50-0x6F
        sLineScrollParams m50_lineScrollParams;  // field envs pass &m50_lineScrollParams to updateWaveDistortionParams
        struct {                                 // battle env interpretation
            s8 m50;
            s8 m51;
            s8 m52;
            s8 m53;
            s8 m54;
            s8 m55;
            s8 m56;
            s8 m57;
            void* m58; // npcFileDeleter* on Saturn
            s8 m5C;
            s8 m5D_pad[0x13];
        };
    };
    union {
        u32 m70_flags;
        struct { s8 m70_colorR; s8 m71_colorG; s8 m72_colorB; u8 m73_pad; };
    };
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
    // 0x7C-0x98: battle envs pass &m7C_lineScrollParams to updateWaveDistortionParams
    sLineScrollParams m7C_lineScrollParams;       // 0x7C
    // size 0x9C
};

void vdp2ApplyWaveDistortion(sVdp2PlaneTask* pThis);
void vdp2SetupRotationPass(sVdp2PlaneTask* pThis);
void updateWaveDistortionParams(sVdp2PlaneTask::sLineScrollParams* pParams);
void updateLineScrollTable(sVdp2PlaneTask* pThis);
