#pragma once

struct sFPInterpolator
{
    fixedPoint m0_currentValue;
    s32 m4_startValue;
    s32 m8_middleValue;
    s32 mC_targetValue;
    fixedPoint m10_currentStepValue;
    s32 m14_stepIncrement;
    s16 m18_interpolationLength;
};

void FPInterpolator_Init(sFPInterpolator* pData);
s32 FPInterpolator_Step(sFPInterpolator* pThis);
