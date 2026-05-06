#include "PDS.h"
#include "fieldDragonInput.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldScriptWaitTask.h"
#include "field/fieldVisibilityGrid.h"

fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);

// Shared dragon input/physics helper functions.
// Compiled identically into every field overlay. Extracted from
// o_fld_a5.cpp and a7_envEntity2C.cpp to eliminate duplication.

static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

// 06085ffc (A5) / 0607e408 (A3) / 0607c9d4 (A7)
void computeDragonSpeed(s_dragonTaskWorkArea* pDragon)
{
    sVec3_FP scaledDelta;
    scaledDelta.m0_X = fixedPoint(pDragon->m160_deltaTranslation.m0_X.m_value << 8);
    scaledDelta.m4_Y = fixedPoint(pDragon->m160_deltaTranslation.m4_Y.m_value << 8);
    scaledDelta.m8_Z = fixedPoint(pDragon->m160_deltaTranslation.m8_Z.m_value << 8);
    fixedPoint mag = sqrt_F(dot3_FP(&scaledDelta, &scaledDelta));
    pDragon->m154_dragonSpeed = fixedPoint(mag.m_value >> 8);
}

// Helper used across all overlays
fixedPoint interpolateAngle28(fixedPoint current, fixedPoint target)
{
    s32 diff = target.m_value - current.m_value;
    if ((diff & 0x8000000) == 0)
        diff &= 0xFFFFFFF;
    else
        diff |= (s32)0xF0000000;
    s32 step = (diff * 0xF) / 0x10;
    return fixedPoint(current.m_value + (target.m_value - step - current.m_value));
}

// 0608617e (A5) / 0607cb56 (A7)
s32 isDragonScriptBlocked()
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    if ((pDragon->mF8_Flags & 0x10000) != 0) return 1;
    if ((pLCS->m8 & 0xC8) != 0) return 1;
    if (pScript->m4_currentScript.m_offset != 0) return 1;
    if (pScript->m30_cinematicBarTask != nullptr) return 1;
    if (pScript->m34 != 0) return 1;
    if (pScript->m38_dialogStringTask != nullptr) return 1;
    if (pScript->m3C_multichoiceTask != nullptr) return 1;
    return 0;
}

// 06086130 (A5) / 0607cb08 (A7)
s32 isDragonFreeFlying()
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    if (pLCS->m8 != 0) return 0;
    if (pScript->m4_currentScript.m_offset != 0) return 0;
    if (pScript->m30_cinematicBarTask != nullptr) return 0;
    if (pScript->m34 != 0) return 0;
    if (pScript->m38_dialogStringTask != nullptr) return 0;
    if (pScript->m3C_multichoiceTask != nullptr) return 0;
    return 1;
}

// 060860a8 (A5) / 0607ca80 (A7)
s32 canAcceptDragonInput()
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    if ((pDragon->mF8_Flags & 0x10000) != 0) return 0;
    if (pLCS->m8 != 0) return 0;
    if (pScript->m4_currentScript.m_offset != 0) return 0;
    if (pScript->m30_cinematicBarTask != nullptr) return 0;
    if (pScript->m34 != 0) return 0;
    if (pScript->m38_dialogStringTask != nullptr) return 0;
    if (pScript->m3C_multichoiceTask != nullptr) return 0;
    return 1;
}

// 0607c4bc (A5) / 06072e94 (A7)
void clearDragonVelocity()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->m188 = {};
    pDragon->m194 = {};
    pDragon->m1A0 = {};
    pDragon->m1AC = {};
}

// 0607c924 (A5) / 060732fc (A7)
void clearDragonScriptFlag()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->mF8_Flags &= ~0x10000;
}

// 0607d23a (A5) / 06073c12 (A7)
bool isBerserkUnlocked()
{
    return (mainGameState.bitField[0x2A] & 1) != 0;
}

// 0607d228 (A5) / 06073c00 (A7)
bool isLightWingUnlocked()
{
    return (mainGameState.bitField[0x2A] & 2) != 0;
}

// 06086458 (A5) / 0607ce30 (A7)
void resetSpeedIndex(s_dragonTaskWorkArea* pDragon)
{
    if ((pDragon->m25C & 1) == 0)
    {
        pDragon->m235_dragonSpeedIndex = 0;
        pDragon->m234 = 0;
        pDragon->m25C = 0;
    }
}

// 060864a4 (A5) / 0607ce7c (A7)
void dragonSpeedControl(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];

    if ((pDragon->m25C & 1) != 0)
    {
        if (((config[0] | config[0xB]) & input.m8_newButtonDown) != 0 ||
            pDragon->m235_dragonSpeedIndex < 1)
        {
            pDragon->m25C &= ~1;
        }
    }

    if ((pDragon->m25C & 2) == 0 &&
        (config[0xB] & input.m6_buttonDown) != 0)
    {
        if ((config[0] & input.m6_buttonDown) == 0)
        {
            resetSpeedIndex(pDragon);
            return;
        }
        pDragon->m235_dragonSpeedIndex = -1;
    }
    else
    {
        pDragon->m25C |= 2;

        if (isBerserkUnlocked() &&
            (config[0xF] & input.m6_buttonDown) != 0)
        {
            pDragon->m25C |= 4;
        }

        s8 maxGear = (pDragon->m25C & 4) ? 4 : 3;

        if ((config[0] & input.m6_buttonDown) == 0)
        {
            resetSpeedIndex(pDragon);
        }
        else
        {
            pDragon->m234++;
            if (pDragon->m234 > 4)
            {
                pDragon->m235_dragonSpeedIndex++;
                if (pDragon->m235_dragonSpeedIndex >= maxGear)
                    pDragon->m235_dragonSpeedIndex = maxGear;
                pDragon->m234 = 0;
            }
        }

        if (isLightWingUnlocked() &&
            pDragon->m235_dragonSpeedIndex > 0 &&
            pDragon->m154_dragonSpeed.m_value > 0 &&
            (config[0xB] & input.m8_newButtonDown) != 0)
        {
            pDragon->m25C |= 1;
            pDragon->m158 = pDragon->m154_dragonSpeed;
        }
    }
}

// 060861de (A5) / 0607cbb6 (A7)
void resetDragonInput(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m254 = 0;
    pDragon->m250 = 0;
    *(u32*)((u8*)pDragon + 0x24C) = 0;
    pDragon->m258 = 0;
    pDragon->m25C = 0;
    pDragon->m25D = 0;
}

// 060865d8 (A5) / 0607cfb0 (A7)
void dragonInput_default(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m258--;
    if ((s32)pDragon->m258 < 1)
    {
        resetDragonInput(pDragon);
    }

    pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);

    if (pDragon->m25D == 2)
    {
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + pDragon->m254);
    }
    else
    {
        pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);
    }

    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    pDragon->m247 = 0;
    pDragon->m246_previousAnalogY = 0;
    pDragon->m245_previousAnalogX = 0;
    pDragon->m25C &= ~1;
}

