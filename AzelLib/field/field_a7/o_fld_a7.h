#pragma once

struct FLD_A7_data : public sSaturnMemoryFile
{
    FLD_A7_data() : sSaturnMemoryFile("FLD_A7.PRG")
    {
    }
};

extern FLD_A7_data* gFLD_A7;

struct s_fieldSpecificData_A7 : public s_workAreaTemplate<s_fieldSpecificData_A7>
{
    u8 m0_pad[0x276];
    u8 m276;
    u8 m277;
    u8 m278;
    u8 m279;
    u8 m27A_pad[6];
    void* m280;
    // size 0x284
};

p_workArea overlayStart_FLD_A7(p_workArea workArea, u32 arg);

void subfieldA7_0(p_workArea workArea);
void subfieldA7_1(p_workArea workArea);
void subfieldA7_2(p_workArea workArea);
