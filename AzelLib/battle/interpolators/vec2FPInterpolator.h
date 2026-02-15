#pragma once

struct sVec2FPInterpolator
{
    sVec3_FP m0_currentValue;
    sVec3_FP mC_startValue;
    sVec3_FP m18_middleValue;
    sVec3_FP m24_targetValue;
    fixedPoint m30_currentStepValue;
    fixedPoint m34_stepIncrement;
    s16 m38_interpolationLength;
};

void vec2FPInterpolator_Init(sVec2FPInterpolator* pThis);
s32 vec2FPInterpolator_Step(sVec2FPInterpolator* pThis);
