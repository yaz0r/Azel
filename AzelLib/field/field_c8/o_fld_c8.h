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
    // size 0xEC
};

p_workArea overlayStart_FLD_C8(p_workArea workArea, u32 arg);
void createC8Vdp2Task(p_workArea parent, sSaturnPtr towerData);

void subfieldC8_0(p_workArea workArea);
void subfieldC8_1(p_workArea workArea);
