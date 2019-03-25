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

fixedPoint dot3_FP(const sVec3_FP* r4, const sVec3_FP* r5)
{
    s64 acc = 0;
    acc += (*r4)[0] * (s64)(*r5)[0];
    acc += (*r4)[1] * (s64)(*r5)[1];
    acc += (*r4)[2] * (s64)(*r5)[2];

    return (s32)(acc >> 16);
}

fixedPoint MTH_Product3d_FP(const sVec3_FP& r4, const sVec3_FP& r5)
{
    return dot3_FP(&r4, &r5);
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

s64 MUL_FP(const fixedPoint& A, const fixedPoint& B)
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
    if (x == 0)
    {
        if (y == 0)
        {
            return 0;
        }
        if (y > 0)
        {
            return 0x400; // 90
        }
        return 0xC00; // 270
    }

    u32 r1 = 0;
    if (y < 0)
    {
        y = -y;
        r1 = 4;
    }

    if (x < 0)
    {
        x = -x;
        r1 += 2;
    }

    if (x <= y)
    {
        s32 temp = y;
        y = x;
        x = temp;
        r1++;
    }

    s32 z = ((float)abs(y) / (float)abs(x)) * 2048;

    s32 tanValue = atanTable[z & 0x7FF];

    switch (r1)
    {
    case 0:
        return tanValue;
    case 1:
        return 0x400 - tanValue;
    case 2:
        return 0x800 - tanValue;
    case 3:
        return 0x400 + tanValue;
    case 4:
        return 0x1000 - tanValue;
    case 5:
        return 0xC00 + tanValue;
    case 6:
        return 0x800 + tanValue;
    case 7:
        return 0xC00 - tanValue;
    default:
        assert(0);
    }

    return atan2_FP(y << 16, x << 16) >> 16;
    if (x)
    {
        if (abs(x) > abs(y))
        {
            s32 z = ((float)abs(y) / (float)abs(x)) * 2048;
            if (x > 0)
            {
                return atanTable[z & 0x7FF];
            }
            else if (y >= 0)
            {
                return (atanTable[z & 0x7FF] + 0x800);
            }
            else
            {
                assert(0);
                return (atanTable[z & 0x7FF] - 0x800);
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

#ifdef _DEBUG

class MathTest
{
public:
    MathTest()
    {
        assert(atan2(s32(0xDB4D), s32(0x80000)) == s32(0x45));

        assert(atan2_FP(s32(0), s32(0xFF00)) == s32(0));
        assert(atan2_FP(s32(0x5000), s32(0x27200)) == s32(0x520000));
        assert(atan2_FP(s32(0x2000), s32(0xFFFD8ED0)) == s32(0x7DF0000));
        assert(atan2_FP(s32(0xFFFF6967), s32(0x0000CF04)) == s32(0xE670000));
    }
}g_MathTest;

#endif

// this is atan2
// see https://www.dsprelated.com/showarticle/1052.php
s32 atan2_FP(s32 y, s32 x)
{
    if (x == 0)
    {
        if (y == 0)
        {
            return 0;
        }
        if (y > 0)
        {
            return 0x400 << 16; // 90
        }
        return 0xC00 << 16; // 270
    }

    u32 r1 = 0;
    if (y < 0)
    {
        y = -y;
        r1 = 4;
    }

    if (x < 0)
    {
        x = -x;
        r1 += 2;
    }

    if (x <= y)
    {
        s32 temp = y;
        y = x;
        x = temp;
        r1++;
    }

    s32 z = ((float)abs(y) / (float)abs(x)) * 2048;

    s32 tanValue = atanTable[z & 0x7FF];

    tanValue <<= 16;

    switch (r1)
    {
    case 0:
        return tanValue;
    case 1:
        return 0x4000000 - tanValue;
    case 2:
        return 0x8000000 - tanValue;
    case 3:
        return 0x4000000 + tanValue;
    case 4:
        return 0x10000000 - tanValue;
    case 5:
        return 0xC000000 + tanValue;
    case 6:
        return 0x8000000 + tanValue;
    case 7:
        return 0xC000000 - tanValue;
    default:
        assert(0);
    }
}
