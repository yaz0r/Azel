#pragma once

struct FLD_B2_data : public sSaturnMemoryFile
{
    FLD_B2_data() : sSaturnMemoryFile("FLD_B2.PRG")
    {
    }
};

extern FLD_B2_data* gFLD_B2;

p_workArea overlayStart_FLD_B2(p_workArea workArea, u32 arg);

void subfieldB2_0(p_workArea workArea);
void subfieldB2_1(p_workArea workArea);
void subfieldB2_2(p_workArea workArea);
void subfieldB2_3(p_workArea workArea);

// Shared helpers
void initDragonParams_B2_desert();
void initDragonParams_B2_oasis();
void initDragonParams_B2_oasisStorm();
void initDragonParams_B2_underground();
void setupB2CameraConfigs(sSaturnPtr configEA, sSaturnPtr visibilityEA, void(*startTasks)(p_workArea));
void setupCameraVisibility_B2_desert();
void setupCameraVisibility_B2_underground();
void activateCameraScript_B2(sSaturnPtr scriptDataEA);
void postBattleSound_B2_empty();

// B2 field-specific data (size 0xA4)
struct s_fieldSpecificData_B2 : public s_workAreaTemplate<s_fieldSpecificData_B2>
{
    u8 m0_pad[0x3C];
    p_workArea m3C_sceneManager;
    p_workArea m40;
    u8 m44_pad[0x10];
    p_workArea m54_ambientSoundTask;
    u8 m58_pad[0x48];
    u8 mA0_flag;
    // size 0xA4
};

inline s_fieldSpecificData_B2* getFieldSpecificData_B2() { return (s_fieldSpecificData_B2*)getFieldTaskPtr()->mC; }

// Shared field data/scene creation
void createFieldSpecificDataTask_B2(p_workArea parent);

// VDP2 plane task creation (per-subfield)
void createB2Vdp2Task_0(p_workArea parent);
void createB2Vdp2Task_1(p_workArea parent);
void createB2Vdp2Task_2(p_workArea parent);
void createB2Vdp2Task_3(p_workArea parent);
