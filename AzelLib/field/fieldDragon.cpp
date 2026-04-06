#include "PDS.h"
#include "fieldDragon.h"
#include "field/dragonLightWingEvolution.h"
#include "field/fieldDragonMovement.h"
#include "field/fieldRadar.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "collision.h"
#include "audio/systemSounds.h"
#include "debugWindows.h"
#include "field/fieldVisibilityGrid.h"

// forward declarations for overlay-local functions
void dragonFieldTaskUpdateSub2(u32);
void updateFieldCameraSlots();
void drawFieldCameraSlots();
void startScriptLeaveArea();
void dragonFieldTaskInitSub4Sub4();
void updateCameraScriptSub0(p_workArea r4);
void activateCameraFollowMode(u32 r4);
void buildDragonRotationMatrix(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13);
fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);

// ============================================================
// Dragon helper functions (moved from field/field_a3/o_fld_a3.cpp)
// ============================================================

// 0600adb8
p_workArea createLightWingEffect(p_workArea parent, s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6)
{
    Unimplemented();
    return nullptr;
}

bool shouldLoadPup()
{
    if (mainGameState.getBit(0x29 * 8 + 0))
    {
        assert(0);
    }
    return false;
}

// ============================================================
// Dragon Rider Task
// ============================================================

struct s_DragonRiderTask : public s_workAreaTemplate<s_DragonRiderTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &dragonRidersTaskInit, &dragonRidersTaskUpdate, NULL, NULL };
        return &taskDefinition;
    }

    static void dragonRidersTaskInit(s_DragonRiderTask* pWorkArea);
    static void dragonRidersTaskUpdate(s_DragonRiderTask* pWorkArea);
};

// 0607b280
void s_DragonRiderTask::dragonRidersTaskInit(s_DragonRiderTask* pWorkArea)
{
    initAnimation(&pRider1State->m18_3dModel, pRider1State->m0_riderBundle->getAnimation(0x30));
    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);

    if (mainGameState.gameStats.m3_rider2 == 7)
    {
        initAnimation(&pRider2State->m18_3dModel, pRider2State->m0_riderBundle->getAnimation(0x30));
        updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
    }
}

void s_DragonRiderTask::dragonRidersTaskUpdate(s_DragonRiderTask* pWorkArea)
{
    PDS_unimplemented("dragonRidersTaskUpdate");
}

static void createDragonRiderTask(p_workArea parent)
{
    createSubTask<s_DragonRiderTask>(parent);
}

// ============================================================
// Dragon field state initialization
// ============================================================

void initDragonFieldStateSub2(s_dragonTaskWorkArea* pDragon)
{
    pDragon->m178_turnRate = 0x222222;
    pDragon->m17C = 0x4CCC;
    pDragon->m180 = 0x16;
    pDragon->m184_animRate = 0x111111;
}

void initDragonFieldStateSub3(s_dragonTaskWorkArea* pWorkArea)
{
    pWorkArea->m1F0.m_0 = 0;
    pWorkArea->m1F0.m_4 = 0;
    pWorkArea->m1F0.m_8 = 0;
    pWorkArea->m1F0.m_C = 0;
    pWorkArea->m1F0.m_E = 0;
    pWorkArea->m1F0.m_10 = 0;
}

static void initDragonFieldStateSub4(s_dragonTaskWorkArea_48* m48)
{
    initMatrixToIdentity(&m48->m0_matrix);

    m48->m30 = 0;
    m48->m34 = 0;
    m48->m38 = 0;

    m48->m3C = 1;
}

void initDragonFieldState(s_dragonTaskWorkArea* pWorkArea)
{
    createFieldRadar(pWorkArea);

    initDragonFieldStateSub2(pWorkArea);

    initDragonFieldStateSub3(pWorkArea);

    pWorkArea->m208 = 0x960000;
    pWorkArea->m20C = 0x960000;
    pWorkArea->m210 = 0xB333;
    pWorkArea->m214 = 0xB333;
    pWorkArea->m150 = 0x10000;

    pWorkArea->m8_pos[0] = 0;
    pWorkArea->m8_pos[1] = 0x1E000;
    pWorkArea->m8_pos[2] = 0;

    pWorkArea->m20_angle[0] = 0;
    pWorkArea->m20_angle[1] = 0;
    pWorkArea->m20_angle[2] = 0;

    pWorkArea->m154_dragonSpeed = 0;

    pWorkArea->m1B8 = 0xB333;
    pWorkArea->m1BC = 0x200000;

    initMatrixToIdentity(&pWorkArea->m88_matrix);

    initDragonFieldStateSub4(&pWorkArea->m48);

    pWorkArea->m1CC_fieldOfView = DEG_80; // field of view
    pWorkArea->m234 = 0;

    pWorkArea->m21C_DragonSpeedValues[0] = 0;
    pWorkArea->m21C_DragonSpeedValues[1] = 0x1284;
    pWorkArea->m21C_DragonSpeedValues[2] = 0x2509;
    pWorkArea->m21C_DragonSpeedValues[3] = 0x3B42;
    pWorkArea->m21C_DragonSpeedValues[4] = 0x58E3;

    initDragonSpeed(0);

    pWorkArea->m230 = 0x1999;

    //060738C0

    pWorkArea->m154_dragonSpeed = pWorkArea->m21C_DragonSpeedValues[pWorkArea->m235_dragonSpeedIndex];

    pWorkArea->m238 = 0;
    pWorkArea->m237 = 0;
    pWorkArea->mC0_lightRotationAroundDragon = 0xC000000;
    pWorkArea->mC4 = 0;

    pWorkArea->mC8_normalLightColor.m0 = 0x10;
    pWorkArea->mC8_normalLightColor.m1 = 0x10;
    pWorkArea->mC8_normalLightColor.m2 = 0x10;

    pWorkArea->mCB_falloffColor0.m0 = 0x8;
    pWorkArea->mCB_falloffColor0.m1 = 0x8;
    pWorkArea->mCB_falloffColor0.m2 = 0x8;

    pWorkArea->mCE_falloffColor1.m0 = 0x14;
    pWorkArea->mCE_falloffColor1.m1 = 0x14;
    pWorkArea->mCE_falloffColor1.m2 = 0x14;

    pWorkArea->mD1_falloffColor2.m0 = 0xC;
    pWorkArea->mD1_falloffColor2.m1 = 0xC;
    pWorkArea->mD1_falloffColor2.m2 = 0xC;

    pWorkArea->mD4.m0 = 0x10;
    pWorkArea->mD4.m1 = 0x10;
    pWorkArea->mD4.m2 = 0x10;

    sFieldCameraStatus* pFieldCameraStatus = getFieldCameraStatus();

    pFieldCameraStatus->mC_rotation[0] = pWorkArea->m20_angle[1];

    pWorkArea->m14C_pitchMax = 0x2AAAAAA;
    pWorkArea->m148_pitchMin = -0x2AAAAAA;
    pWorkArea->m140_maxY = 0x300000;
    pWorkArea->m134_minY = -0x300000;
    pWorkArea->m130_minX = 0x80000000;
    pWorkArea->m13C_maxX = 0x7FFFFFFF;
    pWorkArea->m138_minZ = 0x80000000;
    pWorkArea->m144_maxZ = 0x7FFFFFFF;
}

void initDragonFieldAnimation(s_dragonTaskWorkArea* pWorkArea, s_dragonState* pDragonState, int param2)
{
    setupModelAnimation(&pDragonState->m28_dragon3dModel, pDragonState->m0_pDragonModelBundle->getAnimation(pDragonState->m20_dragonAnimOffsets[param2]));
    updateAndInterpolateAnimation(&pDragonState->m28_dragon3dModel);

    pWorkArea->m23A_dragonAnimation = param2;
    pWorkArea->m237 = pWorkArea->m238;
    pWorkArea->m23B = 1;
}

s32 isDragonInFieldBounds(s_dragonTaskWorkArea* r4)
{
    if (r4->m8_pos[0] <= r4->m130_minX + 0x20000)
        return 0;
    if (r4->m8_pos[0] >= r4->m13C_maxX - 0x20000)
        return 0;
    if (r4->m8_pos[2] <= r4->m138_minZ + 0x20000)
        return 0;
    if (r4->m8_pos[2] >= r4->m144_maxZ - 0x20000)
        return 0;

    return 1;
}

