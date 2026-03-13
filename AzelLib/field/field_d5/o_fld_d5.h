#pragma once

struct FLD_D5_data : public sSaturnMemoryFile
{
    FLD_D5_data() : sSaturnMemoryFile("FLD_D5.PRG")
    {
    }
};

extern FLD_D5_data* gFLD_D5;

p_workArea overlayStart_FLD_D5(p_workArea workArea, u32 arg);
