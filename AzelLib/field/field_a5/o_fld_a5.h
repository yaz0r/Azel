#pragma once

struct FLD_A5_data : public sSaturnMemoryFile
{
    FLD_A5_data() : sSaturnMemoryFile("FLD_A5.PRG")
    {
    }
};

extern FLD_A5_data* gFLD_A5;

p_workArea overlayStart_FLD_A5(p_workArea workArea, u32 arg);

void subfieldA5_0(p_workArea workArea);
void subfieldA5_1(p_workArea workArea);
void subfieldA5_2(p_workArea workArea);
void subfieldA5_3(p_workArea workArea);
void subfieldA5_4(p_workArea workArea);
void subfieldA5_5(p_workArea workArea);
void subfieldA5_6(p_workArea workArea);
void subfieldA5_7(p_workArea workArea);
void subfieldA5_8(p_workArea workArea);
void subfieldA5_9(p_workArea workArea);
void subfieldA5_A(p_workArea workArea);
void subfieldA5_B(p_workArea workArea);
void createA5Vdp2Task(p_workArea parent);

// Shared helpers
void setupCameraConfig_A5_0();
void setupCameraConfig_A5_2();
void setupCameraConfig_A5_4();
void setupCameraConfig_A5_7();
void setupCameraConfig_A5_8();
void setupCameraConfig_A5_9();
void setupCameraConfig_A5_corridor();
void initDragonParams_A5_open();
void initDragonParams_A5_corridor();
void initDragonParams_A5_B();
