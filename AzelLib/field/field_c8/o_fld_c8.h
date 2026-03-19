#pragma once

struct FLD_C8_data : public sSaturnMemoryFile
{
    FLD_C8_data() : sSaturnMemoryFile("FLD_C8.PRG")
    {
    }
};

extern FLD_C8_data* gFLD_C8;

p_workArea overlayStart_FLD_C8(p_workArea workArea, u32 arg);

void subfieldC8_0(p_workArea workArea);
