#include "PDS.h"

s32 FP_GetIntegerPortion(fixedPoint& FP)
{
    return FP.asS32() >> 16;
}

fixedPoint FP_Div(s32 divident, fixedPoint divisor)
{
    if (divident == 0)
        return fixedPoint::fromS32(0);
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = divisor;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    u32 result = DVDNTUL;
    VCRDIV = 0;
    */

    return fixedPoint::fromS32((((s64)divident) << 16) / divisor.asS32());
}

fixedPoint dot3_FP(sVec3_FP* r4, sVec3_FP* r5)
{
    s64 acc = 0;
    acc += (*r4)[0] * (*r5)[0];
    acc += (*r4)[1] * (*r5)[1];
    acc += (*r4)[2] * (*r5)[2];

    return (s32)(acc >> 16);
}

s32 gDivident;
fixedPoint gDivisor;

void asyncDivStart(s32 r4, fixedPoint r5)
{
    /*
    VCRDIV = 0xFFFFFF00;
    DVSR = r5;
    DVDNTH = (divident >> 16) & 0xFFFF;
    DVDNTL = divident << 16;
    */

    gDivident = r4;
    gDivisor = r5;
}

fixedPoint asyncDivEnd()
{
    return fixedPoint::fromS32((((s64)gDivident) << 16) / gDivisor.asS32());
}

fixedPoint performDivision(fixedPoint r0, fixedPoint r1)
{
    return r1 / r0;
}

s64 MUL_FP(fixedPoint& A, fixedPoint& B)
{
    return (s64)A.asS32() * (s64)B.asS32();
}

fixedPoint MTH_Mul_5_6(fixedPoint a, fixedPoint b, fixedPoint c)
{
    return MTH_Mul(a, MTH_Mul(b, c));
}

fixedPoint MTH_Mul(fixedPoint a, fixedPoint b)
{
    return fixedPoint(((s64)a.asS32() * (s64)b.asS32()) >> 16);
}

fixedPoint FP_Pow2(fixedPoint r4)
{
    s64 result = ((s64)r4.asS32()) * ((s64)r4.asS32());
    return fixedPoint::fromS32(result >> 16);
}

s32 atan2(s32 y, s32 x)
{
    if (x)
    {
        if (abs(x) > abs(y))
        {
            s32 z = y / x;
            if (x > 0)
            {
                return atanTable[z];
            }
            else if (y >= 0)
            {
                return (atanTable[z] + 0x800);
            }
            else
            {
                assert(0);
                return (atanTable[z] - 0x800);
            }
        }
        else
        {
            assert(0);
        }
    }
    if (y)
    {
        if (y >= 0)
        {
            return 0x400; //90
        }
        else
        {
            return 0xC00; //270
        }
    }

    return 0;
}

// this is atan2
// see https://www.dsprelated.com/showarticle/1052.php
s32 atan2_FP(s32 y, s32 x)
{
    if (x)
    {
        if (abs(x) > abs(y))
        {
            s32 z = y / x;
            if (x > 0)
            {
                return atanTable[z] << 16;
            }
            else if (y >= 0)
            {
                return (atanTable[z] << 16) + 0x8000000;
            }
            else
            {
                return (atanTable[z] << 16) - 0x8000000;
            }
        }
        else
        {
            s32 z = x / y;
            if (y > 0)
            {
                return (-atanTable[z] << 16) + 0x4000000;
            }
            else
            {
                return (-atanTable[z] << 16) - 0x4000000;
            }
        }
    }
    if (y)
    {
        if (y >= 0)
        {
            return 0x4000000; //90
        }
        else
        {
            return 0xC000000; //270
        }
    }

    return 0;
}

#if 0
s32 atan2_FP(s32 y, s32 x)
{
    if (x)
    {
        u32 r1 = 0;
        if (y < 0)
        {
            y = -y;
            r1 = 1;
        }

        if (x < 0)
        {
            x = -x;
            r1 += 2;
        }

        assert(y == 0);
        assert(x == 0x10000);

        return 0;

        if (y > x)
        {
            x ^= y;
            y ^= x;
            x ^= y;
            r1 += 1;
        }

        y <<= 10;
        x <<= 10;
        // TO BE CONTINUED

    }
    if (y)
    {
        if (y >= 0)
        {
            return 0x400 << 16; //90
        }
        else
        {
            return 0xC00 << 16; //270
        }
    }

    return 0;
}
#endif
