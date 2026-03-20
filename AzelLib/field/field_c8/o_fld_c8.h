#pragma once

struct FLD_C8_data : public sSaturnMemoryFile
{
    FLD_C8_data() : sSaturnMemoryFile("FLD_C8.PRG")
    {
    }
};

extern FLD_C8_data* gFLD_C8;

struct s_fieldSpecificData_C8 : public s_workAreaTemplate<s_fieldSpecificData_C8>
{
    s8 m25;
    s8 m27;
    s32 m28;
    s32 m2C;
    s32 m38;
    s32 m3C;
    sVec3_FP m4C_dragonPos;
    sVec3_FP m64_cameraTarget;
    p_workArea mA0_particlePool;
    p_workArea mB4_effectManager;
    p_workArea mD4_subtask;
    p_workArea mD8_soundEmitter;
    p_workArea mDC_zoneCollision;
    s32 mE0_zoneIndex0;
    s32 mE4_zoneIndex1;
    // size 0xEC
};

p_workArea overlayStart_FLD_C8(p_workArea workArea, u32 arg);
void createC8Vdp2Task(p_workArea parent, sSaturnPtr towerData);

void setupFieldC8(sSaturnPtr dataTable3EA, sSaturnPtr visibilityEA, void(*createFunc)(p_workArea));

void subfieldC8_0(p_workArea workArea);
void subfieldC8_1(p_workArea workArea);