// 06086700 (A5) / 0607d0d8 (A7)
void dragonAnimFromDPad(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];
    s32 rate = pDragon->m184_animRate.m_value;

    if (config[5] & input.m8_newButtonDown)
        incrementAnimationRootY(&gDragonState->m78_animData, rate);
    else if (config[4] & input.m8_newButtonDown)
        incrementAnimationRootY(&gDragonState->m78_animData, -rate);

    if (config[7] & input.m8_newButtonDown)
    {
        incrementAnimationRootX(&gDragonState->m78_animData, rate);
        incrementAnimationRootZ(&gDragonState->m78_animData, -rate);
    }
    else if (config[6] & input.m8_newButtonDown)
    {
        incrementAnimationRootX(&gDragonState->m78_animData, -rate);
        incrementAnimationRootZ(&gDragonState->m78_animData, rate);
    }
}

// 060861fa (A5) / 0607cbd2 (A7)
void setYawInterpMode(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m25D = 1;
    pDragon->m258 = 0x1E;
    pDragon->m23C |= 4;
    pDragon->m244 = 4;
}

// 0608622c (A5) / 0607cc04 (A7)
void interpYawWithBanking(s_dragonTaskWorkArea* pDragon)
{
    fixedPoint turnRate = pDragon->m178_turnRate;
    pDragon->m20_angle.m4_Y = interpolateRotation(pDragon->m20_angle.m4_Y, fixedPoint(pDragon->m250), fixedPoint(0x2000), fixedPoint(0x444444), 0);

    s32 yawDelta = pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value;
    if (yawDelta < turnRate.m_value)
    {
        s32 bankAmount = performDivision(4, turnRate.m_value * 3);
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + bankAmount);
    }
    else if (turnRate.m_value <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
    {
        s32 bankAmount = performDivision(4, turnRate.m_value * 3);
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - bankAmount);
    }
}

// 0608630c (A5) / 0607cce4 (A7)
void dragonSpecialInput(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];

    pDragon->m258--;
    if ((s32)pDragon->m258 >= 1)
        return;

    pDragon->m25D = 0;
    pDragon->m258 = 0;

    if ((config[0xF] & input.m8_newButtonDown) == 0)
        return;

    if ((config[0] & input.m6_buttonDown) != 0)
    {
        if (gDragonState->mC_dragonType == 8)
            return;
        pDragon->m25D = 2;
        pDragon->m258 = 0x18;
        if (pDragon->m25E == 0)
            pDragon->m254 = 0x84BDA1;
        else
            pDragon->m254 = (s32)0xFF7B425F;
        pDragon->m23C |= 4;
        pDragon->m244 = 9;
        return;
    }

    if (pDragon->m235_dragonSpeedIndex != 0)
        return;

    if ((config[7] & input.m6_buttonDown) != 0)
    {
        s32 target = pDragon->m20_angle.m4_Y.m_value + (s32)0xF838E38F;
        target = (target & 0x8000000) ? (target | (s32)0xF0000000) : (target & 0x0FFFFFFF);
        pDragon->m250 = target;
        setYawInterpMode(pDragon);
        return;
    }

    if ((config[6] & input.m6_buttonDown) != 0)
    {
        s32 target = pDragon->m20_angle.m4_Y.m_value + 0x07C71C71;
        target = (target & 0x8000000) ? (target | (s32)0xF0000000) : (target & 0x0FFFFFFF);
        pDragon->m250 = target;
        setYawInterpMode(pDragon);
    }
}

// 0608629a (A5) / 0607cc72 (A7)
void triggerBarrelRoll(s_dragonTaskWorkArea* pDragon)
{
    if (gDragonState->mC_dragonType == 8)
        return;

    pDragon->m25D = 2;
    pDragon->m258 = 0x18;
    if (pDragon->m25E == 0)
        pDragon->m254 = 0x84BDA1;
    else
        pDragon->m254 = (s32)0xFF7B425F;
    pDragon->m23C |= 4;
    pDragon->m244 = 9;
}

// 060863c4 (A5) / 0607cd9c (A7)
void dragonAnalogSpecialInput(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];
    s8 analogX = input.m2_analogX;

    pDragon->m258--;
    if ((s32)pDragon->m258 >= 1)
        return;

    pDragon->m25D = 0;
    pDragon->m258 = 0;

    if ((config[0xF] & input.m8_newButtonDown) == 0)
        return;

    if ((config[0] & input.m6_buttonDown) != 0)
    {
        triggerBarrelRoll(pDragon);
        return;
    }

    if (pDragon->m235_dragonSpeedIndex != 0)
        return;

    if (analogX < 0)
    {
        s32 target = pDragon->m20_angle.m4_Y.m_value + (s32)0xF838E38F;
        target = (target & 0x8000000) ? (target | (s32)0xF0000000) : (target & 0x0FFFFFFF);
        pDragon->m250 = target;
        setYawInterpMode(pDragon);
    }
    else if (analogX > 0)
    {
        s32 target = pDragon->m20_angle.m4_Y.m_value + 0x07C71C71;
        target = (target & 0x8000000) ? (target | (s32)0xF0000000) : (target & 0x0FFFFFFF);
        pDragon->m250 = target;
        setYawInterpMode(pDragon);
    }
}

// 06086d60 (A5) / 0607d738 (A7)
void dragonAnimFromAnalog(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    s32 rate = pDragon->m184_animRate.m_value;

    s32 analogY;
    if (graphicEngineStatus.m4514.m138[1] == 0)
        analogY = (s32)input.m3_analogY;
    else
        analogY = -(s32)input.m3_analogY;

    s32 yDelta = (s32)pDragon->m246_previousAnalogY - analogY;
    if (yDelta > 0x40)
        incrementAnimationRootY(&gDragonState->m78_animData, performDivision(0x80, yDelta * rate));
    else if (yDelta < -0x40)
        incrementAnimationRootY(&gDragonState->m78_animData, performDivision(0x80, yDelta * rate));

    s32 xDelta = (s32)input.m2_analogX - (s32)pDragon->m245_previousAnalogX;
    if (xDelta > 0x40 || xDelta < -0x40)
    {
        incrementAnimationRootX(&gDragonState->m78_animData, performDivision(0x80, -(rate * xDelta)));
        incrementAnimationRootZ(&gDragonState->m78_animData, performDivision(0x80, rate * xDelta));
    }
}

