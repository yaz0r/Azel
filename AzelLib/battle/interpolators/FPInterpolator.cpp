#include "PDS.h"
#include "FPInterpolator.h"

void FPInterpolator_Init(sFPInterpolator* pThis)
{
    pThis->m8_middleValue = (pThis->mC_targetValue - pThis->m4_startValue) / 2;
    pThis->m10_currentStepValue = 0;
    pThis->m14_stepIncrement = FP_Div(0x8000000, fixedPoint::fromInteger(pThis->m18_interpolationLength));
    pThis->m0_currentValue = pThis->m4_startValue;
}

s32 FPInterpolator_Step(sFPInterpolator* pThis)
{
    if (pThis->m10_currentStepValue > 0x7ffffff)
    {
        pThis->m10_currentStepValue = 0x8000000;
        pThis->m14_stepIncrement = 0;
        pThis->m0_currentValue = pThis->mC_targetValue;
        return true;
    }
    else
    {
        pThis->m0_currentValue = (pThis->m8_middleValue + pThis->m4_startValue) - MTH_Mul(getCos(pThis->m10_currentStepValue.getInteger()), pThis->m8_middleValue);
        pThis->m10_currentStepValue += pThis->m14_stepIncrement;
        return false;
    }
}