// 0607f8d4
void computeDragonDeltaTranslation(s_dragonTaskWorkArea* r14)
{
    s32 margin = 0x60000;

    r14->m160_deltaTranslation.m0_X = 0;
    r14->m160_deltaTranslation.m4_Y = 0;
    r14->m160_deltaTranslation.m8_Z = 0;

    // X axis: push back if near min or max boundary
    s32 delta = ((s32)r14->m130_minX - (s32)r14->m8_pos.m0_X) + margin;
    if ((delta > 0) ||
        (delta = ((s32)r14->m13C_maxX - (s32)r14->m8_pos.m0_X) - margin, delta < 0)) {
        r14->m160_deltaTranslation.m0_X = delta / (s32)r14->m_1C4;
    }

    // Z axis: push back if near min or max boundary
    delta = ((s32)r14->m138_minZ - (s32)r14->m8_pos.m8_Z) + margin;
    if ((delta > 0) ||
        (delta = ((s32)r14->m144_maxZ - (s32)r14->m8_pos.m8_Z) - margin, delta < 0)) {
        r14->m160_deltaTranslation.m8_Z = delta / (s32)r14->m_1C4;
    }
}

// ============================================================
// Dragon movement — no input
// ============================================================

void updateDragonMovementNoInputSub1(s_dragonTaskWorkArea* r14)
{
    r14->m254 = 0;
    r14->m250 = 0;
    r14->m24A_runningCameraScript = 0;
    r14->m258 = 0;
    r14->m25C = 0;
    r14->m25D = 0;
}

void updateDragonMovementNoInput(s_dragonTaskWorkArea* r14)
{
    if (--r14->m25D < 0)
    {
        updateDragonMovementNoInputSub1(r14);
    }

    // update yaw
    {
        fixedPoint tempRotX = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, tempRotX.normalized() * 15) - r14->m20_angle[0];
    }

    if (r14->m25D == 2)
    {
        //0607EA3E
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        // update roll
        fixedPoint tempRotZ = r14->m3C_targetAngles[2] - r14->m20_angle[2];
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, tempRotZ.normalized() * 15) - r14->m20_angle[2];
    }

    //607EA84
    // clamp angle.x to valid range
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    r14->m247 = 0;
    r14->m246_previousAnalogY = 0;
    r14->m245_previousAnalogX = 0;

    r14->m25C &= ~1;
}

// ============================================================
// Dragon input
// ============================================================

u32 isDragonInputAllowed(s_dragonTaskWorkArea* r4)
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x10000)
        return 0;

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8)
        return 0;

    if (r14->m4_currentScript.m_offset)
        return 0;

    if (r14->m30_cinematicBarTask)
        return 0;

    if (r14->m34)
        return 0;

    if (r14->m38_dialogStringTask)
        return 0;

    if (r14->m3C_multichoiceTask)
        return 0;

    return 1;
}

void applyDragonAnimationFromInput(s_dragonTaskWorkArea* r14, s_dragonState* r12)
{
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // down
    {
        incrementAnimationRootY(&r12->m78_animData, r14->m184_animRate);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // up
    {
        incrementAnimationRootY(&r12->m78_animData, -r14->m184_animRate);
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7]) // right
    {
        incrementAnimationRootX(&r12->m78_animData, r14->m184_animRate);
        incrementAnimationRootZ(&r12->m78_animData, -r14->m184_animRate);
    }
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6]) // left
    {
        incrementAnimationRootX(&r12->m78_animData, -r14->m184_animRate);
        incrementAnimationRootZ(&r12->m78_animData, r14->m184_animRate);
    }
}

void applyDragonAnimationFromAnalog(s_dragonTaskWorkArea* r11, s_dragonState* r12)
{
    s32 r4_y;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r4_y = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r4_y = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }

    s32 var0 = r11->m246_previousAnalogY - r4_y;
    if (var0 > 0x40)
    {
        incrementAnimationRootY(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * var0));
    }
    else if (var0 < -0x40)
    {
        incrementAnimationRootY(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * var0));
    }

    //607F1E8
    s32 r4_x = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
    s32 r10 = r4_x - r11->m245_previousAnalogX;
    if (r10 > 0x40)
    {
        incrementAnimationRootX(&r12->m78_animData, intDivide(0x80, -(r11->m184_animRate.asS32() * r10)));
        incrementAnimationRootZ(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * r10));
    }
    else if (r10 < -0x40)
    {
        incrementAnimationRootX(&r12->m78_animData, intDivide(0x80, -(r11->m184_animRate.asS32() * r10)));
        incrementAnimationRootZ(&r12->m78_animData, intDivide(0x80, r11->m184_animRate.asS32() * r10));
    }
}

// ============================================================
// Barrel roll
// ============================================================

static void startBarrelRollMode(s_dragonTaskWorkArea* r14);

void handleBarrelRollInputAnalog(s_dragonTaskWorkArea* r4)
{
    s32 timer = (s32)r4->m258 - 1;
    r4->m258 = timer;
    if (timer < 1)
    {
        r4->m25D = 0;
        r4->m258 = 0;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][15])
        {
            // 0607E7F8: forward + barrel roll button -> reverse barrel roll
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
            {
                if (gDragonState->mC_dragonType != 8) // not floater
                {
                    r4->m25D = 2;
                    r4->m258 = 0x18;
                    if (r4->m25E == 0)
                    {
                        r4->m254 = 0x84BDA1;
                    }
                    else
                    {
                        r4->m254 = -0x84BDA1;
                    }
                    r4->m23C |= 4;
                    r4->m244 = 9;
                }
                return;
            }

            // Analog stick rotation dash (only at idle speed)
            if (r4->m235_dragonSpeedIndex == 0)
            {
                s8 analogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
                if (analogX < 0)
                {
                    // Tilt left -> barrel roll left
                    r4->m250 = fixedPoint((s32)r4->m20_angle[1] + (s32)0xF838E38F).normalized();
                    startBarrelRollMode(r4);
                    return;
                }
                if (analogX > 0)
                {
                    // Tilt right -> barrel roll right
                    r4->m250 = fixedPoint((s32)r4->m20_angle[1] + (s32)0x7C71C71).normalized();
                    startBarrelRollMode(r4);
                    return;
                }
            }
        }
    }
}

// ============================================================
// Dragon pitch/yaw/roll — analog
// ============================================================

void interpolateYawWithBanking(s_dragonTaskWorkArea* r14); // forward decl

void applyDragonPitchYawRollAnalog(s_dragonTaskWorkArea* r14)
{
    s32 r9_y;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r9_y = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r9_y = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    r9_y = -r9_y;

    // TODO: understand the code that tests angle and 0x80000000
    handleBarrelRollInputAnalog(r14);

    fixedPoint r3;
    if (r14->m154_dragonSpeed >= 0)
    {
        r3 = r14->m154_dragonSpeed;
    }
    else
    {
        r3 = -r14->m154_dragonSpeed;
    }

    //607F2F8
    if (r3 >= 0x100)
    {
        if (r9_y > 0)
        {
            // down
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->m20_angle[0] += r14->m1F0.m_8;
            r14->m238 |= 2;
            r14->mFC |= 2;
        }
        else if(r9_y < 0)
        {
            // up
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->m20_angle[0] += r14->m1F0.m_8;
            r14->m238 |= 1;
            r14->mFC |= 1;
        }
        else
        {
            //607F360
            fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
        }
    }
    else
    {
        //607F3B4
        fixedPoint r4 = intDivide(0x7F, -(r9_y << 11));
        if (r9_y > 0)
        {
            r14->m160_deltaTranslation[1] += r4;
            r14->m238 |= 2;
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->mFC |= 2;
        }
        else if (r9_y < 0)
        {
            r14->m160_deltaTranslation[1] += r4;
            r14->m238 |= 1;
            r14->m1F0.m_8 = intDivide(0x7F, r9_y * r14->m178_turnRate.asS32());
            r14->mFC |= 1;
        }

        //607F428
        fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
    }

    //607F45A
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    //607F49A
    r14->m1F0.m_C = intDivide(0x7F, graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX * r14->m178_turnRate.asS32());

    if (r14->m25D == 1)
    {
        interpolateYawWithBanking(r14);
    }
    else
    {
        if ((r14->mF8_Flags & 0x8000) == 0)
        {
            r14->m20_angle[1] += r14->m1F0.m_C;
        }
    }

    //0607F4F0
    if (r14->m25D == 2)
    {
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, fixedPoint(r14->m3C_targetAngles[2] - r14->m20_angle[2]).normalized() * 15) - r14->m20_angle[2];
        s32 analogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
        if (analogX > 0)
        {
            //0607F540
            fixedPoint r2 = r14->m20_angle[1] - r14->m30;
            if (r2 >= intDivide(0x7F, analogX * r14->m178_turnRate.asS32()))
            {
                r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
                r14->mFC |= 4;
                r14->m25E = 1;
            }
        }
        else if (analogX < 0)
        {
            //607F57E
            fixedPoint r2 = r14->m30 - r14->m20_angle[1];
            if (r2 >= intDivide(0x7F, -analogX * r14->m178_turnRate.asS32()))
            {
                r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
                r14->mFC |= 8;
                r14->m25E = 0;
            }
        }
    }

    //607F5BA
    r14->m245_previousAnalogX = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
    if (graphicEngineStatus.m4514.m138[1])
    {
        r14->m246_previousAnalogY = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }
    else
    {
        r14->m246_previousAnalogY = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
    }

    r14->m247 = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m4;
}