// 060867cc (A5) / 0607d1a4 (A7)
void dragonDPadPitchRoll(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];
    fixedPoint turnRate = pDragon->m178_turnRate;

    dragonSpecialInput(pDragon);

    s32 absSpeed = pDragon->m154_dragonSpeed.m_value;
    if (absSpeed < 0) absSpeed = -absSpeed;

    // Pitch
    if (absSpeed < 0x100)
    {
        if ((config[5] & input.m6_buttonDown) != 0)
        {
            pDragon->m160_deltaTranslation.m4_Y = fixedPoint(pDragon->m160_deltaTranslation.m4_Y.m_value - 0x800);
            pDragon->m238 |= 2;
            pDragon->m1F0.m_8 = turnRate.m_value;
            pDragon->mFC |= 2;
        }
        else if ((config[4] & input.m6_buttonDown) != 0)
        {
            pDragon->m160_deltaTranslation.m4_Y = fixedPoint(pDragon->m160_deltaTranslation.m4_Y.m_value + 0x800);
            pDragon->m238 |= 1;
            pDragon->m1F0.m_8 = -turnRate.m_value;
            pDragon->mFC |= 1;
        }
        pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
    }
    else
    {
        if ((config[5] & input.m6_buttonDown) != 0)
        {
            pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value + turnRate.m_value);
            pDragon->m238 |= 2;
            pDragon->m1F0.m_8 = turnRate.m_value;
            pDragon->mFC |= 2;
        }
        else if ((config[4] & input.m6_buttonDown) != 0)
        {
            pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value - turnRate.m_value);
            pDragon->m238 |= 1;
            pDragon->m1F0.m_8 = -turnRate.m_value;
            pDragon->mFC |= 1;
        }
        else
        {
            pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
        }
    }

    // Clamp pitch
    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    // Roll + Yaw
    if (pDragon->m25D == 2)
    {
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + pDragon->m254);
    }
    else
    {
        pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

        if (pDragon->m25D == 1)
        {
            interpYawWithBanking(pDragon);
        }
        else
        {
            if ((config[7] & input.m6_buttonDown) != 0)
            {
                if ((pDragon->mF8_Flags & 0x8000) == 0)
                    pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value - turnRate.m_value);
                pDragon->m1F0.m_C = -turnRate.m_value;
                if (turnRate.m_value <= pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value)
                {
                    s32 bankAmount = performDivision(4, turnRate.m_value * 3);
                    pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + bankAmount);
                }
                pDragon->mFC |= 8;
                pDragon->m25E = 0;
            }
            else if ((config[6] & input.m6_buttonDown) != 0)
            {
                if ((pDragon->mF8_Flags & 0x8000) == 0)
                    pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + turnRate.m_value);
                pDragon->m1F0.m_C = turnRate.m_value;
                if (turnRate.m_value <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
                {
                    s32 bankAmount = performDivision(4, turnRate.m_value * 3);
                    pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - bankAmount);
                }
                pDragon->mFC |= 4;
                pDragon->m25E = 1;
            }
        }
    }

    pDragon->m247 = 0;
    pDragon->m246_previousAnalogY = 0;
    pDragon->m245_previousAnalogX = 0;
}

// 06086ea4 (A5) / 0607d87c (A7)
void dragonAnalogPitchRoll(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    fixedPoint turnRate = pDragon->m178_turnRate;

    s32 analogY;
    if (graphicEngineStatus.m4514.m138[1] == 0)
        analogY = (s32)input.m3_analogY;
    else
        analogY = -(s32)input.m3_analogY;
    s32 negAnalogY = -analogY;

    dragonAnalogSpecialInput(pDragon);

    s32 absSpeed = pDragon->m154_dragonSpeed.m_value;
    if (absSpeed < 0) absSpeed = -absSpeed;

    // Pitch
    if (absSpeed < 0x100)
    {
        s32 yTranslation = performDivision(0x7F, analogY * 0x800);
        if (negAnalogY > 0)
        {
            pDragon->m160_deltaTranslation.m4_Y = fixedPoint(pDragon->m160_deltaTranslation.m4_Y.m_value + yTranslation);
            pDragon->m238 |= 2;
            pDragon->m1F0.m_8 = performDivision(0x7F, turnRate.m_value * negAnalogY);
            pDragon->mFC |= 2;
        }
        else if (negAnalogY < 0)
        {
            pDragon->m160_deltaTranslation.m4_Y = fixedPoint(pDragon->m160_deltaTranslation.m4_Y.m_value + yTranslation);
            pDragon->m238 |= 1;
            pDragon->m1F0.m_8 = performDivision(0x7F, turnRate.m_value * negAnalogY);
            pDragon->mFC |= 1;
        }
        pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
    }
    else
    {
        if (negAnalogY > 0)
        {
            pDragon->m1F0.m_8 = performDivision(0x7F, turnRate.m_value * negAnalogY);
            pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value + pDragon->m1F0.m_8);
            pDragon->m238 |= 2;
            pDragon->mFC |= 2;
        }
        else if (negAnalogY < 0)
        {
            pDragon->m1F0.m_8 = performDivision(0x7F, turnRate.m_value * negAnalogY);
            pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value + pDragon->m1F0.m_8);
            pDragon->m238 |= 1;
            pDragon->mFC |= 1;
        }
        else
        {
            pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
        }
    }

    // Clamp pitch
    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = (pitch & 0x8000000) ? (pitch | (s32)0xF0000000) : (pitch & 0x0FFFFFFF);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    // Yaw
    s32 analogX = (s32)input.m2_analogX;
    pDragon->m1F0.m_C = performDivision(0x7F, turnRate.m_value * analogX);

    if (pDragon->m25D == 1)
    {
        interpYawWithBanking(pDragon);
    }
    else if ((pDragon->mF8_Flags & 0x8000) == 0)
    {
        pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + pDragon->m1F0.m_C);
    }

    // Roll
    if (pDragon->m25D == 2)
    {
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + pDragon->m254);
    }
    else
    {
        pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

        if (analogX > 0)
        {
            s32 scaledX = performDivision(0x7F, turnRate.m_value * analogX);
            if (scaledX <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
            {
                s32 bankAmount = performDivision(4, turnRate.m_value * 3);
                pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - bankAmount);
                pDragon->mFC |= 4;
                pDragon->m25E = 1;
            }
        }
        else if (analogX < 0)
        {
            s32 scaledX = performDivision(0x7F, -(turnRate.m_value * analogX));
            if (scaledX <= pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value)
            {
                s32 bankAmount = performDivision(4, turnRate.m_value * 3);
                pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + bankAmount);
                pDragon->mFC |= 8;
                pDragon->m25E = 0;
            }
        }
    }

    pDragon->m245_previousAnalogX = input.m2_analogX;
    s8 storedY = input.m3_analogY;
    if (graphicEngineStatus.m4514.m138[1] != 0)
        storedY = -input.m3_analogY;
    pDragon->m246_previousAnalogY = storedY;
    pDragon->m247 = input.m4;
}

