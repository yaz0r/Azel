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

s32 FP_GetIntegerPortion(fixedPoint& FP);
fixedPoint FP_Div(fixedPoint divident, fixedPoint divisor);