// 0607e638
void interpolateYawWithBanking(s_dragonTaskWorkArea* r14)
{
    r14->m20_angle[1] = interpolateRotation(r14->m20_angle[1], r14->m250, 0x2000, 0x888889, 0x10);
    if (r14->m178_turnRate <= r14->m30 - r14->m20_angle[1])
    {
        r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
    }
    else if (r14->m178_turnRate <= r14->m20_angle[1] - r14->m30)
    {
        r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
    }
}

// 0607e606
static void startBarrelRollMode(s_dragonTaskWorkArea* r14)
{
    r14->m25D = 1;
    r14->m258 = 0x1E;
    r14->m23C |= 4;
    r14->m244 = 4;
}

// ============================================================
// Dragon pitch/yaw/roll — digital
// ============================================================

// 0607e718
void handleBarrelRollInput(s_dragonTaskWorkArea* r14)
{
    s32 timer = (s32)r14->m258 - 1;
    r14->m258 = timer;
    if (timer < 1)
    {
        r14->m25D = 0;
        r14->m258 = 0;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0xF])
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
            {
                if (gDragonState->mC_dragonType != 8)
                {
                    r14->m25D = 2;
                    r14->m258 = 0x18;
                    if (r14->m25E == 0)
                    {
                        r14->m254 = 0x84BDA1;
                    }
                    else
                    {
                        r14->m254 = -0x84BDA1;
                    }
                    r14->m23C |= 4;
                    r14->m244 = 9;
                }
                return;
            }

            if (r14->m235_dragonSpeedIndex == 0)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7])
                {
                    r14->m250 = fixedPoint((s32)r14->m20_angle[1] + (s32)0xF838E38F).normalized();
                    startBarrelRollMode(r14);
                    return;
                }

                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6])
                {
                    r14->m250 = fixedPoint((s32)r14->m20_angle[1] + (s32)0x7C71C71).normalized();
                    startBarrelRollMode(r14);
                    return;
                }
            }
        }
    }
}

void applyDragonPitchYawRoll(s_dragonTaskWorkArea* r14)
{
    handleBarrelRollInput(r14);

    fixedPoint r2;
    if (r14->m154_dragonSpeed >= 0)
    {
        r2 = r14->m154_dragonSpeed;
    }
    else
    {
        r2 = -r14->m154_dragonSpeed;
    }

    if (r2 >= 0x100)
    {
        //0607EC06
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // Go down
        {
            //0607EC12
            r14->m20_angle[0] += r14->m178_turnRate;
            r14->m238 |= 2;
            r14->m1F0.m_8 = r14->m178_turnRate;
            r14->mFC |= 2;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // Go up
        {
            //0607EC44
            r14->m20_angle[0] -= r14->m178_turnRate;
            r14->m238 |= 1;
            r14->m1F0.m_8 = -r14->m178_turnRate;
            r14->mFC |= 1;
        }
        else
        {
            //607EC6C
            fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
            r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
        }
    }
    else
    {
        //0607ECA4
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][5]) // Go down
        {
            //607ECB0
            r14->m160_deltaTranslation[1] -= 0x800;
            r14->m238 |= 2;
            r14->m1F0.m_8 = r14->m178_turnRate;
            r14->mFC |= 2;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][4]) // Go up
        {
            //607ED00
            r14->m160_deltaTranslation[1] += 0x800;
            r14->m238 |= 1;
            r14->m1F0.m_8 = -r14->m178_turnRate;
            r14->mFC |= 1;
        }

        //607ED32
        fixedPoint r1 = r14->m3C_targetAngles[0] - r14->m20_angle[0];
        r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[0];
    }

    //0607ED68
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }
    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }



    if (r14->m25D == 2)
    {
        //0607EDB0
        r14->m20_angle[2] += r14->m254;
    }
    else
    {
        fixedPoint r1 = r14->m3C_targetAngles[2] - r14->m20_angle[2];
        r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, r1.normalized() * 15) - r14->m20_angle[2];

        if (r14->m25D == 1)
        {
            interpolateYawWithBanking(r14);
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][7]) // right
            {
                //0607EE34
                if ((r14->mF8_Flags & 0x8000) == 0)
                {
                    r14->m20_angle[1] -= r14->m178_turnRate;
                }

                r14->m1F0.m_C = -r14->m178_turnRate;
                if (r14->m30 - r14->m20_angle[1] >= r14->m178_turnRate)
                {
                    r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
                }

                r14->mFC |= 8;
                r14->m25E = 0;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][6]) // left
            {
                //0607EE8C
                if ((r14->mF8_Flags & 0x8000) == 0)
                {
                    r14->m20_angle[1] += r14->m178_turnRate;
                }

                r14->m1F0.m_C = r14->m178_turnRate;
                if (r14->m20_angle[1] - r14->m30 >= r14->m178_turnRate)
                {
                    r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
                }

                r14->mFC |= 4;
                r14->m25E = 1;
            }
        }
    }

    r14->m247 = 0;
    r14->m246_previousAnalogY = 0;
    r14->m245_previousAnalogX = 0;
}

// ============================================================
// Dragon movement — digital and analog
// ============================================================

void updateDragonMovementDigital(s_dragonTaskWorkArea* r14)
{
    if (isDragonInputAllowed(r14))
    {
        applyDragonAnimationFromInput(r14, gDragonState);
        applyDragonPitchYawRoll(r14);
    }
    else
    {
        updateDragonMovementNoInput(r14);
    }
}

void updateDragonMovementAnalog(s_dragonTaskWorkArea* r14)
{
    if (isDragonInputAllowed(r14))
    {
        applyDragonAnimationFromAnalog(r14, gDragonState);
        applyDragonPitchYawRollAnalog(r14);
    }
    else
    {
        updateDragonMovementNoInput(r14);
    }
}

// ============================================================
// Dragon speed
// ============================================================

static u32 isDragonControlledByScripts()
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags & 0x10000)
    {
        return 1;
    }

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0xC8)
    {
        return 1;
    }

    if (r14->m4_currentScript.m_offset)
    {
        return 1;
    }

    if (r14->m30_cinematicBarTask)
    {
        return 1;
    }

    if (r14->m34)
    {
        return 1;
    }

    if (r14->m38_dialogStringTask)
    {
        return 1;
    }

    if (r14->m3C_multichoiceTask)
    {
        return 1;
    }

    return 0;
}

static u32 isDragonPlayerControlAllowed()
{
    s_fieldScriptWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;

    if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8)
    {
        return 0;
    }

    if (r14->m4_currentScript.m_offset)
    {
        return 0;
    }

    if (r14->m30_cinematicBarTask)
    {
        return 0;
    }

    if (r14->m34)
    {
        return 0;
    }

    if (r14->m38_dialogStringTask)
    {
        return 0;
    }

    if (r14->m3C_multichoiceTask)
    {
        return 0;
    }

    return 1;
}

static void clearDragonPlayerInputs()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    pDragonTask->m188[0] = 0;
    pDragonTask->m188[1] = 0;
    pDragonTask->m188[2] = 0;

    pDragonTask->m194[0] = 0;
    pDragonTask->m194[1] = 0;
    pDragonTask->m194[2] = 0;

    pDragonTask->m1A0[0] = 0;
    pDragonTask->m1A0[1] = 0;
    pDragonTask->m1A0[2] = 0;

    pDragonTask->m1AC[0] = 0;
    pDragonTask->m1AC[1] = 0;
    pDragonTask->m1AC[2] = 0;
}

static u32 isDragonBoostAvailable()
{
    u32 T = !mainGameState.getBit(0x2A * 8 + 6);
    return T ^ 1;
}

static u32 isDragonBoostLocked()
{
    u32 T = !mainGameState.getBit(0x2A * 8 + 7);
    return T ^ 1;
}

void resetDragonSpeedIndex(s_dragonTaskWorkArea* r4)
{
    if ((r4->m25C & 1) == 0)
    {
        r4->m235_dragonSpeedIndex = 0;
        r4->m234 = 0;
        r4->m25C = 0;
    }
}