// 060879c0 (A5) / 0607e398 (A7)
void dragonUpdate_normalTail(s_dragonTaskWorkArea* pDragon)
{
    getFieldTaskPtr()->m28_status &= ~0x10000;

    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    if (isDragonScriptBlocked() != 0)
    {
        clearDragonVelocity();
    }

    if (isDragonFreeFlying() == 0)
    {
        s32 speed = pDragon->m154_dragonSpeed.m_value;
        s32 decel;
        if (speed < 0)
            decel = -MTH_Mul(fixedPoint(speed), pDragon->m230).m_value;
        else
            decel = MTH_Mul(fixedPoint(-speed), pDragon->m230).m_value;
        pDragon->m15C_dragonSpeedIncrement = decel;
    }
    else
    {
        if ((pDragon->mF8_Flags & 0x10000) == 0)
        {
            dragonSpeedControl(pDragon);
        }

        if (pDragon->m235_dragonSpeedIndex < 0)
        {
            pDragon->m15C_dragonSpeedIncrement = MTH_Mul(
                fixedPoint(-0x1284 - pDragon->m154_dragonSpeed.m_value), pDragon->m230).m_value;
        }
        else
        {
            s32 targetSpeed;
            if (pDragon->m25C & 1)
                targetSpeed = pDragon->m158.m_value;
            else
                targetSpeed = pDragon->m21C_DragonSpeedValues[pDragon->m235_dragonSpeedIndex].m_value;

            s32 diff = pDragon->m154_dragonSpeed.m_value - targetSpeed;
            if (diff < 0)
                pDragon->m15C_dragonSpeedIncrement = -MTH_Mul(fixedPoint(diff), pDragon->m230).m_value;
            else
                pDragon->m15C_dragonSpeedIncrement = MTH_Mul(fixedPoint(-diff), pDragon->m230).m_value;
        }
    }

    pDragon->m154_dragonSpeed = fixedPoint(pDragon->m154_dragonSpeed.m_value + pDragon->m15C_dragonSpeedIncrement);

    if (keyboardIsKeyDown(0xA9))
    {
        pDragon->m154_dragonSpeed = fixedPoint(pDragon->m154_dragonSpeed.m_value << 2);
    }

    if (pDragon->m154_dragonSpeed.m_value < -0x7000)
        pDragon->m154_dragonSpeed = fixedPoint(-0x7000);
    else if (pDragon->m154_dragonSpeed.m_value > 0x7000)
        pDragon->m154_dragonSpeed = fixedPoint(0x7000);

    // Dampen rotation velocity
    pDragon->m1AC.m0_X = interpolateRotation(pDragon->m1AC.m0_X, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);
    pDragon->m1AC.m4_Y = interpolateRotation(pDragon->m1AC.m4_Y, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);
    pDragon->m1AC.m8_Z = interpolateRotation(pDragon->m1AC.m8_Z, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);

    pDragon->m1AC.m0_X = fixedPoint(pDragon->m1AC.m0_X.m_value + pDragon->m1A0.m0_X.m_value);
    pDragon->m1AC.m4_Y = fixedPoint(pDragon->m1AC.m4_Y.m_value + pDragon->m1A0.m4_Y.m_value);
    pDragon->m1AC.m8_Z = fixedPoint(pDragon->m1AC.m8_Z.m_value + pDragon->m1A0.m8_Z.m_value);

    pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value + pDragon->m1AC.m0_X.m_value);
    pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + pDragon->m1AC.m4_Y.m_value);
    pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + pDragon->m1AC.m8_Z.m_value);

    pDragon->m1A0 = {};

    // Dampen position velocity
    pDragon->m194.m0_X = interpolateDistance(pDragon->m194.m0_X, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);
    pDragon->m194.m4_Y = interpolateDistance(pDragon->m194.m4_Y, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);
    pDragon->m194.m8_Z = interpolateDistance(pDragon->m194.m8_Z, fixedPoint(0), fixedPoint(0x2000), fixedPoint(0x444444), 0);

    pDragon->m194.m0_X = fixedPoint(pDragon->m194.m0_X.m_value + pDragon->m188.m0_X.m_value);
    pDragon->m194.m4_Y = fixedPoint(pDragon->m194.m4_Y.m_value + pDragon->m188.m4_Y.m_value);
    pDragon->m194.m8_Z = fixedPoint(pDragon->m194.m8_Z.m_value + pDragon->m188.m8_Z.m_value);

    if (pDragon->m154_dragonSpeed.m_value >= 0)
    {
        MTH_Mul(fixedPoint(pDragon->m88_matrix.m[2][0].m_value), pDragon->m154_dragonSpeed);
    }
    else
    {
        MTH_Mul(fixedPoint(pDragon->m88_matrix.m[2][0].m_value), pDragon->m154_dragonSpeed);
    }
}

// ============================================================================
// Dragon update state machine
// ============================================================================

// 060878ac (A5) / 0607fcb8 (A3) — dragon update: idle (no camera script, no cutscene)
void dragonUpdate_idle(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m24A_runningCameraScript = 5;

    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    pLCS->m8 |= 1;

    getFieldTaskPtr()->m28_status |= 0x10000;

    pDragon->m160_deltaTranslation.m0_X = 0;
    pDragon->m160_deltaTranslation.m4_Y = 0;
    pDragon->m160_deltaTranslation.m8_Z = 0;

    if (pDragon->mF4 != nullptr)
    {
        pDragon->mF4(pDragon);
    }

    pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
    pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    computeDragonSpeed(pDragon);
}

// 06087e70 (A5) / 0608027c (A3) — dragon update: normal gameplay (default morph)
void dragonUpdate_normal(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m24A_runningCameraScript = 0;

    if (pDragon->m104_dragonScriptStatus == 0)
    {
        pDragon->m154_dragonSpeed = 0;
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~1;
        pDragon->mF8_Flags |= 0x400;
        pDragon->mF8_Flags &= ~0x20000;
        pDragon->m104_dragonScriptStatus++;
    }
    else if (pDragon->m104_dragonScriptStatus != 1)
    {
        goto inputPhase;
    }

    if (checkDragonTransition(pDragon) == 0)
    {
        pDragon->mF0 = &dragonUpdate_corridorAutopilot;
        pDragon->m104_dragonScriptStatus = 0;
        dragonUpdate_corridorAutopilot(pDragon);
        return;
    }

inputPhase:
    pDragon->m160_deltaTranslation = {};
    pDragon->m238 &= 0xFC;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 1)
        dragonInput_digital(pDragon);
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        dragonInput_analog(pDragon);
    else
        dragonInput_default(pDragon);

    dragonUpdate_normalTail(pDragon);
}

// 06087f4c (A5) / 06080358 (A3) — dragon update: normal gameplay (dragonType == 8)
void dragonUpdate_normal_type8(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m24A_runningCameraScript = 0;

    if (pDragon->m104_dragonScriptStatus == 0)
    {
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~1;
        pDragon->mF8_Flags |= 0x400;
        pDragon->mF8_Flags &= ~0x20000;
        pDragon->m104_dragonScriptStatus++;
    }

    pDragon->m160_deltaTranslation = {};
    pDragon->m238 &= 0xFC;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 1)
        dragonInput_digital_type8(pDragon);
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        dragonInput_analog_type8(pDragon);
    else
        dragonInput_default(pDragon);

    dragonUpdate_normalTail(pDragon);
}

