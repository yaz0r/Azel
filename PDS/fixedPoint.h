#pragma once

//typedef s32 fixedPoint;

class fixedPoint
{
private:
    s32 value;
    //u16 m_integer;
    //u16 m_fractional;

public:

    fixedPoint() : value(0)
    {

    }

    fixedPoint(s32 newValue) : value(newValue)
    {

    }

    fixedPoint operator - () const
    {
        fixedPoint newFixedPoint;
        newFixedPoint.value = -value;
        return newFixedPoint;
    }

    fixedPoint operator += (const fixedPoint& otherValue)
    {
        value += otherValue.value;
        return *this;
    }

    fixedPoint operator -= (const fixedPoint& otherValue)
    {
        value -= otherValue.value;
        return *this;
    }

    s32 asS32()
    {
        return value;
    }

    s32 getInteger()
    {
        return value >> 16;
    }

    static fixedPoint fromS32(s32 input)
    {
        fixedPoint value;
        value.value = input;
        return value;
    }

    operator s32() const
    {
        return value;
    }
};

fixedPoint MTH_Mul(fixedPoint a, fixedPoint b);
s64 MUL_FP(fixedPoint& A, fixedPoint& B);

s32 FP_GetIntegerPortion(fixedPoint& FP);
fixedPoint FP_Div(s32 divident, fixedPoint divisor);
fixedPoint performDivision(fixedPoint r0, fixedPoint r1);