void updateDragonSpeed(s_dragonTaskWorkArea* r14)
{
    // 0607E8B6: clear boost flag if new button pressed or speed <= 0
    if ((r14->m25C & 0x1) &&
        (((graphicEngineStatus.m4514.mD8_buttonConfig[1][0] | graphicEngineStatus.m4514.mD8_buttonConfig[1][11]) &
          graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown) != 0 ||
         r14->m235_dragonSpeedIndex < 1))
    {
        r14->m25C &= ~1;
    }

    // 0607E8E4: brake button check (buttonConfig[1][11] without forward)
    if ((r14->m25C & 0x2) == 0 &&
        (graphicEngineStatus.m4514.mD8_buttonConfig[1][11] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
    {
        if (graphicEngineStatus.m4514.mD8_buttonConfig[1][0] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown)
        {
            // Both brake and forward held -> set speed to -1
            r14->m235_dragonSpeedIndex = -1;
        }
        else
        {
            // 0607E910: brake only -> reset speed and return
            resetDragonSpeedIndex(r14);
            return;
        }
    }
    else
    {
        // 0607E920: normal speed update path
        r14->m25C |= 2;

        if (isDragonBoostLocked() &&
            (graphicEngineStatus.m4514.mD8_buttonConfig[1][15] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown))
        {
            r14->m25C |= 4;
        }

        s8 maxSpeed = (r14->m25C & 4) ? 4 : 3;

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[1][0])
        {
            // 0607E960: forward held -- accelerate
            if (++r14->m234 > 4)
            {
                if (++r14->m235_dragonSpeedIndex >= maxSpeed)
                {
                    r14->m235_dragonSpeedIndex = maxSpeed;
                }
                r14->m234 = 0;
            }
        }
        else
        {
            resetDragonSpeedIndex(r14);
        }

        // 0607E9AE: boost initiation
        if (isDragonBoostAvailable() && r14->m235_dragonSpeedIndex > 0 && (s32)r14->m154_dragonSpeed > 0 &&
            (graphicEngineStatus.m4514.mD8_buttonConfig[1][11] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown))
        {
            r14->m25C |= 1;
            r14->m158 = r14->m154_dragonSpeed;
        }
    }
}

// ============================================================
// Dragon movement integration
// ============================================================

void integrateDragonMovement(s_dragonTaskWorkArea* r14)
{
    getFieldTaskPtr()->m28_status &= ~0x10000;

    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    if (isDragonControlledByScripts())
    {
        clearDragonPlayerInputs();
    }

    if (isDragonPlayerControlAllowed())
    {
        if ((r14->mF8_Flags & 0x10000) == 0)
        {
            updateDragonSpeed(r14);
        }

        if (r14->m235_dragonSpeedIndex < 0)
        {
            //0607FE38
            r14->m15C_dragonSpeedIncrement = MTH_Mul(-r14->m154_dragonSpeed - 0x1284, r14->m230);
        }
        else
        {
            //607FE50
            fixedPoint r3;
            if (r14->m25C & 1)
            {
                r3 = r14->m158;
            }
            else
            {
                r3 = r14->m21C_DragonSpeedValues[r14->m235_dragonSpeedIndex];
            }

            fixedPoint r12 = r14->m154_dragonSpeed - r3;
            if (r12 < 0)
            {
                r14->m15C_dragonSpeedIncrement = -MTH_Mul(r12, r14->m230);
            }
            else
            {
                r14->m15C_dragonSpeedIncrement = MTH_Mul(-r12, r14->m230);
            }
        }
    }
    else
    {
        //607FE98
        if (r14->m154_dragonSpeed < 0)
        {
            r14->m15C_dragonSpeedIncrement = -MTH_Mul(r14->m154_dragonSpeed, r14->m230);
        }
        else
        {
            r14->m15C_dragonSpeedIncrement = MTH_Mul(-r14->m154_dragonSpeed, r14->m230);
        }
    }

    // 607FF02
    r14->m154_dragonSpeed += r14->m15C_dragonSpeedIncrement;

    // speed up?
    if (keyboardIsKeyDown(0xA9))
    {
        r14->m154_dragonSpeed.m_value *= 4;
    }

    // Clamp dragon speed
    fixedPoint r2 = r14->m154_dragonSpeed;
    if (r14->m154_dragonSpeed >= 0)
    {
        if (r14->m154_dragonSpeed < 0x7000)
        {
            r2 = r14->m154_dragonSpeed;
        }
        else
        {
            r2 = 0x7000;
        }
    }
    else
    {
        if (r14->m154_dragonSpeed >= -0x7000)
        {
            r2 = r14->m154_dragonSpeed;
        }
        else
        {
            r2 = -0x7000;
        }
    }
    r14->m154_dragonSpeed = r2;

    r14->m1AC[0] = interpolateRotation(r14->m1AC[0], 0, 0x2000, 0x444444, 0x10);
    r14->m1AC[1] = interpolateRotation(r14->m1AC[1], 0, 0x2000, 0x444444, 0x10);
    r14->m1AC[2] = interpolateRotation(r14->m1AC[2], 0, 0x2000, 0x444444, 0x10);

    r14->m1AC += r14->m1A0;

    r14->m20_angle += r14->m1AC;

    r14->m1A0.zeroize();

    // ~0607FFEC
    r14->m194[0] = interpolateDistance(r14->m194[0], 0, 0x2000, 0xAAA, 0x10);
    r14->m194[1] = interpolateDistance(r14->m194[1], 0, 0x2000, 0xAAA, 0x10);
    r14->m194[2] = interpolateDistance(r14->m194[2], 0, 0x2000, 0xAAA, 0x10);

    r14->m194 += r14->m188;

    if (r14->m154_dragonSpeed < 0)
    {
        //06080066
        r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.m[0][2], r14->m154_dragonSpeed);
        r14->m160_deltaTranslation[1] += r14->m194[1];
    }
    else
    {
        r14->m160_deltaTranslation[0] += r14->m194[0] - MTH_Mul(r14->m88_matrix.m[0][2], r14->m154_dragonSpeed);
        r14->m160_deltaTranslation[1] += r14->m194[1] + MTH_Mul(r14->m88_matrix.m[1][2], r14->m154_dragonSpeed);
    }
    r14->m160_deltaTranslation[2] += r14->m194[2] - MTH_Mul(r14->m88_matrix.m[2][2], r14->m154_dragonSpeed);

    r14->m188.zeroize();

    if (r14->mF4)
    {
        r14->mF4(r14);
    }

    //6080140
    r14->m8_pos += r14->m160_deltaTranslation;

    if ((r14->m134_minY == 0) && (r14->m140_maxY == 0))
    {
        return;
    }

    if (r14->m8_pos[1] < r14->m134_minY)
        r14->m8_pos[1] = r14->m134_minY;

    if (r14->m8_pos[1] > r14->m140_maxY)
        r14->m8_pos[1] = r14->m140_maxY;

    //608018E
    r14->m160_deltaTranslation = r14->m8_pos - r14->m14_oldPos;

    // Adjust pitch min when close to maxY (ceiling)
    // Early-out when far from boundary to avoid s32 overflow in the multiplication
    if ((r14->m140_maxY - r14->m8_pos[1]) > 0x200000)
    {
        r14->m148_pitchMin = -0x3555555;
    }
    else
    {
        fixedPoint r6 = (r14->m140_maxY - r14->m8_pos[1]) * -0x111;
        fixedPoint r2 = (r6 >= -0x3555555) ? r6 : fixedPoint(-0x3555555);
        fixedPoint r3;
        if (r2 >= 0)
            r3 = 0;
        else if (r6 >= -0x3555555)
            r3 = r6;
        else
            r3 = -0x3555555;
        r14->m148_pitchMin = r3;
    }

    // Adjust pitch max when close to minY (floor)
    if ((r14->m8_pos[1] - r14->m134_minY) > 0x200000)
    {
        r14->m14C_pitchMax = 0x3555555;
    }
    else
    {
        fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
        fixedPoint r2 = (r5 < 0x3555555) ? r5 : fixedPoint(0x3555555);
        fixedPoint r3;
        if (r2 < 0)
            r3 = 0;
        else if (r5 < 0x3555555)
            r3 = r5;
        else
            r3 = 0x3555555;
        r14->m14C_pitchMax = r3;
    }
}

// ============================================================
// Dragon update modes
// ============================================================