// 0608604a (A5) — set dragon animation state based on current speed vs mid-speed threshold
void setDragonAnimationFromSpeed(s_dragonTaskWorkArea* pDragon)
{
    // Compute mid-speed as average of first two speed values
    s32 midSpeed = (s32)((u32)(pDragon->m21C_DragonSpeedValues[0].m_value
                              + pDragon->m21C_DragonSpeedValues[1].m_value
                              + (u32)(pDragon->m21C_DragonSpeedValues[0].m_value
                                    + pDragon->m21C_DragonSpeedValues[1].m_value < 0)) >> 1);

    if (pDragon->m154_dragonSpeed.m_value > midSpeed)
    {
        // Fast: set animation to "fast flight" unless already in that state
        if (pDragon->m23A_dragonAnimation == 0 || pDragon->m23A_dragonAnimation == 2)
            return;
        pDragon->m238 = 4;
        pDragon->m237 = 4;
        pDragon->m244 = 0;
    }
    else
    {
        // Slow: set animation to "slow flight" unless already there
        if (pDragon->m23A_dragonAnimation == 5)
            return;
        pDragon->m238 = 0;
        pDragon->m237 = 0;
        pDragon->m244 = 5;
    }
    pDragon->m23C |= 5;
}

// 06088008 (A5) / 06080414 (A3) — process camera script waypoints (3-state machine)
void processCameraScript(s_dragonTaskWorkArea* pDragon, s_cameraScript* pScript)
{
    pDragon->m24A_runningCameraScript = 1;

    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    pLCS->m8 |= 1;

    s32 state = pDragon->m104_dragonScriptStatus;
    if (state == 0)
    {
        // State 0: initialize from script data
        updateCameraScriptSub0((p_workArea)pDragon->mB8_lightWingEffect);
        activateCameraFollowMode(0);
        pDragon->mF8_Flags &= ~0x400;
        pDragon->mF8_Flags |= 0x20000;

        pDragon->m1E8_cameraScriptDelay = (s16)pScript->m20_length;

        // Set dragon angles from script rotation
        pDragon->m20_angle = pScript->mC_rotation;

        // Set dragon position from script position
        pDragon->m8_pos = pScript->m0_position;

        // Compute delta translation from script speed + yaw angle
        u16 yawIdx = (u16)((u32)pDragon->m20_angle.m4_Y.m_value >> 16) & 0xFFF;
        pDragon->m160_deltaTranslation.m0_X = MTH_Mul(fixedPoint(-pScript->m1C), getSin(yawIdx));
        pDragon->m160_deltaTranslation.m4_Y = fixedPoint(performDivision(
            (s32)pDragon->m1E8_cameraScriptDelay, pScript->m18.m_value - pDragon->m8_pos.m4_Y.m_value));
        pDragon->m160_deltaTranslation.m8_Z = MTH_Mul(fixedPoint(-pScript->m1C), getCos(yawIdx));

        computeDragonSpeed(pDragon);
        setDragonAnimationFromSpeed(pDragon);

        // Set camera position from script's secondary position
        sFieldCameraStatus* pCamSlot = getActiveCameraSlot();
        pCamSlot->m0_position = pScript->m24_pos2;
        pCamSlot->m88 = (s32)pDragon->m1E8_cameraScriptDelay;

        pDragon->m104_dragonScriptStatus++;
    }
    else if (state == 1)
    {
        // State 1: count down, then switch to follow mode
        pDragon->m1E8_cameraScriptDelay--;
        if (pDragon->m1E8_cameraScriptDelay < 1)
        {
            sFieldCameraStatus* pCamSlot = getActiveCameraSlot();
            pCamSlot->m88 = 0x1E;
            pDragon->m1E8_cameraScriptDelay = 0x1E;

            sFieldCameraManager* pCam = getFieldTaskPtr()->m8_pSubFieldData->m334;
            activateCameraFollowMode((u32)(s8)pCam->m50E_followModeIndex);

            pDragon->m104_dragonScriptStatus++;
        }
    }
    else if (state == 2)
    {
        // State 2: count down remaining, then clear script and transition
        pDragon->m1E8_cameraScriptDelay--;
        if (pDragon->m1E8_cameraScriptDelay < 1)
        {
            pDragon->m1D0_cameraScript = nullptr;
            dragonTransitionFromScript();
        }
    }
}

// 060881b2 (A5) / 060805be (A3) — process cutscene movement data (init only)
void processCutscene(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m24A_runningCameraScript = 2;

    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    pLCS->m8 |= 1;

    if (pDragon->m104_dragonScriptStatus == 0)
    {
        updateCameraScriptSub0((p_workArea)pDragon->mB8_lightWingEffect);
        pDragon->mF8_Flags &= ~0x400;
        pDragon->mF8_Flags |= 0x20000;
        computeDragonSpeed(pDragon);
        setDragonAnimationFromSpeed(pDragon);
        pDragon->m104_dragonScriptStatus++;
    }
}

// 0608820e (A5) / 0608061a (A3) — dragon update: camera script active
void dragonUpdate_cameraScript(s_dragonTaskWorkArea* pDragon)
{
    getFieldTaskPtr()->m28_status |= 0x10000;

    if (pDragon->m1D4_cutsceneData != nullptr)
    {
        processCutscene(pDragon);
    }
    else if (pDragon->m1D0_cameraScript != nullptr)
    {
        processCameraScript(pDragon, pDragon->m1D0_cameraScript);
    }
    else
    {
        sFieldCameraManager* pCam = getFieldTaskPtr()->m8_pSubFieldData->m334;
        // On Saturn, calls overlay-local startCameraFollowMode which dispatches
        // through the follow mode tables. In C++, we call the A3 implementation
        // which uses activateCameraFollowModeTable1/2 (identical across overlays).
        extern void startCameraFollowModeByIndex(s32 followMode);
        startCameraFollowModeByIndex((s32)pCam->m50E_followModeIndex);
        dragonTransitionFromScript();
    }

    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    pDragon->m8_pos.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value + pDragon->m160_deltaTranslation.m0_X.m_value);
    pDragon->m8_pos.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value + pDragon->m160_deltaTranslation.m4_Y.m_value);
    pDragon->m8_pos.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value + pDragon->m160_deltaTranslation.m8_Z.m_value);

    computeDragonSpeed(pDragon);
}

// Helper: sign-extend 28-bit angle difference
static inline s32 signExtend28(s32 val)
{
    return (val & 0x8000000) ? (val | (s32)0xF0000000) : (val & 0x0FFFFFFF);
}

