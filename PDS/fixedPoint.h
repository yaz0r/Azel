#pragma once

typedef s32 fixedPoint;

class fixedPoint_
{
    u16 m_integer;
    u16 m_fractional;
};

s32 FP_GetIntegerPortion(fixedPoint& FP);
s32 FP_Div(fixedPoint divident, fixedPoint divisor);

