#pragma once

struct FLD_D3_data : public sSaturnMemoryFile
{
    FLD_D3_data() : sSaturnMemoryFile("FLD_D3.PRG")
    {
    }
};

extern FLD_D3_data* gFLD_D3;

p_workArea overlayStart_FLD_D3(p_workArea workArea, u32 arg);
