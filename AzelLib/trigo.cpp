#include "PDS.h"
#include "commonOverlay.h"

#ifdef _DEBUG
class MathTest
{
public:
    MathTest()
    {
        assert(atan2_FP(s32(0x64000), s32(0x00400000)) == s32(0x1De0000));
        assert(atan2(s32(0xDB4D), s32(0x80000)) == s32(0x45));

        assert(atan2_FP(s32(0), s32(0xFF00)) == s32(0));
        assert(atan2_FP(s32(0x5000), s32(0x27200)) == s32(0x520000));
        assert(atan2_FP(s32(0x2000), s32(0xFFFD8ED0)) == s32(0x7DF0000));
        assert(atan2_FP(s32(0xFFFF6967), s32(0x0000CF04)) == s32(0xE670000));
    }
}g_MathTest;
#endif

extern s32 CosSinTable[5120];
extern u16 atanTable[2049]; 

fixedPoint getCos(u32 value)
{
    value &= 0xFFF;
    return fixedPoint(CosSinTable[value + 1024]);
}

fixedPoint getSin(u32 value)
{
    value &= 0xFFF;
    return fixedPoint(CosSinTable[value]);
}

fixedPoint atan_FP(fixedPoint inValue)
{
    if (inValue > -1)
        return (fixedPoint::fromInteger(readSaturnS16(gCommonFile->getSaturnPtr(0x021be80) + 2 * (inValue / 16))));
    return (-fixedPoint::fromInteger(readSaturnS16(gCommonFile->getSaturnPtr(0x021be80) + 2 * (-inValue / 16))));
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

u32 Q = 0;
u32 M = 0;
u32 T = 0;

void div1(s32& rm, s32& rn)
{
    unsigned long tmp0;
    unsigned char old_q, tmp1;
    old_q = Q;
    Q = (unsigned char)((0x80000000 & rn) != 0);
    rn <<= 1;
    rn |= (unsigned long)T;
    switch (old_q) {
    case 0:
        switch (M) {
        case 0:tmp0 = rn;
            rn -= rm;
            tmp1 = (rn > tmp0);
            switch (Q) {
            case 0:Q = tmp1;
                break;
            case 1:Q = (unsigned char)(tmp1 == 0);
                break;
            }
            break;
        case 1:tmp0 = rn;
            rn += rm;
            tmp1 = (rn < tmp0);
            switch (Q) {
            case 0:Q = (unsigned char)(tmp1 == 0);
                break;
            case 1:Q = tmp1;
                break;
            }
            break;
        }
        break;
    case 1:
        switch (M) {
        case 0:tmp0 = rn;
            rn += rm;
            tmp1 = (rn < tmp0);
            switch (Q) {
            case 0:Q = tmp1;
                break;
            case 1:Q = (unsigned char)(tmp1 == 0);
                break;
            }
            break;
        case 1:tmp0 = rn;
            rn -= rm;
            tmp1 = (rn > tmp0);
            switch (Q) {
            case 0:Q = (unsigned char)(tmp1 == 0);
                break;
            case 1:Q = tmp1;
                break;
            }
            break;
        }
        break;
    }
    T = (Q == M);
}

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

    y <<= 10;
    x <<= 10;

    M = T = Q = 0;

    for (int i = 0; i < 11; i++)
    {
        div1(x, y);
    }
    //s32 z = ((float)abs(y*1024) / (float)abs(x*1024)) * 2048;

    s32 z = (y << 1) | T;
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