// 0608838a (A5) / 06080796 (A3) — dragon update: cutscene keyframe interpolation
void dragonUpdate_cutscene(s_dragonTaskWorkArea* pDragon)
{
    s_scriptData3* pKeyFrame = pDragon->m1E4_cutsceneKeyFrame;

    pDragon->m24A_runningCameraScript = 4;

    // If no keyframe, skip to state 3 (idle)
    if (pKeyFrame == nullptr)
    {
        pDragon->m104_dragonScriptStatus = 3;
    }

    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    s32 state = pDragon->m104_dragonScriptStatus;

    if (state == 0)
    {
        // State 0: compute position and rotation velocities from keyframe
        pDragon->mF8_Flags &= ~0x400;
        pDragon->mF8_Flags |= 0x20000;

        s32 posDuration = pKeyFrame->m0_duration.m_value;
        s32 rotDuration = pKeyFrame->m10_rotationDuration.m_value;

        pDragon->m1E8_cameraScriptDelay = (s16)posDuration;
        pDragon->m1EA = (s16)rotDuration;

        // Position velocity = (target - current) / duration
        pDragon->m160_deltaTranslation.m0_X = fixedPoint(performDivision(posDuration,
            pKeyFrame->m4_pos.m0_X.m_value - pDragon->m8_pos.m0_X.m_value));
        pDragon->m160_deltaTranslation.m4_Y = fixedPoint(performDivision(posDuration,
            pKeyFrame->m4_pos.m4_Y.m_value - pDragon->m8_pos.m4_Y.m_value));
        pDragon->m160_deltaTranslation.m8_Z = fixedPoint(performDivision(posDuration,
            pKeyFrame->m4_pos.m8_Z.m_value - pDragon->m8_pos.m8_Z.m_value));

        // Rotation velocity = sign-extended 28-bit (target - current) / rotDuration
        if (rotDuration == 0)
        {
            pDragon->m16C_deltaRotation = {};
        }
        else
        {
            pDragon->m16C_deltaRotation.m0_X = fixedPoint(performDivision(rotDuration,
                signExtend28(pKeyFrame->m14_rot.m0_X.m_value - pDragon->m20_angle.m0_X.m_value)));
            pDragon->m16C_deltaRotation.m4_Y = fixedPoint(performDivision(rotDuration,
                signExtend28(pKeyFrame->m14_rot.m4_Y.m_value - pDragon->m20_angle.m4_Y.m_value)));
            pDragon->m16C_deltaRotation.m8_Z = fixedPoint(performDivision(rotDuration,
                signExtend28(pKeyFrame->m14_rot.m8_Z.m_value - pDragon->m20_angle.m8_Z.m_value)));
        }

        computeDragonSpeed(pDragon);

        if ((pDragon->mF8_Flags & 0x40000) != 0)
        {
            setDragonAnimationFromSpeed(pDragon);
            updateCameraScriptSub0((p_workArea)pDragon->mB8_lightWingEffect);
            pDragon->mF8_Flags &= ~0x40000;
        }

        pDragon->m104_dragonScriptStatus++;
        // Fall through to state 1
    }
    else if (state == 2)
    {
        // State 2: position-only interpolation (rotation done)
        pDragon->m8_pos.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value + pDragon->m160_deltaTranslation.m0_X.m_value);
        pDragon->m8_pos.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value + pDragon->m160_deltaTranslation.m4_Y.m_value);
        pDragon->m8_pos.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value + pDragon->m160_deltaTranslation.m8_Z.m_value);

        pDragon->m1E8_cameraScriptDelay--;
        if (pDragon->m1E8_cameraScriptDelay < 1)
        {
            pDragon->m104_dragonScriptStatus = 3;
            pDragon->m1E4_cutsceneKeyFrame = nullptr;
        }
        goto buildMatrix;
    }
    else if (state != 1)
    {
        goto buildMatrix;
    }

    // States 0 (after init) and 1: both position and rotation interpolation
    pDragon->m20_angle.m0_X = fixedPoint(pDragon->m20_angle.m0_X.m_value + pDragon->m16C_deltaRotation.m0_X.m_value);
    pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + pDragon->m16C_deltaRotation.m4_Y.m_value);
    pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + pDragon->m16C_deltaRotation.m8_Z.m_value);

    pDragon->m8_pos.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value + pDragon->m160_deltaTranslation.m0_X.m_value);
    pDragon->m8_pos.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value + pDragon->m160_deltaTranslation.m4_Y.m_value);
    pDragon->m8_pos.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value + pDragon->m160_deltaTranslation.m8_Z.m_value);

    // Count down rotation duration
    pDragon->m1EA--;
    if (pDragon->m1EA < 1)
    {
        pDragon->m104_dragonScriptStatus++;
    }

    // Count down position duration
    pDragon->m1E8_cameraScriptDelay--;
    if (pDragon->m1E8_cameraScriptDelay < 1)
    {
        pDragon->m104_dragonScriptStatus = 3;
        pDragon->m1E4_cutsceneKeyFrame = nullptr;
    }

buildMatrix:
    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    pDragon->m1EE--;
    if (pDragon->m1EE < 0)
    {
        pDragon->m1EE = 0;
    }

    computeDragonSpeed(pDragon);
}

// 06088454 (A5) / 06080860 (A3) — transition dragon to normal flight update
void dragonTransitionToNormal()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (gDragonState->mC_dragonType == 8)
        pDragon->mF0 = &dragonUpdate_normal_type8;
    else
        pDragon->mF0 = &dragonUpdate_normal;
    pDragon->m104_dragonScriptStatus = 0;
    clearDragonScriptFlag();
}

// 0607c88e (A5) / 06074c9a (A3) — transition dragon from script/cutscene
// Byte-identical to dragonTransitionToNormal (same Saturn function at different address)
void dragonTransitionFromScript()
{
    dragonTransitionToNormal();
}

// 0608746e (A5) / 0607f87a (A3) — check if dragon is within safe bounds
s32 checkDragonTransition(s_dragonTaskWorkArea* pDragon)
{
    s32 posX = pDragon->m8_pos.m0_X.m_value;
    s32 posZ = pDragon->m8_pos.m8_Z.m_value;

    if (pDragon->m130_minX.m_value + 0x20000 < posX &&
        posX < pDragon->m13C_maxX.m_value - 0x20000 &&
        pDragon->m138_minZ.m_value + 0x20000 < posZ &&
        posZ < pDragon->m144_maxZ.m_value - 0x20000)
    {
        return 1;
    }
    return 0;
}

// 060874c8 (A5) / 0607f8d4 (A3) — compute delta translation to push dragon inside bounds
void computeBoundsPushback(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m160_deltaTranslation.m0_X = 0;
    pDragon->m160_deltaTranslation.m4_Y = 0;
    pDragon->m160_deltaTranslation.m8_Z = 0;

    s32 dxMin = (pDragon->m130_minX.m_value - pDragon->m8_pos.m0_X.m_value) + 0x60000;
    if (dxMin > 0)
        pDragon->m160_deltaTranslation.m0_X = performDivision(pDragon->m_1C4, dxMin);
    else
    {
        s32 dxMax = (pDragon->m13C_maxX.m_value - pDragon->m8_pos.m0_X.m_value) - 0x60000;
        if (dxMax < 0)
            pDragon->m160_deltaTranslation.m0_X = performDivision(pDragon->m_1C4, dxMax);
    }

    s32 dzMin = (pDragon->m138_minZ.m_value - pDragon->m8_pos.m8_Z.m_value) + 0x60000;
    if (dzMin > 0)
        pDragon->m160_deltaTranslation.m8_Z = performDivision(pDragon->m_1C4, dzMin);
    else
    {
        s32 dzMax = (pDragon->m144_maxZ.m_value - pDragon->m8_pos.m8_Z.m_value) - 0x60000;
        if (dzMax < 0)
            pDragon->m160_deltaTranslation.m8_Z = performDivision(pDragon->m_1C4, dzMax);
    }
}