void dragonCutsceneUpdate(s_dragonTaskWorkArea* r14)
{
    r14->m24A_runningCameraScript = 4;
    s_scriptData3* r13 = r14->m1E4_cutsceneKeyFrame;
    if (r13 == NULL)
    {
        r14->m104_dragonScriptStatus = 3;
    }

    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    switch (r14->m104_dragonScriptStatus)
    {
    case 0:
        r14->mF8_Flags &= ~0x400;
        r14->mF8_Flags |= 0x20000;
        r14->m1E8_cameraScriptDelay = r13->m0_duration;
        r14->m1EA = r13->m10_rotationDuration;
        r14->m160_deltaTranslation[0] = intDivide(r13->m0_duration, r13->m4_pos[0] - r14->m8_pos[0]);
        r14->m160_deltaTranslation[1] = intDivide(r13->m0_duration, r13->m4_pos[1] - r14->m8_pos[1]);
        r14->m160_deltaTranslation[2] = intDivide(r13->m0_duration, r13->m4_pos[2] - r14->m8_pos[2]);

        if (r13->m10_rotationDuration)
        {
            r14->m16C_deltaRotation[0] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[0] - r14->m20_angle[0]).normalized());
            r14->m16C_deltaRotation[1] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[1] - r14->m20_angle[1]).normalized());
            r14->m16C_deltaRotation[2] = intDivide(r13->m10_rotationDuration, fixedPoint(r13->m14_rot[2] - r14->m20_angle[2]).normalized());
        }
        else
        {
            r14->m16C_deltaRotation[0] = 0;
            r14->m16C_deltaRotation[1] = 0;
            r14->m16C_deltaRotation[2] = 0;
        }

        updateDragonCollision(r14);
        if (r14->mF8_Flags & 0x40000)
        {
            updateCameraScriptSub0Sub2(r14);
            updateCameraScriptSub0(r14->mB8_lightWingEffect);
            r14->mF8_Flags &= ~0x40000;
        }
        r14->m104_dragonScriptStatus++;
    case 1:

        r14->m20_angle += r14->m16C_deltaRotation;
        r14->m8_pos += r14->m160_deltaTranslation;

        if (--r14->m1EA <= 0)
        {
            r14->m104_dragonScriptStatus++;
        }

        if (--r14->m1E8_cameraScriptDelay <= 0)
        {
            r14->m104_dragonScriptStatus = 3;
            r14->m1E4_cutsceneKeyFrame = NULL;
        }
        break;
    case 2:
        r14->m8_pos += r14->m160_deltaTranslation;
        if (--r14->m1E8_cameraScriptDelay <= 0)
        {
            r14->m104_dragonScriptStatus = 3;
            r14->m1E4_cutsceneKeyFrame = NULL;
        }
        break;
    case 3:
        break;
    default:
        assert(0);
        break;
    }

    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    if (--r14->m1EE < 0)
    {
        r14->m1EE = 0;
    }
    updateDragonCollision(r14);
}

// 060889ac
void dragonIdleUpdate(s_dragonTaskWorkArea* r14)
{
    r14->m24A_runningCameraScript = 5;

    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
    pLCS->m8 |= 1;

    getFieldTaskPtr()->m28_status |= 0x10000;

    r14->m160_deltaTranslation[0] = 0;
    r14->m160_deltaTranslation[1] = 0;
    r14->m160_deltaTranslation[2] = 0;

    if (r14->mF4)
    {
        r14->mF4(r14);
    }

    // Smooth pitch towards target
    s32 tempRotX = r14->m3C_targetAngles[0] - r14->m20_angle[0];
    if (tempRotX & 0x8000000)
    {
        tempRotX |= 0xF0000000;
    }
    else
    {
        tempRotX &= 0xFFFFFFF;
    }
    r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, (tempRotX << 4) - tempRotX) - r14->m20_angle[0];

    // Smooth roll towards target
    s32 tempRotZ = r14->m3C_targetAngles[2] - r14->m20_angle[2];
    if (tempRotZ & 0x8000000)
    {
        tempRotZ |= 0xF0000000;
    }
    else
    {
        tempRotZ &= 0xFFFFFFF;
    }
    r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, (tempRotZ << 4) - tempRotZ) - r14->m20_angle[2];

    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);
    updateDragonCollision(r14);
}

// ============================================================
// Dragon flight update
// ============================================================

void dragonFlightUpdate(s_dragonTaskWorkArea* r4)
{
    r4->m24A_runningCameraScript = 0;

    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        r4->m154_dragonSpeed = 0;
        getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 &= ~1;
        r4->mF8_Flags |= 0x400;
        r4->mF8_Flags &= ~0x20000;
        r4->m104_dragonScriptStatus++;
    case 1:
        if (!isDragonInFieldBounds(r4))
        {
            r4->mF0 = dragonExitField;
            r4->m104_dragonScriptStatus = 0;
            dragonExitField(r4);
            return;
        }
    }

    r4->m160_deltaTranslation[0] = 0;
    r4->m160_deltaTranslation[1] = 0;
    r4->m160_deltaTranslation[2] = 0;

    r4->m238 &= ~3;

    switch (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType)
    {
    case 1:
        updateDragonMovementDigital(r4);
        break;
    case 2:
        updateDragonMovementAnalog(r4);
        break;
    default:
        assert(0);
    }

    integrateDragonMovement(r4);
}

// ============================================================
// Dragon exit field
// ============================================================

void dragonExitField(s_dragonTaskWorkArea* r14)
{
    s_visibilityGridWorkArea* r12 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    //r13 = r4->m178

    r14->m24A_runningCameraScript = 6;
    getFieldTaskPtr()->m28_status |= 0x200;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;

    switch (r14->m104_dragonScriptStatus)
    {
    case 0:
        startScriptLeaveArea();
        initDragonSpeed(0);
        r14->m_1C4 = 45;
        r14->m104_dragonScriptStatus++;
    case 1:
        if (r14->m_1C4--)
            break;
        r14->m_1C4 = 1;
        r14->m104_dragonScriptStatus++;
        break;
    case 2:
        if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m3C_multichoiceTask == 0)
        {
            dragonFieldTaskInitSub4Sub4();
            r14->mF0(r14);
            getFieldTaskPtr()->m28_status &= 0xFFFFFDFF;
            return;
        }
        break;
    default:
        assert(0);
        break;
    }

    //607FA40
    r14->m238 &= 0xFFFFFFFC;
    sVec3_FP var_8;
    var_8[0] = r12->mC[0] - r14->m8_pos[0];
    var_8[1] = 0;
    var_8[2] = r12->mC[2] - r14->m8_pos[2];

    sVec2_FP var0;
    computeLookAt(var_8, var0);

    // update yaw
    s32 tempRotX = r14->m3C_targetAngles[0] - r14->m20_angle[0];
    if (tempRotX & 0x8000000)
    {
        tempRotX |= 0xF0000000;
    }
    else
    {
        tempRotX &= 0xFFFFFFF;
    }

    r14->m20_angle[0] += r14->m3C_targetAngles[0] - intDivide(0x10, (tempRotX << 4) - tempRotX) - r14->m20_angle[0];

    // update pitch
    r14->m20_angle[1] = interpolateRotation(r14->m20_angle[1], var_8[0], 0x2000, 0x444444, 0x10);

    // update roll
    s32 tempRotZ = r14->m3C_targetAngles[2] - r14->m20_angle[2];
    if (tempRotZ & 0x8000000)
    {
        tempRotZ |= 0xF0000000;
    }
    else
    {
        tempRotZ &= 0xFFFFFFF;
    }
    r14->m20_angle[2] += r14->m3C_targetAngles[2] - intDivide(0x10, (tempRotZ << 4) - tempRotZ) - r14->m20_angle[2];

    if (r14->m30 - r14->m20_angle[1] < r14->m178_turnRate)
    {
        r14->m20_angle[2] += intDivide(4, r14->m178_turnRate * 3);
    }
    else if (r14->m20_angle[1] - r14->m30 < r14->m178_turnRate)
    {
        r14->m20_angle[2] -= intDivide(4, r14->m178_turnRate * 3);
    }

    //607FB2A
    // clamp angle.x to valid range
    if (r14->m20_angle[0].normalized() > r14->m14C_pitchMax)
    {
        r14->m20_angle[0] = r14->m14C_pitchMax;
    }

    if (r14->m20_angle[0].normalized() < r14->m148_pitchMin)
    {
        r14->m20_angle[0] = r14->m148_pitchMin;
    }

    //607FB96
    buildDragonRotationMatrix(&r14->m48, &r14->m20_angle);
    copyMatrix(&r14->m48.m0_matrix, &r14->m88_matrix);

    r14->m15C_dragonSpeedIncrement = 0;
    r14->m154_dragonSpeed = 0;

    computeDragonDeltaTranslation(r14);

    r14->m8_pos += r14->m160_deltaTranslation;

    // this is all copied from dragonExitField but looks like it's exactly the same
    if ((r14->m134_minY == 0) && (r14->m140_maxY == 0))
        return;

    if (r14->m8_pos[1] < r14->m134_minY)
        r14->m8_pos[1] = r14->m134_minY;

    if (r14->m8_pos[1] > r14->m140_maxY)
        r14->m8_pos[1] = r14->m140_maxY;

    r14->m160_deltaTranslation = r14->m8_pos - r14->m14_oldPos;

    // Adjust pitch min when close to maxY (ceiling)
    // Early-out when far from boundary to avoid s32 overflow in the multiplication
    if ((r14->m140_maxY - r14->m8_pos[1]) > 0x200000)
    {
        r14->m148_pitchMin = -0x3555555;
    }
    else
    {
        fixedPoint r6 = (r14->m140_maxY - r14->m8_pos[1]) * -0x111;
        fixedPoint r2 = (r6 >= -0x3555555) ? r6 : fixedPoint(-0x3555555);
        fixedPoint r3;
        if (r2 >= 0)
            r3 = 0;
        else if (r6 >= -0x3555555)
            r3 = r6;
        else
            r3 = -0x3555555;
        r14->m148_pitchMin = r3;
    }

    // Adjust pitch max when close to minY (floor)
    if ((r14->m8_pos[1] - r14->m134_minY) > 0x200000)
    {
        r14->m14C_pitchMax = 0x3555555;
    }
    else
    {
        fixedPoint r5 = (r14->m8_pos[1] - r14->m134_minY) * 0x111;
        fixedPoint r2 = (r5 < 0x3555555) ? r5 : fixedPoint(0x3555555);
        fixedPoint r3;
        if (r2 < 0)
            r3 = 0;
        else if (r5 < 0x3555555)
            r3 = r5;
        else
            r3 = 0x3555555;
        r14->m14C_pitchMax = r3;
    }
}

