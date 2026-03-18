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
