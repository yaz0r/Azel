#include "PDS.h"

s32 FP_GetIntegerPortion(fixedPoint& FP)
{
    return FP >> 16;
}

s32 FP_Div(fixedPoint divident, fixedPoint divisor)
{
    if (divident == 0)
        return 0;
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = divisor;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    u32 result = DVDNTUL;
    VCRDIV = 0;
    */

    return (fixedPoint)((((s64)divident) << 16) / divisor);
}
