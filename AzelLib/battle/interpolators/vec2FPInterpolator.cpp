#include "PDS.h"
#include "vec2FPInterpolator.h"
#include "kernel/debug/trace.h"

void vec2FPInterpolator_Init(sVec2FPInterpolator* pThis)
{
    pThis->m18_middleValue = (pThis->m24_targetValue - pThis->mC_startValue) / 2;
    pThis->m30_currentStepValue = 0;
    pThis->m34_stepIncrement = FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m38_interpolationLength));
    pThis->m0_currentValue = pThis->mC_startValue;
}

s32 vec2FPInterpolator_Step(sVec2FPInterpolator* pThis)
{
    if (pThis->m30_currentStepValue > 0x7ffffff)
    {
        pThis->m30_currentStepValue = 0x8000000;
        pThis->m34_stepIncrement = 0;
        pThis->m0_currentValue = pThis->m24_targetValue;
        return true;
    }
    else
    {
        pThis->m0_currentValue = (pThis->m18_middleValue + pThis->mC_startValue) - MTH_Mul(getCos(pThis->m30_currentStepValue.getInteger()), pThis->m18_middleValue);
        pThis->m30_currentStepValue += pThis->m34_stepIncrement;

        if (isTraceEnabled())
        {
            addTraceLog(pThis->m0_currentValue, "vec2FPInterpolator_Init m0_currentValue");
        }

        return false;
    }
}