// ============================================================
// Dragon collision
// ============================================================

void updateDragonCollision(s_dragonTaskWorkArea* r4)
{
    sVec3_FP var;

    var[0] = r4->m160_deltaTranslation[0] * 0x100;
    var[1] = r4->m160_deltaTranslation[1] * 0x100;
    var[2] = r4->m160_deltaTranslation[2] * 0x100;

    s32 r0 = dot3_FP(&var, &var);
    r4->m154_dragonSpeed = sqrt_F(r0) >> 8;
}

// ============================================================
// Camera script
// ============================================================

void updateCameraScriptSub0Sub2(s_dragonTaskWorkArea* r4)
{
    s32 r2 = r4->m21C_DragonSpeedValues[0] + r4->m21C_DragonSpeedValues[1];

    if (0 > r2)
        r2++;
    r2 >>= 1;
    if (r4->m154_dragonSpeed > r2)
    {
        if (r4->m23A_dragonAnimation == 0)
            return;
        if (r4->m23A_dragonAnimation == 2)
            return;

        r4->m238 = 4;
        r4->m237 = 4;
        r4->m244 = 0;
    }
    else
    {
        if (r4->m23A_dragonAnimation == 5)
            return;

        r4->m238 = 0;
        r4->m237 = 0;
        r4->m244 = 5;
    }

    r4->m23C |= 5;
}

void updateCameraScript(s_dragonTaskWorkArea* r4, s_cameraScript* r5)
{
    r4->m24A_runningCameraScript = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1; // Disable LCS?

    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        updateCameraScriptSub0(r4->mB8_lightWingEffect);
        activateCameraFollowMode(0);
        r4->mF8_Flags &= 0xFFFFFBFF;
        r4->mF8_Flags |= 0x20000;
        r4->m1E8_cameraScriptDelay = r5->m20_length;
        r4->m20_angle = r5->mC_rotation;
        r4->m8_pos = r5->m0_position;

        r4->m160_deltaTranslation[0] = MTH_Mul(-r5->m1C, getSin(r4->m20_angle[1].getInteger() & 0xFFF));
        r4->m160_deltaTranslation[1] = intDivide(r4->m1E8_cameraScriptDelay, r5->m18 - r4->m8_pos[1]);
        r4->m160_deltaTranslation[2] = MTH_Mul(-r5->m1C, getCos(r4->m20_angle[1].getInteger() & 0xFFF));

        updateDragonCollision(r4);
        updateCameraScriptSub0Sub2(r4);

        getFieldCameraStatus()->m0_position = r5->m24_pos2;
        getFieldCameraStatus()->m88 = r4->m1E8_cameraScriptDelay;
        r4->m104_dragonScriptStatus++;
        break;
    case 1:
        if (--r4->m1E8_cameraScriptDelay)
        {
            return;
        }
        getFieldCameraStatus()->m88 = 30;
        r4->m1E8_cameraScriptDelay = 30;
        activateCameraFollowMode(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
        r4->m104_dragonScriptStatus++;
        break;
    case 2:
        if (--r4->m1E8_cameraScriptDelay)
        {
            return;
        }
        r4->m1D0_cameraScript = NULL;
        dragonFieldTaskInitSub4Sub4();
        break;
    default:
        assert(0);
    }
}

void dragonScriptMovement(s_dragonTaskWorkArea* pTypedWorkArea)
{
    getFieldTaskPtr()->m28_status |= 0x10000;

    if (pTypedWorkArea->m1D4_cutsceneData == nullptr)
    {
        if (pTypedWorkArea->m1D0_cameraScript == nullptr)
        {
            dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
            dragonFieldTaskInitSub4Sub4();
        }
        else
        {
            updateCameraScript(pTypedWorkArea, pTypedWorkArea->m1D0_cameraScript);
        }
    }
    else
    {
        assert(0);
    }

    buildDragonRotationMatrix(&pTypedWorkArea->m48, &pTypedWorkArea->m20_angle);

    copyMatrix(&pTypedWorkArea->m48.m0_matrix, &pTypedWorkArea->m88_matrix);

    pTypedWorkArea->m8_pos[0] += pTypedWorkArea->m160_deltaTranslation[0];
    pTypedWorkArea->m8_pos[1] += pTypedWorkArea->m160_deltaTranslation[1];
    pTypedWorkArea->m8_pos[2] += pTypedWorkArea->m160_deltaTranslation[2];

    updateDragonCollision(pTypedWorkArea);
}

// ============================================================
// Light wing evolution check
// ============================================================

void dragonFieldInit_setupLightWingEvolutionCheck(s_dragonTaskWorkArea* pTypedWorkArea)
{
    initDragonLightWingEvolutionCheck(pTypedWorkArea);
}

// ============================================================
// Dragon update sub-functions
// ============================================================

void dragonFieldTaskUpdateSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    pTypedWorkArea->mFC = 0;
    pTypedWorkArea->m1F0.m_8 = 0;
    pTypedWorkArea->m1F0.m_C = 0;

    pTypedWorkArea->m14_oldPos = pTypedWorkArea->m8_pos;

    pTypedWorkArea->mF0(pTypedWorkArea);

    resolveDragonTerrainCollision();
}

s32 getDragonSpeedIndex(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (pTypedWorkArea->mF8_Flags & 0x20000)
    {
        s32 r1 = pTypedWorkArea->m21C_DragonSpeedValues[0] + pTypedWorkArea->m21C_DragonSpeedValues[1];

        //Average speeds
        if (0 > r1)
            r1++;
        r1 >>= 1;

        if (pTypedWorkArea->m154_dragonSpeed >= r1)
        {
            return 1;
        }
        return 0;
    }
    else
    {
        return pTypedWorkArea->m235_dragonSpeedIndex;
    }
}

// ============================================================
// Dragon animation
// ============================================================

static void dragonFieldPlayAnimation(s_dragonTaskWorkArea* r14, s_dragonState* r13, u8 r12)
{
    if (r14->m23A_dragonAnimation == r12)
        return;

    u8 r4 = r14->m23A_dragonAnimation;
    if (r4 == 0)
    {
        r4 = 2;
    }

    u8 r5;
    if (r12)
    {
        r5 = r12;
    }
    else
    {
        r5 = 2;
    }

    if (r5 == r4)
    {
        setupModelAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelBundle->getAnimation(r13->m20_dragonAnimOffsets[r12]));
        r14->m23B = 1;
    }
    else
    {
        playAnimation(&r13->m28_dragon3dModel, r13->m0_pDragonModelBundle->getAnimation(r13->m20_dragonAnimOffsets[r12]), 10);
        r14->m23B = 0;
    }

    updateAndInterpolateAnimation(&r13->m28_dragon3dModel);

    r14->m23A_dragonAnimation = r12;
    r14->m237 = r14->m238;
}

static s8 dragonFieldAnimation[] = {
    5,7,8,11,9,9,9,10,
    4,4,4,11,0,0,2,10,
};

static s32 getDragonFieldAnimation(s_dragonTaskWorkArea* pTypedWorkArea)
{
    return dragonFieldAnimation[(pTypedWorkArea->m238 >> 2) * 8 + pTypedWorkArea->m238];
}

static std::vector<s8> getFieldDragonAnimTable(int type, int subtype)
{
    sSaturnPtr EA = readSaturnEA(sSaturnPtr{ gFieldDragonAnimTableEA.m_offset + ((type * 5) + subtype) * 4, gFieldDragonAnimTableEA.m_file });

    std::vector<s8> result;

    int numEntries = 0;
    while (readSaturnS8(EA) != -1)
    {
        result.push_back(readSaturnS8(EA));
        EA += 1;
        numEntries++;
    }
    result.push_back(-1);

    return result;
}