// 06087544 (A5) / 0607f950 (A3) — corridor autopilot dragon update.
// Steers the dragon toward the visibility grid target, pushes back inside bounds.
// 3-state: 0=trigger script+countdown, 1=countdown, 2=wait for multichoice close.
void dragonUpdate_corridorAutopilot(s_dragonTaskWorkArea* pDragon)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    fixedPoint turnRate = pDragon->m178_turnRate;

    pDragon->m24A_runningCameraScript = 6;
    getFieldTaskPtr()->m28_status |= 0x200;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    s32 state = pDragon->m104_dragonScriptStatus;
    if (state == 0)
    {
        corridorEntryScript();
        setDragonSpeedIndex(0);
        pDragon->m_1C4 = 0x2D;
        pDragon->m104_dragonScriptStatus++;
    }
    else if (state == 1)
    {
        // Count down
    }
    else if (state == 2)
    {
        // Wait for multichoice to clear
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m3C_multichoiceTask == nullptr)
        {
            dragonTransitionFromScript();
            pDragon->mF0(pDragon);
            getFieldTaskPtr()->m28_status &= ~0x200;
            return;
        }
        goto steering;
    }
    else
    {
        goto steering;
    }

    // States 0 and 1: decrement timer
    pDragon->m_1C4--;
    if ((s32)pDragon->m_1C4 < 1)
    {
        pDragon->m_1C4 = 1;
        pDragon->m104_dragonScriptStatus++;
    }

steering:
    pDragon->m238 &= 0xFC;

    // Compute direction to grid target
    sVec3_FP toTarget;
    toTarget.m0_X = fixedPoint(pGrid->m1294.mC - pDragon->m8_pos.m0_X.m_value);
    toTarget.m4_Y = 0;
    toTarget.m8_Z = fixedPoint(pGrid->m1294.m14 - pDragon->m8_pos.m8_Z.m_value);

    sVec2_FP lookAt;
    computeLookAt(toTarget, lookAt);

    // Interpolate pitch toward target (15/16 damping)
    pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);

    // Interpolate yaw toward look-at direction
    pDragon->m20_angle.m4_Y = interpolateRotation(pDragon->m20_angle.m4_Y, lookAt[1], fixedPoint(0x2000), fixedPoint(0x444444), 0);

    // Interpolate roll toward target
    pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

    // Roll banking from yaw delta
    s32 yawDelta = pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value;
    if (yawDelta < turnRate.m_value)
    {
        if (turnRate.m_value <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
        {
            s32 bankAmount = performDivision(4, turnRate.m_value * 3);
            pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - bankAmount);
        }
    }
    else
    {
        s32 bankAmount = performDivision(4, turnRate.m_value * 3);
        pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + bankAmount);
    }

    // Clamp pitch
    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);
    copyMatrix(&pDragon->m48.m0_matrix, &pDragon->m88_matrix);

    // Zero speed
    pDragon->m15C_dragonSpeedIncrement = 0;
    pDragon->m154_dragonSpeed = 0;

    // Compute bounds pushback and integrate position
    computeBoundsPushback(pDragon);

    pDragon->m8_pos.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value + pDragon->m160_deltaTranslation.m0_X.m_value);
    pDragon->m8_pos.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value + pDragon->m160_deltaTranslation.m4_Y.m_value);
    pDragon->m8_pos.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value + pDragon->m160_deltaTranslation.m8_Z.m_value);

    // Y clamp and dynamic pitch limits
    if (pDragon->m134_minY.m_value != 0 || pDragon->m140_maxY.m_value != 0)
    {
        if (pDragon->m8_pos.m4_Y.m_value < pDragon->m134_minY.m_value)
            pDragon->m8_pos.m4_Y = pDragon->m134_minY;
        if (pDragon->m140_maxY.m_value < pDragon->m8_pos.m4_Y.m_value)
            pDragon->m8_pos.m4_Y = pDragon->m140_maxY;

        pDragon->m160_deltaTranslation.m0_X = fixedPoint(pDragon->m8_pos.m0_X.m_value - pDragon->m14_oldPos.m0_X.m_value);
        pDragon->m160_deltaTranslation.m4_Y = fixedPoint(pDragon->m8_pos.m4_Y.m_value - pDragon->m14_oldPos.m4_Y.m_value);
        pDragon->m160_deltaTranslation.m8_Z = fixedPoint(pDragon->m8_pos.m8_Z.m_value - pDragon->m14_oldPos.m8_Z.m_value);

        // Dynamic pitch min from distance to maxY ceiling
        s32 distToCeiling = (pDragon->m140_maxY.m_value - pDragon->m8_pos.m4_Y.m_value) * -0x111;
        s32 pitchMin = -0x3555555;
        if (distToCeiling > -0x3555555) pitchMin = distToCeiling;
        if (pitchMin >= 1) pitchMin = 0;
        else if (distToCeiling <= -0x3555555) pitchMin = -0x3555555;
        pDragon->m148_pitchMin = fixedPoint(pitchMin);

        // Dynamic pitch max from distance to minY floor
        s32 distToFloor = (pDragon->m8_pos.m4_Y.m_value - pDragon->m134_minY.m_value) * 0x111;
        s32 pitchMax = 0x3555555;
        if (distToFloor < 0x3555555) pitchMax = distToFloor;
        if (pitchMax < 1) pitchMax = 0;
        else if (distToFloor >= 0x3555555) pitchMax = 0x3555555;
        pDragon->m14C_pitchMax = fixedPoint(pitchMax);
    }
}

// 06086cfe (A5) / 0607f10a (A3) — dragon input: digital pad
void dragonInput_digital(s_dragonTaskWorkArea* pDragon)
{
    if (canAcceptDragonInput() != 0)
    {
        dragonAnimFromDPad(pDragon);
        dragonDPadPitchRoll(pDragon);
    }
    else
    {
        dragonInput_default(pDragon);
    }
}

// 06087428 (A5) / 0607f834 (A3) — dragon input: analog stick
void dragonInput_analog(s_dragonTaskWorkArea* pDragon)
{
    if (canAcceptDragonInput() != 0)
    {
        dragonAnimFromAnalog(pDragon);
        dragonAnalogPitchRoll(pDragon);
    }
    else
    {
        dragonInput_default(pDragon);
    }
}

