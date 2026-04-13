#include "PDS.h"
#include "fieldDragonInput.h"
#include "field/field_a3/o_fld_a3.h"

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

// 06088008 (A5) / 06080414 (A3) — process camera script waypoints
void processCameraScript(s_dragonTaskWorkArea* pDragon, s_cameraScript* pScript)
{
    Unimplemented();
}

// 060881b2 (A5) / 060805be (A3) — process cutscene movement data
void processCutscene(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented();
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

// 0608838a (A5) / 06080796 (A3) — dragon update: cutscene active (keyframe interpolation)
void dragonUpdate_cutscene(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented();
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
void dragonTransitionFromScript()
{
    Unimplemented();
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

// 06087544 (A5) / 0607f950 (A3) — corridor autopilot dragon update
void dragonUpdate_corridorAutopilot(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented();
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

// 06086d22 (A5) / 0607f12e (A3) — dragon input: digital pad (type 8 floater)
void dragonInput_digital_type8(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented();
}

// 0608744e (A5) / 0607f85a (A3) — dragon input: analog stick (type 8 floater)
void dragonInput_analog_type8(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented();
}

// 060797e0 (A5) / 06071bec (A3) — trigger corridor entry script
void corridorEntryScript()
{
    Unimplemented();
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