void dragonFieldAnimationUpdate(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
{
    u8 var = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex;

    if (pTypedWorkArea->m23C & 4)
    {
        switch (pTypedWorkArea->m23C & 3)
        {
        case 1:
            dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
            pTypedWorkArea->m23C &= ~1;
            break;
        case 2:
            assert(0);
        default:
            break;
        }

        pTypedWorkArea->m23C &= ~4;
    }
    else
    {
        // when activating LCS
        if (getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 & 0x10)
        {
            pTypedWorkArea->m238 |= 3;
            dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
            return;
        }
        else
        {
            if (pTypedWorkArea->m238 & 4)
            {
                if (getDragonSpeedIndex(pTypedWorkArea) <= 0)
                {
                    pTypedWorkArea->m238 &= ~4;
                    if (pTypedWorkArea->m154_dragonSpeed > 0xDDD)
                    {
                        //060734B6
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
                        return;
                    }
                }
            }
            //6073508
            else if (getDragonSpeedIndex(pTypedWorkArea) > 0)
            {
                pTypedWorkArea->m238 |= 4;
                if (pTypedWorkArea->m154_dragonSpeed < 0x555)
                {
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, getDragonFieldAnimation(pTypedWorkArea));
                    return;
                }
            }

            //6073524
            if (r5->m28_dragon3dModel.m16_previousAnimationFrame)
            {
                return;
            }

            //6073530
            if (pTypedWorkArea->m23C)
            {
                switch (pTypedWorkArea->m23C)
                {
                case 1:
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, pTypedWorkArea->m244);
                    pTypedWorkArea->m23C &= ~1;
                    break;
                case 2:
                    assert(0);
                default:
                    break;
                }

                return;
            }

            //06073588
            if (var >= 0)
            {
                s32 r6 = getDragonFieldAnimation(pTypedWorkArea);
                if (r6 == 0)
                {
                    std::vector<s8> r3 = getFieldDragonAnimTable(r5->mC_dragonType, r5->m1C_dragonArchetype);

                    pTypedWorkArea->m239++;
                    r6 = r3[pTypedWorkArea->m239];

                    if (r6 < 0)
                    {
                        pTypedWorkArea->m239 = 0;
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, r3[pTypedWorkArea->m239]);
                    }
                    else
                    {
                        dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
                    }
                }
                if (r6 > 0)
                {
                    dragonFieldPlayAnimation(pTypedWorkArea, r5, r6);
                }
                return;
            }
            else
            {
                dragonFieldPlayAnimation(pTypedWorkArea, r5, 5);
                return;
            }

            assert(0);
        }
    }
    PDS_unimplemented("dragonFieldAnimationUpdate");
}

// ============================================================
// Dragon sound effects
// ============================================================

// 060732da
static s8 dragonSoundEffectTable[] = {
    0x0D, -1, -1, -1, 0x0A, 0x0D, -1, 0x0B, -1, 0x0A, -1, -1,
    0x18, -1, -1, -1, 0x19, 0x1A, -1, 0x16, -1, 0x1E, -1, -1,
    0x1E, -1, -1, -1, 0x1F, 0x1E, -1, 0x14, -1, 0x1F, -1, -1,
    0x16, -1, -1, -1, 0x12, 0x1A, -1, 0x11, -1, 0x15, -1, -1,
    0x1E, -1, -1, -1, 0x14, 0x1E, -1, 0x16, -1, 0x16, -1, -1,
    0x1A, -1, -1, -1, 0x15, 0x1B, -1, 0x18, -1, 0x19, -1, -1,
    0x1F, -1, -1, -1, 0x15, -1, -1, -1, -1, 0x19, 0x0A, 0x0A,
    0x1B, -1, -1, -1, 0x15, 0x1A, -1, 0x16, -1, 0x14, -1, -1,
};

void playDragonSoundEffect(s_dragonTaskWorkArea* pTypedWorkArea, s_dragonState* r5)
{
    s32 iVar1 = (s8)pTypedWorkArea->m249_noCollisionAndHideDragon;
    if (iVar1 == 0)
    {
        iVar1 = (s8)dragonSoundEffectTable[(s8)((s8)r5->mC_dragonType * 12) + (s8)pTypedWorkArea->m23A_dragonAnimation];
        if (r5->m28_dragon3dModel.m16_previousAnimationFrame == iVar1)
        {
            playSystemSoundEffect(0xb);
        }
    }
}

// ============================================================
// Dragon terrain callback
// ============================================================

// 06012674 -- per-field terrain callback
static void fieldTerrainCallback(s32 subFieldIndex, sVec3_FP* pPos, sVec3_FP* pAngle)
{
    // Table at 0x0020a388 indexed by current field index
    // Many entries are null (e.g., field 8, 0xC, 0xD, 0xE)
    // When null, this is a no-op
    Unimplemented();
}

// 0602f8e4 -- read dragon pos and transform for special fields (0xF, 0x12)
static void readAndTransformDragonPos(sVec3_FP* pOut)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pOut->m0_X = pDragon->m8_pos[0];
    pOut->m4_Y = pDragon->m8_pos[1];
    pOut->m8_Z = pDragon->m8_pos[2];
    Unimplemented(); // FUN_0602f87c -- transform pos based on sub-field offsets
}

// 06073c7a
void updateDragonSavedAnglesAndTerrain(s_dragonTaskWorkArea* r14)
{
    r14->m2C_savedPitch = r14->m20_angle[0];
    r14->m30 = r14->m20_angle[1];
    r14->m34_savedRoll = r14->m20_angle[2];

    fixedPoint speed = r14->m154_dragonSpeed;
    if (speed < 0) speed = -speed;
    r14->m38_distanceAccum = r14->m38_distanceAccum + speed;

    auto pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 0xF || pFieldTask->m2C_currentFieldIndex == 0x12)
    {
        sVec3_FP transformedPos;
        readAndTransformDragonPos(&transformedPos);
        fieldTerrainCallback(0, &transformedPos, nullptr);
    }
    else
    {
        fieldTerrainCallback((s32)pFieldTask->m2E_currentSubFieldIndex, &r14->m8_pos, &r14->m20_angle);
    }
}

// ============================================================
// Dragon draw sub-functions
// ============================================================

void printMainDebugStats(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (enableDebugTask)
    {
        assert(0);
    }
}

void dragonFieldTaskDrawSub1(s_dragonTaskWorkArea* pTypedWorkArea)
{
    drawFieldCameraSlots();

    if ((pTypedWorkArea->m_EC & 1) == 0)
    {
        s_RGB8* pColor;
        sVec3_FP lightLocation;
        if (pTypedWorkArea->m_EB_useSpecialColor)
        {
            dragonFieldTaskDrawSub1Sub0();

            sVec3_FP varC;
            varC[0] = pTypedWorkArea->m8_pos[0];
            varC[1] = pTypedWorkArea->m8_pos[1] + 0xA000;
            varC[2] = pTypedWorkArea->m8_pos[2];

            transformVecByCurrentMatrix(varC, lightLocation);

            dragonFieldTaskDrawSub1Sub1(lightLocation[0], lightLocation[1], lightLocation[2]);
            pColor = &pTypedWorkArea->m_E8_specialColor;
        }
        else
        {
            sVec3_FP varC;
            varC[0] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()), getSin(pTypedWorkArea->mC4.getInteger()));
            varC[1] = MTH_Mul(fixedPoint(0x10000), getSin(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()));
            varC[2] = -MTH_Mul_5_6(fixedPoint(0x10000), getCos(pTypedWorkArea->mC0_lightRotationAroundDragon.getInteger()), getCos(pTypedWorkArea->mC4.getInteger()));
            transformVecByCurrentMatrix(varC, lightLocation);
            pColor = &pTypedWorkArea->mC8_normalLightColor;
        }
        //060740F6
        setupLight(lightLocation[0], lightLocation[1], lightLocation[2], pColor->toU32());
        generateLightFalloffMap(pTypedWorkArea->mCB_falloffColor0.toU32(), pTypedWorkArea->mCE_falloffColor1.toU32(), pTypedWorkArea->mD1_falloffColor2.toU32());
    }

#ifndef SHIPPING_BUILD
    if (gDebugWindows.field)
    {
        if (ImGui::Begin("Field", &gDebugWindows.field))
        {
            Imgui_FP_Angle("Field of view", &pTypedWorkArea->m1CC_fieldOfView);
        }
        ImGui::End();
    }
