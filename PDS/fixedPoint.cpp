#include "PDS.h"

s32 FP_GetIntegerPortion(fixedPoint& FP)
{
    return FP.asS32() >> 16;
}

fixedPoint FP_Div(fixedPoint divident, fixedPoint divisor)
{
    if (divident.asS32() == 0)
        return fixedPoint::fromS32(0);
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = divisor;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    u32 result = DVDNTUL;
    VCRDIV = 0;
    */

    return fixedPoint::fromS32((((s64)divident.asS32()) << 16) / divisor.asS32());
}
