#pragma once

//typedef s32 fixedPoint;

class fixedPoint
{
private:
    //u16 m_integer;
    //u16 m_fractional;

public:
    s32 m_value;

    fixedPoint() : m_value(0)
    {

    }

    fixedPoint(s32 newValue) : m_value(newValue)
    {

    }

    fixedPoint operator - () const
    {
        fixedPoint newFixedPoint;
        newFixedPoint.m_value = -m_value;
        return newFixedPoint;
    }

    fixedPoint operator += (const fixedPoint& otherValue)
    {
        m_value += otherValue.m_value;
        return *this;
    }

    fixedPoint operator -= (const fixedPoint& otherValue)
    {
        m_value -= otherValue.m_value;
        return *this;
    }

    s32 asS32()
    {
        return m_value;
    }

    s32 getInteger()
    {
        return m_value >> 16;
    }

    static fixedPoint fromS32(s32 input)
    {
        fixedPoint value;
        value.m_value = input;
        return value;
    }

    operator s32() const
    {
        return m_value;
    }
};

fixedPoint MTH_Mul_5_6(fixedPoint a, fixedPoint b, fixedPoint c);
fixedPoint MTH_Mul(fixedPoint a, fixedPoint b);
s64 MUL_FP(fixedPoint& A, fixedPoint& B);

s32 FP_GetIntegerPortion(fixedPoint& FP);
fixedPoint FP_Div(s32 divident, fixedPoint divisor);
fixedPoint performDivision(fixedPoint r0, fixedPoint r1);
s32 atan2_FP(s32 y, s32 x);
s32 atan2(s32 y, s32 x);
fixedPoint FP_Pow2(fixedPoint r4);

void asyncDivStart(s32 r4, fixedPoint r5);
fixedPoint asyncDivEnd();