#endif

    //0607416C
    initVDP1Projection(pTypedWorkArea->m1CC_fieldOfView / 2, 0);
    printMainDebugStats(pTypedWorkArea);
}

// ============================================================
// Dragon hotspot
// ============================================================

struct s_dragonHotspotPerDragonType
{
    u32 m0;
    u32 m4;
};

static s_dragonHotspotPerDragonType dragonHotspotPerDragonType[DR_LEVEL_MAX][5] = {
    { {0, 0},{0,1},{17,1}, {9, 0}, {12,0} },
    { {15, 0},{15,1}, {20, 1}, {9, 0}, {26, 0} },
    { {12, 0},{12,1}, {16, 0}, {23, 0}, {26, 0} },
    { {1, 0}, {1,1}, {4,1}, {12,0}, {15,0} },
    { {1, 0}, {1,1}, {5,1}, {12,0}, {31,0} },
    { {10, 0}, {10,1}, {13,0}, {25,0}, {28,0}},
    { {1,0}, {1,1}, {2,1}, {23,0}, {25,0} },
    { {1,0}, {0,0}, {3,1}, {7,0}, {22,0} },
    { {0,3}, {0,4}, {0,0}, {0,1}, {0,0} },
};

void getDragonHotSpot(s_dragonState* r4, u32 r5, sVec3_FP* r6)
{
    s_dragonHotspotPerDragonType* pHotSpotData = &dragonHotspotPerDragonType[r4->mC_dragonType][r5];
    if (pHotSpotData->m0 < 0) // don't think that can ever happen
        return;

    sVec3_FP* pVec = &r4->m28_dragon3dModel.m44_hotpointData[pHotSpotData->m0][pHotSpotData->m4];
    transformAndAddVec(*pVec, *r6, cameraProperties2.m28[0]);
}

static void dragonFieldTaskDrawSub3Sub0()
{
    s_visibilityGridWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_visibilityGridWorkArea_1294* r5 = &r4->m1294;
    if (r5->m14 < r5->m10)
    {
        r5->m14 = r5->m10;
    }

    r5->m10 = 0;
    r5->mC = 0;
    r5->m8_processedQuadsForCollision3 = 0;
    r5->m4_processedQuadsForCollision2 = 0;

    r4->m44 = r4->m68.begin();
}

void dragonFieldTaskDrawSub3(s_dragonTaskWorkArea* pTypedWorkArea)
{
    if (((pTypedWorkArea->m_EC & 1) == 0) && (pTypedWorkArea->m_EB_useSpecialColor))
    {
        PDS_unimplemented("dragonFieldTaskDrawSub3 for setup light during dragon rendering");
    }

    dragonFieldTaskDrawSub3Sub0();
}

// ============================================================
// s_dragonTaskWorkArea Init / Update / Draw
// ============================================================

// 06073a7e
void s_dragonTaskWorkArea::Init(s_dragonTaskWorkArea* pThis, s32 arg)
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask = pThis;

    getMemoryArea(&pThis->m0, 0);
    initDragonFieldState(pThis);
    initDragonFieldAnimation(pThis, gDragonState, 5);
    pThis->mF0 = dragonScriptMovement;

    createDragonRiderTask(pThis);

    if ((gDragonState->mC_dragonType == DR_LEVEL_6_LIGHT_WING) && (pThis->mB8_lightWingEffect == nullptr))
    {
        // 0600adb8
        s16 vdp1Base = (s16)(((s32)pThis->m0.m4_characterArea + (s32)0xDA400000) >> 3);
        pThis->mB8_lightWingEffect = createLightWingEffect(pThis,
            vdp1Base + 0x11FC, vdp1Base + 0x1790, 0x20C,
            vdp1Base + 0x1208, vdp1Base + 0x1790, 0x20C);
    }

    dragonFieldInit_setupLightWingEvolutionCheck(pThis);

    if (shouldLoadPup())
    {
        assert(0);
    }
}

// 06073b44
void s_dragonTaskWorkArea::Update(s_dragonTaskWorkArea* pTypedWorkArea)
{
    dragonFieldTaskUpdateSub1(pTypedWorkArea);

    fieldTaskVar2 = gDragonState->mC_dragonType;

    if (pTypedWorkArea->mB8_lightWingEffect)
    {
        assert(0);
    }

    if (gDragonState->mC_dragonType != pTypedWorkArea->m100_previousDragonType)
    {
        pTypedWorkArea->m100_previousDragonType = gDragonState->mC_dragonType;
        dragonFieldTaskUpdateSub2(gDragonState->mC_dragonType + 1);

        if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
        {
            assert(0);
        }
    }

    updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    updateAndInterpolateAnimation(&pRider1State->m18_3dModel);
    if (mainGameState.gameStats.m3_rider2)
    {
        updateAndInterpolateAnimation(&pRider2State->m18_3dModel);
    }

    dragonFieldAnimationUpdate(pTypedWorkArea, gDragonState);
    playDragonSoundEffect(pTypedWorkArea, gDragonState);

    updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);

    if (pTypedWorkArea->m25D != 2)
    {
        getFieldCameraStatus()->m30 = -pTypedWorkArea->m20_angle[2] / 2;
    }

    updateFieldCameraSlots();

    updateDragonSavedAnglesAndTerrain(pTypedWorkArea);
}

// 06073fd0
void s_dragonTaskWorkArea::Draw(s_dragonTaskWorkArea* pTypedWorkArea)
{
    dragonFieldTaskDrawSub1(pTypedWorkArea);

    // if we need to draw the dragon shadow (and dragon Y >= 0)
    if (!pTypedWorkArea->m249_noCollisionAndHideDragon && pTypedWorkArea->m248 && (pTypedWorkArea->m8_pos[1] >= 0))
    {
        assert(0);
    }

    if (pTypedWorkArea->m249_noCollisionAndHideDragon)
    {
        WRITE_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30, READ_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30) & ~1);
    }
    else
    {
        WRITE_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30, READ_BE_U16(gDragonState->m0_pDragonModelBundle->getRawBuffer() + 0x30) | 1);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pTypedWorkArea->m8_pos);
    rotateCurrentMatrixShiftedY(0x8000000);
    multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
    scaleCurrentMatrixRow0(pTypedWorkArea->m150);
    scaleCurrentMatrixRow1(pTypedWorkArea->m150);
    scaleCurrentMatrixRow2(pTypedWorkArea->m150);

    gDragonState->m28_dragon3dModel.mC_modelIndexOffset = gDragonState->m14_modelIndex;
    gDragonState->m28_dragon3dModel.m18_drawFunction(&gDragonState->m28_dragon3dModel);
    popMatrix();

    //06074438
    getDragonHotSpot(gDragonState, 2, &pTypedWorkArea->m10C_hotSpot2);
    getDragonHotSpot(gDragonState, 3, &pTypedWorkArea->m118_hotSpot3);
    getDragonHotSpot(gDragonState, 4, &pTypedWorkArea->m124_hotSpot4);

    if (pTypedWorkArea->m249_noCollisionAndHideDragon == 0)
    {
        if (mainGameState.gameStats.m2_rider1)
        {
            sVec3_FP rider1_hotSpot;
            getDragonHotSpot(gDragonState, 0, &rider1_hotSpot);

            pushCurrentMatrix();
            translateCurrentMatrix(&rider1_hotSpot);
            rotateCurrentMatrixShiftedY(0x8000000);
            multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
            pRider1State->m18_3dModel.m18_drawFunction(&pRider1State->m18_3dModel);
            popMatrix();

            // draw rider's gun
            if (pRider1State->m18_3dModel.m44_hotpointData[5].size())
            {
                //060744AA
                transformAndAddVec(pRider1State->m18_3dModel.m44_hotpointData[5][0], rider1_hotSpot, cameraProperties2.m28[0]);
                pushCurrentMatrix();
                translateCurrentMatrix(&rider1_hotSpot);
                rotateCurrentMatrixShiftedY(0x8000000);
                multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
                addObjectToDrawList(pRider1State->m0_riderBundle->get3DModel(pRider1State->m14_weaponModelIndex));
                popMatrix();
            }
        }
        //60744E4
        if (mainGameState.gameStats.m3_rider2)
        {
            sVec3_FP rider2_hotSpot;
            getDragonHotSpot(gDragonState, 1, &rider2_hotSpot);

            pushCurrentMatrix();
            translateCurrentMatrix(&rider2_hotSpot);
            rotateCurrentMatrixShiftedY(0x8000000);
            multiplyCurrentMatrixSaveStack(&pTypedWorkArea->m48.m0_matrix);
            pRider2State->m18_3dModel.m18_drawFunction(&pRider2State->m18_3dModel);
            popMatrix();
        }
    }

    //06074520
    dragonFieldTaskDrawSub3(pTypedWorkArea);
}