// 06086ade (A5) / 0607eeea (A3) — type 8 floater d-pad pitch/roll
static void dragonDPadPitchRoll_type8(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    auto& config = graphicEngineStatus.m4514.mD8_buttonConfig[1];
    s32 turnRate = pDragon->m178_turnRate.m_value;

    dragonSpecialInput(pDragon);

    s32 absSpeed = pDragon->m154_dragonSpeed.m_value;
    if (absSpeed < 0) absSpeed = -absSpeed;

    if (absSpeed > 0xFF)
    {
        if ((config[5] & input.m6_buttonDown) != 0)
        {
            pDragon->m1F0.m_8 = turnRate;
        }
        else if ((config[4] & input.m6_buttonDown) != 0)
        {
            pDragon->m1F0.m_8 = -turnRate;
        }
        else
        {
            pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
        }
    }

    // Clamp pitch
    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    // Interpolate pitch toward target
    pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);

    // Interpolate roll toward target
    pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

    // Yaw
    if (pDragon->m25D == 1)
    {
        interpYawWithBanking(pDragon);
    }
    else if ((config[7] & input.m6_buttonDown) != 0)
    {
        if ((pDragon->mF8_Flags & 0x8000) == 0)
            pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value - turnRate);
        pDragon->m1F0.m_C = -turnRate;
        if (turnRate <= pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value)
        {
            pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + turnRate);
        }
        pDragon->m25E = 0;
    }
    else if ((config[6] & input.m6_buttonDown) != 0)
    {
        if ((pDragon->mF8_Flags & 0x8000) == 0)
            pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + turnRate);
        pDragon->m1F0.m_C = turnRate;
        if (turnRate <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
        {
            pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - turnRate);
        }
        pDragon->m25E = 1;
    }

    pDragon->m247 = 0;
    pDragon->m246_previousAnalogY = 0;
    pDragon->m245_previousAnalogX = 0;
}

// 0608720c (A5) / 0607f618 (A3) — type 8 floater analog pitch/roll
static void dragonAnalogPitchRoll_type8(s_dragonTaskWorkArea* pDragon)
{
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current;
    s32 turnRate = pDragon->m178_turnRate.m_value;

    s32 analogY;
    if (graphicEngineStatus.m4514.m138[1] == 0)
        analogY = (s32)input.m3_analogY;
    else
        analogY = -(s32)input.m3_analogY;
    s32 negAnalogY = -analogY;

    dragonAnalogSpecialInput(pDragon);

    // Pitch
    if (negAnalogY > 0)
    {
        pDragon->m1F0.m_8 = performDivision(0x7F, turnRate * negAnalogY);
    }
    else if (negAnalogY < 0)
    {
        pDragon->m1F0.m_8 = performDivision(0x7F, turnRate * negAnalogY);
    }
    else
    {
        // No analog input: interpolate pitch toward target
        pDragon->m20_angle.m0_X = interpolateAngle28(pDragon->m20_angle.m0_X, pDragon->m3C_targetAngles.m0_X);
    }

    // Clamp pitch
    s32 pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch > pDragon->m14C_pitchMax.m_value)
        pDragon->m20_angle.m0_X = pDragon->m14C_pitchMax;
    pitch = pDragon->m20_angle.m0_X.m_value;
    pitch = signExtend28(pitch);
    if (pitch < pDragon->m148_pitchMin.m_value)
        pDragon->m20_angle.m0_X = pDragon->m148_pitchMin;

    // Yaw
    s32 analogX = (s32)input.m2_analogX;
    pDragon->m1F0.m_C = performDivision(0x7F, turnRate * analogX);

    if (pDragon->m25D == 1)
    {
        interpYawWithBanking(pDragon);
    }
    else if ((pDragon->mF8_Flags & 0x8000) == 0)
    {
        pDragon->m20_angle.m4_Y = fixedPoint(pDragon->m20_angle.m4_Y.m_value + pDragon->m1F0.m_C);
    }

    // Roll: interpolate toward target
    pDragon->m20_angle.m8_Z = interpolateAngle28(pDragon->m20_angle.m8_Z, pDragon->m3C_targetAngles.m8_Z);

    // Banking from analogX
    if (analogX > 0)
    {
        s32 scaledX = performDivision(0x7F, turnRate * analogX);
        if (scaledX <= pDragon->m20_angle.m4_Y.m_value - pDragon->m30.m_value)
        {
            pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value - turnRate);
        }
        pDragon->m25E = 1;
    }
    else if (analogX < 0)
    {
        s32 scaledX = performDivision(0x7F, -(turnRate * analogX));
        if (scaledX <= pDragon->m30.m_value - pDragon->m20_angle.m4_Y.m_value)
        {
            pDragon->m20_angle.m8_Z = fixedPoint(pDragon->m20_angle.m8_Z.m_value + turnRate);
        }
        pDragon->m25E = 0;
    }

    pDragon->m245_previousAnalogX = input.m2_analogX;
    s8 storedY = input.m3_analogY;
    if (graphicEngineStatus.m4514.m138[1] != 0)
        storedY = -input.m3_analogY;
    pDragon->m246_previousAnalogY = storedY;
    pDragon->m247 = input.m4;
}

// 06086d22 (A5) / 0607f12e (A3) — dragon input: digital pad (type 8 floater)
void dragonInput_digital_type8(s_dragonTaskWorkArea* pDragon)
{
    if (canAcceptDragonInput() != 0)
    {
        dragonDPadPitchRoll_type8(pDragon);
    }
    else
    {
        dragonInput_default(pDragon);
    }
}

// 0608744e (A5) / 0607f85a (A3) — dragon input: analog stick (type 8 floater)
void dragonInput_analog_type8(s_dragonTaskWorkArea* pDragon)
{
    if (canAcceptDragonInput() != 0)
    {
        dragonAnalogPitchRoll_type8(pDragon);
    }
    else
    {
        dragonInput_default(pDragon);
    }
}

// 060797e0 (A5) / 06071bec (A3) — trigger corridor entry script.
// On Saturn the script address is a PC-relative constant. In C++ we use
// gCorridorEntryScriptEA which each overlay sets during init.
sSaturnPtr gCorridorEntryScriptEA = {};

void corridorEntryScript()
{
    if (gCorridorEntryScriptEA.m_offset == 0)
    {
        Unimplemented();
        return;
    }

    if (queueNewFieldScript(gCorridorEntryScriptEA, -1) != 0)
    {
        createScriptWaitTask(0, &dispatchTutorialMultiChoiceSub2, 0);
    }
}

// 0607c8c2 (A5) / 06074cce (A3) — initDragonMovementMode
// Already implemented in o_fld_a3.cpp, declared in o_fld_a3.h

// 0607cf3a (A5) / 06075346 (A3) — set cutscene keyframe on dragon
void setCutsceneKeyFrame(s_scriptData3* pKeyFrame)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->m1E4_cutsceneKeyFrame = pKeyFrame;
    pDragon->m104_dragonScriptStatus = 0;
    pDragon->mF0 = &dragonUpdate_cutscene;
    pDragon->mF8_Flags &= ~0x400;
}

// 0607cf70 (A5) / 0607537c (A3) — clear cutscene keyframe
void clearCutsceneKeyFrame()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->m1E4_cutsceneKeyFrame = nullptr;
}

// 06079044 (A5) / 06071450 (A3) — enable script skipping and trigger field exit
void enableScriptSkippingAndExit(s32 fieldIndex, s32 param, s32 exitNumber, s16 r7)
{
    enableFieldScriptSkipping();
    exitCutsceneTaskUpdateSub0Sub1(fieldIndex, param, exitNumber, r7);
}
