#include "PDS.h"
#include "berserkEffects.h"
#include "berserkShared.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleGrid.h"
#include "battleOverlay_C.h"
#include "battleEngineSub1.h"
#include "battleTargetable.h"
#include "battleGenericData.h"
#include "battleDamageDisplay.h"
#include "kernel/fade.h"
#include "audio/systemSounds.h"
#include "gunShotRootTask.h"
#include "battleIntro.h"
#include "battleResultScreen.h"
#include "mainMenuDebugTasks.h"
#include "trigo.h"
#include "kernel/animation.h"
#include "BTL_A3/BTL_A3_data.h"
#include "BTL_A3/BTL_A3_map6.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "3dModels.h"
#include "battleDebug.h"

//=============================================================================
// Dragon Phoenix
//=============================================================================

// BTL_A3::06085c3c — Dragon Phoenix
// Largest berserk task (0xBE0 WithCopy) with multi-phase phoenix animation.

struct sTrailEntry
{
    sVec3_FP m0_velocity;   // 0x00
    sVec3_FP mC_velocity2;  // 0x0C
    sVec3_FP m18_position;  // 0x18
    // size 0x24
};

struct sPhoenixParticle
{
    sAnimatedQuad m0_quad;  // 0x00 (8 bytes)
    sVec3_FP m8_position;  // 0x08
    s8 m14_lifetime;        // 0x14
    u8 m15_pad[3];          // 0x15
    // size 0x18
};

struct sDragonPhoenixTask : public s_workAreaTemplateWithCopy<sDragonPhoenixTask>
{
    // 0x00: m0_fileBundle (from s_workAreaCopy)
    // 0x04: m4_vd1Allocation (from s_workAreaCopy)
    u8* m8_rawModelData;                // 0x08
    s_3dModel m0C_model1;               // 0x0C (phoenix body, Saturn size 0x4C)
    u8 m58_pad[4];                      // 0x58
    s_3dModel m5C_model2;               // 0x5C (phoenix aura, Saturn size 0x4C)
    u8 mA8_pad[4];                      // 0xA8
    s8 mAC_state;                       // 0xAC — main state (0=approach, 1=attack, 2=retreat)
    s8 mAD_cameraQuadrant;              // 0xAD
    s8 mAE_prevState;                   // 0xAE
    s8 mAF_subState;                    // 0xAF
    s8 mB0_particleVisualIndex;         // 0xB0
    u8 mB1_trailFrameIndex;             // 0xB1
    u16 mB2_subTimer;                   // 0xB2
    u16 mB4_globalTimer;                // 0xB4
    u8 mB6_pad[2];                      // 0xB6
    u16 mB8_targetIndex;                // 0xB8
    s8 mBA_attackCount;                 // 0xBA
    u8 mBB_pad;                         // 0xBB
    s32* mBC_targetList;                // 0xBC
    s32 mC0_hasInitModel;               // 0xC0
    s32 mC4_hasNoTargets;               // 0xC4 (1 = no target list allocated)
    s32 mC8_doOrbitCamera;              // 0xC8
    s32 mCC_hasLightEffect;             // 0xCC
    s32 mD0_hasParticles;               // 0xD0
    s32 mD4_lightingRestored;           // 0xD4
    s32 mD8_hasCamera;                  // 0xD8
    s32 mDC_hasCameraInterp;            // 0xDC
    s32 mE0_hasCameraTarget;            // 0xE0
    s32 mE4_isReturning;               // 0xE4
    s32 mE8_hasCameraDesired;           // 0xE8
    s32 mEC_hasSecondaryAnim;           // 0xEC
    s32 mF0_hasLight;                   // 0xF0
    s32 mF4_hasPaletteFade;             // 0xF4
    s32 mF8_cameraMode;                // 0xF8 (0 = sin/cos orbit, nonzero = overlay camera)
    s32 mFC_cameraHeightMode;           // 0xFC
    s32 m100_cameraDistMode;            // 0x100
    s32 m104_animLoopFlag;              // 0x104
    s32 m108_scale;                     // 0x108
    s32 m10C_secondaryScale;            // 0x10C
    s32 m110_interpProgress;            // 0x110
    s32 m114_interpTarget;              // 0x114
    s32 m118_interpSpeed;               // 0x118
    s32 m11C_posDamping;                // 0x11C
    s32 m120_rotDamping;                // 0x120
    s32 m124_cameraDamping;             // 0x124
    s32 m128_cameraRotDamping;          // 0x128
    s32 m12C_damageBase;                // 0x12C
    sVec3_FP m130_cameraVelocity;       // 0x130
    sVec3_FP m13C_cameraAccel;          // 0x13C
    sVec3_FP m148_currentPos;           // 0x148 — model draw position / camera look-at
    sVec3_FP m154_retreatVelocity;      // 0x154
    sVec3_FP m160_retreatAccel;         // 0x160
    sVec3_FP m16C_retreatPos;           // 0x16C
    sVec3_FP m178_rotVelocity;          // 0x178
    sVec3_FP m184_rotAccel;             // 0x184
    sVec3_FP m190_currentRotation;      // 0x190 (28-bit angles)
    sVec3_FP m19C_cameraPosition;       // 0x19C — passed to engine as current cam pos
    sVec3_FP m1A8_attackBasePos;        // 0x1A8
    sVec3_FP m1B4_cameraTarget;         // 0x1B4 — camera look-at target
    sTrailEntry m1C0_trailA[2][16];     // 0x1C0 (2 sides × 16 slots × 0x24 = 0x480)
    sTrailEntry m640_trailB[2][16];     // 0x640 (2 sides × 16 slots × 0x24 = 0x480)
    sVec3_FP mAC0_vertexPos[2];         // 0xAC0 — bone 2 vertex 0, bone 4 vertex 0
    sVec3_FP mAD8_vertexPos2[2];        // 0xAD8 — bone 2 vertex 1, bone 4 vertex 1
    sVec3_FP mAF0_vertexCenter[2];      // 0xAF0 — bone 0 vertex 0, bone 0 vertex 1
    sVec3_FP mB08_lookAngle;            // 0xB08 (X=yaw, Y=pitch, Z=0)
    s32 mB14_rotation;                  // 0xB14
    s32 mB18_rotationAngle;             // 0xB18
    sPhoenixParticle mB1C_particles[8]; // 0xB1C (8 × 0x18 = 0xC0)
    s32 mBDC_secondaryTask;             // 0xBDC
    // size 0xBE0

    s_fileBundle* m_fileBundle;
    sHotpointBundle* m_hotpointBundle;
};

// Forward declaration
s32 drawLineSquareProject(const std::array<sVec3_FP, 4>& coordinates, sScreenQuad& projectedCoordinates);

// Gouraud color table for trail segments (16 entries, one per segment)
// 060ad384
static quadColor s_trailGouraudColors[16];
static bool s_trailGouraudColorsLoaded = false;

static void ensureTrailGouraudColors()
{
    if (!s_trailGouraudColorsLoaded)
    {
        sSaturnPtr ptr = g_BTL_A3->getSaturnPtr(0x060ad384);
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                s_trailGouraudColors[i][j] = readSaturnU16(ptr);
                ptr = ptr + 2;
            }
        }
        s_trailGouraudColorsLoaded = true;
    }
}

// 0602e8a4
static void dragonPhoenix_drawTrailQuadProjected(std::array<sVec3_FP, 4>& vertices, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    sScreenQuad projectedCoords;
    if (drawLineSquareProject(vertices, projectedCoords))
    {
        sScreenQuad3 screenQuad3;
        for (int i = 0; i < 4; i++)
        {
            screenQuad3[i].m0_X = projectedCoords[i][0];
            screenQuad3[i].m4_Y = projectedCoords[i][1];
            screenQuad3[i].m8_Z = vertices[i].m8_Z;
        }

        fixedPoint depth;
        depth.m_value = (vertices[0].m8_Z.m_value + vertices[1].m8_Z.m_value +
                         vertices[2].m8_Z.m_value + vertices[3].m8_Z.m_value) >> 2;

        sendRaySegmentToVdp1(screenQuad3, depth, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
    }
}

// 0602e83c
static void dragonPhoenix_drawTrailQuadTransformed(sVec3_FP* quadPts, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    std::array<sVec3_FP, 4> transformed;
    transformAndAddVecByCurrentMatrix(&quadPts[0], &transformed[0]);
    transformAndAddVecByCurrentMatrix(&quadPts[1], &transformed[1]);
    transformAndAddVecByCurrentMatrix(&quadPts[2], &transformed[2]);
    transformAndAddVecByCurrentMatrix(&quadPts[3], &transformed[3]);
    dragonPhoenix_drawTrailQuadProjected(transformed, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
}

// 06089c30
static void dragonPhoenix_drawTrails(sDragonPhoenixTask* pThis)
{
    ensureTrailGouraudColors();

    for (u8 side = 0; side < 2; side++)
    {
        s32 vertexArrayOffset = (s8)(side * 0xC); // byte offset into mAC0/mAD8 arrays (0 or 12)
        s32 trailSideOffset = (s16)((u16)side * 0x240); // byte offset for trail arrays (0 or 0x240)

        for (u16 segIdx = 0; segIdx < 16; segIdx++)
        {
            // Determine VDP1 rendering params based on whether this is the "next" segment
            u32 nextFrameIdx = performModulo(0x10, pThis->mB1_trailFrameIndex + 0x11);
            s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
            u16 vdp1Base = pEngine->mAA0_berzerkCustomTexture->m4_vdp1Memory;
            u16 characterAddress;
            u16 characterColor;
            if (segIdx == nextFrameIdx)
            {
                characterAddress = vdp1Base + 0x40;
                characterColor = 0x136C;
            }
            else
            {
                characterAddress = vdp1Base;
                characterColor = 0x1368;
            }
            s16 characterSize = 0x0420;

            // Build quad vertices: [0]=trailA curr, [1]=trailB curr, [2]=trailB next, [3]=trailA next
            sVec3_FP quadPts[4];

            if (segIdx == pThis->mB1_trailFrameIndex)
            {
                // Current write head: connect trail history to live vertex positions
                // quadPts[3] = mAC0_vertexPos[side]
                sVec3_FP* pVertexPos = (sVec3_FP*)((u8*)&pThis->mAC0_vertexPos[0] + vertexArrayOffset);
                quadPts[3] = *pVertexPos;

                // quadPts[0] = trailA[side][mB1].m18_position
                u8 frameIdx = pThis->mB1_trailFrameIndex;
                s16 slotOffset = (s16)(((u16)frameIdx * 8 + (u16)frameIdx) * 4); // frameIdx * 0x24
                sTrailEntry* pTrailA = (sTrailEntry*)((u8*)&pThis->m1C0_trailA[0][0] + trailSideOffset);
                sTrailEntry* pTrailB = (sTrailEntry*)((u8*)&pThis->m640_trailB[0][0] + trailSideOffset);
                sTrailEntry* pEntryA = (sTrailEntry*)((u8*)pTrailA + slotOffset);
                sTrailEntry* pEntryB = (sTrailEntry*)((u8*)pTrailB + slotOffset);

                quadPts[0] = pEntryA->m18_position;

                // quadPts[1] = trailB[side][mB1].m18_position
                quadPts[1] = pEntryB->m18_position;

                // quadPts[2] = mAD8_vertexPos2[side]
                sVec3_FP* pVertexPos2 = (sVec3_FP*)((u8*)&pThis->mAD8_vertexPos2[0] + vertexArrayOffset);
                quadPts[2] = *pVertexPos2;
            }
            else
            {
                // History segments: connect two consecutive trail entries
                s16 nextIdx = (s16)performModulo(0x10, (s32)(s16)segIdx + 0x11); // (segIdx+1) % 16
                s16 nextOffset = (s16)(nextIdx * 0x24);

                s16 currIdx = (s16)performModulo(0x10, (s32)(s16)segIdx + 0x10); // segIdx % 16
                s16 currOffset = (s16)(currIdx * 0x24);

                sTrailEntry* pTrailA = (sTrailEntry*)((u8*)&pThis->m1C0_trailA[0][0] + trailSideOffset);
                sTrailEntry* pTrailB = (sTrailEntry*)((u8*)&pThis->m640_trailB[0][0] + trailSideOffset);

                // quadPts[3] = trailA[nextIdx].m18_position
                sTrailEntry* pNextA = (sTrailEntry*)((u8*)pTrailA + nextOffset);
                quadPts[3] = pNextA->m18_position;

                // quadPts[0] = trailA[currIdx].m18_position
                sTrailEntry* pCurrA = (sTrailEntry*)((u8*)pTrailA + currOffset);
                quadPts[0] = pCurrA->m18_position;

                // quadPts[1] = trailB[currIdx].m18_position
                sTrailEntry* pCurrB = (sTrailEntry*)((u8*)pTrailB + currOffset);
                quadPts[1] = pCurrB->m18_position;

                // quadPts[2] = trailB[nextIdx].m18_position
                sTrailEntry* pNextB = (sTrailEntry*)((u8*)pTrailB + nextOffset);
                quadPts[2] = pNextB->m18_position;
            }

            // Compute distance from current write index (circular buffer distance)
            s16 dist;
            if ((s16)pThis->mB1_trailFrameIndex < (s16)segIdx)
            {
                dist = segIdx - pThis->mB1_trailFrameIndex;
            }
            else
            {
                dist = (segIdx - pThis->mB1_trailFrameIndex) + 0x10;
            }

            // Modulate quadPts[1] towards quadPts[0] using sine wave
            {
                sVec3_FP diff;
                diff.m0_X.m_value = quadPts[1].m0_X.m_value - quadPts[0].m0_X.m_value;
                diff.m4_Y.m_value = quadPts[1].m4_Y.m_value - quadPts[0].m4_Y.m_value;
                diff.m8_Z.m_value = quadPts[1].m8_Z.m_value - quadPts[0].m8_Z.m_value;

                s32 angle = (dist - 1) * 0x1555555 + pThis->mB18_rotationAngle;
                fixedPoint sinVal = getSin((angle >> 16) & 0xFFF);
                fixedPoint halfSin = MTH_Mul(sinVal, fixedPoint(0x8000));
                fixedPoint scale;
                scale.m_value = halfSin.m_value + 0x10000;

                quadPts[1].m0_X.m_value = quadPts[0].m0_X.m_value + MTH_Mul(diff.m0_X, scale).m_value;
                quadPts[1].m4_Y.m_value = quadPts[0].m4_Y.m_value + MTH_Mul(diff.m4_Y, scale).m_value;
                quadPts[1].m8_Z.m_value = quadPts[0].m8_Z.m_value + MTH_Mul(diff.m8_Z, scale).m_value;
            }

            // Modulate quadPts[2] towards quadPts[3] (only if dist != 0)
            if (dist != 0)
            {
                sVec3_FP diff;
                diff.m0_X.m_value = quadPts[2].m0_X.m_value - quadPts[3].m0_X.m_value;
                diff.m4_Y.m_value = quadPts[2].m4_Y.m_value - quadPts[3].m4_Y.m_value;
                diff.m8_Z.m_value = quadPts[2].m8_Z.m_value - quadPts[3].m8_Z.m_value;

                s32 angle = dist * 0x1555555 + pThis->mB18_rotationAngle;
                fixedPoint sinVal = getSin((angle >> 16) & 0xFFF);
                fixedPoint halfSin = MTH_Mul(sinVal, fixedPoint(0x8000));
                fixedPoint scale;
                scale.m_value = halfSin.m_value + 0x10000;

                quadPts[2].m0_X.m_value = quadPts[3].m0_X.m_value + MTH_Mul(diff.m0_X, scale).m_value;
                quadPts[2].m4_Y.m_value = quadPts[3].m4_Y.m_value + MTH_Mul(diff.m4_Y, scale).m_value;
                quadPts[2].m8_Z.m_value = quadPts[3].m8_Z.m_value + MTH_Mul(diff.m8_Z, scale).m_value;
            }

            // Draw the trail quad
            dragonPhoenix_drawTrailQuadTransformed(quadPts, characterAddress, characterSize, characterColor, &s_trailGouraudColors[segIdx], 0x0C);
        }
    }
}

// Static VDP1 quad data for particles
static std::vector<sVdp1Quad> s_phoenixParticleQuad;  // 060acfb4
static std::vector<sVdp1Quad> s_phoenixOrbQuad;       // 060ad120
static std::vector<sVdp1Quad> s_phoenixHitQuad;       // 060ae7f0

static void ensurePhoenixQuadData()
{
    if (s_phoenixParticleQuad.empty())
        s_phoenixParticleQuad = initVdp1Quad(g_BTL_A3->getSaturnPtr(0x060acfb4));
    if (s_phoenixOrbQuad.empty())
        s_phoenixOrbQuad = initVdp1Quad(g_BTL_A3->getSaturnPtr(0x060ad120));
    if (s_phoenixHitQuad.empty())
        s_phoenixHitQuad = initVdp1Quad(g_BTL_A3->getSaturnPtr(0x060ae7f0));
}

// 0608a6cc — extract vertex position from model bone hotpoint
static void dragonPhoenix_getModelVertexPos(s_3dModel* pModel, u32 boneIndex, u32 vertexIndex, sVec3_FP* pOut)
{
    if (pModel->m44_hotpointData.size() > boneIndex && pModel->m44_hotpointData[boneIndex].size() > vertexIndex)
    {
        transformAndAddVec(pModel->m44_hotpointData[boneIndex][vertexIndex], *pOut, cameraProperties2.m28[0]);
    }
}

// 06089b6e — check animation state for model1
static s32 dragonPhoenix_checkAnimState(sDragonPhoenixTask* pThis)
{
    s16 frame = pThis->m0C_model1.m16_previousAnimationFrame;
    if (frame == 0)
        return 0;
    if (frame == 0x1D)
    {
        if (pThis->m104_animLoopFlag != 0 && pThis->m104_animLoopFlag == 1)
            return 2;
    }
    else if (frame != 0x3B)
    {
        return 3;
    }
    return 1;
}

// 0608a198 — update camera look-at angles from current position vs camera target
static void dragonPhoenix_updateLookAt(sDragonPhoenixTask* pThis)
{
    sVec3_FP delta;
    delta.m0_X = pThis->m148_currentPos.m0_X - pThis->m1B4_cameraTarget.m0_X;
    delta.m4_Y = pThis->m148_currentPos.m4_Y - pThis->m1B4_cameraTarget.m4_Y;
    delta.m8_Z = pThis->m148_currentPos.m8_Z - pThis->m1B4_cameraTarget.m8_Z;

    fixedPoint distSq = MTH_Mul(delta.m0_X, delta.m0_X) + MTH_Mul(delta.m4_Y, delta.m4_Y) + MTH_Mul(delta.m8_Z, delta.m8_Z);
    fixedPoint threshold = MTH_Mul(fixedPoint(0x1000), fixedPoint(0x1000));
    if (threshold < distSq)
    {
        sVec3_FP dir;
        dir.m0_X = pThis->m1B4_cameraTarget.m0_X - pThis->m148_currentPos.m0_X;
        dir.m4_Y = pThis->m1B4_cameraTarget.m4_Y - pThis->m148_currentPos.m4_Y;
        dir.m8_Z = pThis->m1B4_cameraTarget.m8_Z - pThis->m148_currentPos.m8_Z;
        sVec2_FP angles;
        computeLookAt(dir, angles);
        pThis->mB08_lookAngle.m0_X = angles[0];
        pThis->mB08_lookAngle.m4_Y = angles[1];
        pThis->mB08_lookAngle.m8_Z = fixedPoint(0);
    }
}

// 060894c4 — camera look-at update with timer-based state transition
static void dragonPhoenix_updateLookAtWithTransition(sDragonPhoenixTask* pThis, u16 startTime, u16 endTime, s8 nextSubState)
{
    if (startTime < pThis->mB2_subTimer)
    {
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        sVec3_FP dir;
        dir.m0_X = pEngine->mC_battleCenter.m0_X - pThis->m148_currentPos.m0_X;
        dir.m4_Y = pEngine->mC_battleCenter.m4_Y - pThis->m148_currentPos.m4_Y;
        dir.m8_Z = pEngine->mC_battleCenter.m8_Z - pThis->m148_currentPos.m8_Z;
        sVec2_FP angles;
        computeLookAt(dir, angles);
        pThis->mB08_lookAngle.m0_X = angles[0];
        pThis->mB08_lookAngle.m4_Y = angles[1];
        pThis->mB08_lookAngle.m8_Z = fixedPoint(0);
        if (endTime < pThis->mB2_subTimer)
        {
            pThis->mAF_subState = nextSubState;
        }
    }
    else
    {
        dragonPhoenix_updateLookAt(pThis);
    }
}

// 0608a26a — reset animation and reinit from file bundle
static void dragonPhoenix_resetAndInitAnim(sDragonPhoenixTask* pThis, s16 animOffset)
{
    if (animOffset == 0xA4)
        pThis->m104_animLoopFlag = 1;
    else
        pThis->m104_animLoopFlag = 0;

    // Reset model animation state
    s_3dModel* pModel = &pThis->m0C_model1;
    if ((pModel->mA_animationFlags & 0x38) != 0)
    {
        pModel->m10_currentAnimationFrame = 0;
        u16 animType = *(u16*)pModel->m30_pCurrentAnimation & 7;
        if (animType == 1 || animType == 4 || animType == 5)
        {
            for (int i = 0; i < pModel->m12_numBones; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    pModel->m2C_poseData[i].m48[j].currentStep = 0;
                    pModel->m2C_poseData[i].m48[j].delay = 0;
                    pModel->m2C_poseData[i].m48[j].value = 0;
                }
            }
        }
        stepAnimation(pModel);
    }

    // Reinit animation from file bundle at given offset
    s_fileBundle* pBundle = (s_fileBundle*)pThis->m0_fileBundle;
    initAnimation(pModel, pBundle->getAnimation(animOffset));
}

// 0608a2c8 — compute random target position relative to camera offset quadrant
static void dragonPhoenix_computeRandomTargetPos(s8 quadrant, sVec3_FP* pOut)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sVec3_FP camOffset = readOverlayCameraOffset(pEngine, quadrant);

    // Each quadrant scatters the camera target differently.
    // dx/dz map to camOffset.Z/X respectively (swapped axes).
    switch (quadrant)
    {
    case 0:
    {
        s32 dx = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x19000)) + 0x5000;
        s32 dy = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x2D000));
        s32 dz = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x3C000)) - 0x1E000;
        camOffset.m8_Z = (s32)camOffset.m8_Z + dx;
        camOffset.m4_Y = (s32)camOffset.m4_Y + dy + 0x5000;
        camOffset.m0_X = (s32)camOffset.m0_X + dz;
        break;
    }
    case 1:
    {
        s32 dx = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x3C000)) - 0x1E000;
        s32 dy = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x2D000));
        s32 dz = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x19000)) + 0x5000;
        camOffset.m8_Z = (s32)camOffset.m8_Z + dx;
        camOffset.m4_Y = (s32)camOffset.m4_Y + dy + 0x5000;
        camOffset.m0_X = (s32)camOffset.m0_X + dz;
        break;
    }
    case 2:
    {
        s32 dx = -((s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x19000)) + 0x5000);
        s32 dy = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x2D000));
        s32 dz = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x3C000)) - 0x1E000;
        camOffset.m8_Z = (s32)camOffset.m8_Z + dx;
        camOffset.m4_Y = (s32)camOffset.m4_Y + dy + 0x5000;
        camOffset.m0_X = (s32)camOffset.m0_X + dz;
        break;
    }
    case 3:
    {
        camOffset.m8_Z = (s32)camOffset.m8_Z + (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x3C000)) - 0x1E000;
        camOffset.m4_Y = (s32)camOffset.m4_Y + (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x2D000)) + 0x5000;
        camOffset.m0_X = (s32)camOffset.m0_X - ((s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0)) + 0x1E000);
        break;
    }
    default:
        break;
    }

    // Clamp Y to camera max altitude
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    if (pTargetSystem->m204_cameraMaxAltitude < (s32)(pEngine->mC_battleCenter.m4_Y + camOffset.m4_Y))
    {
        camOffset.m4_Y = (pTargetSystem->m204_cameraMaxAltitude - (s32)pEngine->mC_battleCenter.m4_Y) - 0x5000;
    }

    *pOut = camOffset;
}

// 0608a700 — create wing fire particle sub-task (0x60C WithCopy, 0x23 particles)
static p_workArea dragonPhoenix_createFireSubTask(sDragonPhoenixTask* pThis)
{
    Unimplemented(); // TODO: implement wing fire subtask creation
    return nullptr;
}

// 0608ac7c — compute secondary scale from camera distance
static void dragonPhoenix_computeSecondaryScale(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sVec3_FP camOffset = readOverlayCameraOffset(pEngine, pThis->mAD_cameraQuadrant);
    fixedPoint distSq = MTH_Mul(camOffset.m0_X, camOffset.m0_X) +
                        MTH_Mul(camOffset.m4_Y, camOffset.m4_Y) +
                        MTH_Mul(camOffset.m8_Z, camOffset.m8_Z);
    fixedPoint dist = sqrt_F(distSq);
    pThis->m10C_secondaryScale = (s32)dist >> 2;
    if (pThis->m10C_secondaryScale > 0x30000)
        pThis->m10C_secondaryScale = 0x30000;
}

// 0608ab94 — compute direction vector between two points, scaled by 0x5000
static void dragonPhoenix_computeScaledDirection(sVec3_FP* pOut, sVec3_FP* pFrom, sVec3_FP* pTo)
{
    sVec3_FP dir;
    dir.m0_X = pFrom->m0_X - pTo->m0_X;
    dir.m4_Y = pFrom->m4_Y - pTo->m4_Y;
    dir.m8_Z = pFrom->m8_Z - pTo->m8_Z;
    sVec2_FP angles;
    computeLookAt(dir, angles);
    fixedPoint cosX = getCos(((u32)angles[0] >> 16) & 0xFFF);
    fixedPoint sinY = getSin(((u32)angles[1] >> 16) & 0xFFF);
    fixedPoint sinX = getSin(((u32)angles[0] >> 16) & 0xFFF);
    fixedPoint cosY = getCos(((u32)angles[1] >> 16) & 0xFFF);
    pOut->m0_X = MTH_Mul_5_6(fixedPoint(cosX), fixedPoint(sinY), fixedPoint(0x5000));
    pOut->m4_Y = MTH_Mul(fixedPoint(-sinX), fixedPoint(0x5000));
    pOut->m8_Z = MTH_Mul_5_6(fixedPoint(cosX), fixedPoint(cosY), fixedPoint(0x5000));
}

// 06089ba4 — apply phoenix damage to an enemy
static void dragonPhoenix_applyDamage(u16 enemyIndex, s16 baseDamage, sVec3_FP* pHitPos)
{
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleEnemy* pEnemy = pTargetSystem->m0_enemyTargetables[enemyIndex];
    s16 damage = phantomSlasher_getBaseDamage(baseDamage);
    damage = (s16)sGunShotTask_UpdateSub1Sub2(pEnemy->m4_targetable, damage, 2);
    applyDamageToEnnemy(pEnemy->m4_targetable, damage, pHitPos, 3, *pHitPos, fixedPoint(0x2000));
}

// Helper: check if enemy is targetable
static bool dragonPhoenix_isEnemyTargetable(s32 index)
{
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleEnemy* pEnemy = pTargetSystem->m0_enemyTargetables[index];
    if (pEnemy->m4_targetable == nullptr) return false;
    if ((pEnemy->m4_targetable->m50_flags & 0x140003) != 0) return false;
    if (pEnemy->m0_isActive <= 0) return false;
    return true;
}

// BTL_A3::0608a58c — VDP2 background toggle + palette fade (used by Dragon Phoenix)
// param_1: 0=disable background (darken), 1=enable background (restore)
// param_2: fade duration
static void dragonPhoenix_restoreLighting(s32 mode, s32 duration)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m24_fade1, 0xC210, 0xC210, 1);

    u32 fadeFrom, fadeTo;
    if (mode == 0)
    {
        // Darken: disable VDP2 background
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
        vdp2Controls.m_isDirty = 1;
        // Pause env task if present (grid flag 0x10 = has env task)
        if ((pGrid->m1C8_flags & 0x10) != 0)
        {
            s_BTL_A3_Env* envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
            p_workArea envSubTask = (p_workArea)envTask->m58;
            if (envSubTask != nullptr)
            {
                envSubTask->getTask()->m14_flags |= 2;
            }
        }
        pGrid->m1C8_flags |= 0x40;
        fadeFrom = 0x8002;
        fadeTo = 0xC210;
    }
    else if (mode == 1)
    {
        // Restore: enable VDP2 background
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
        vdp2Controls.m_isDirty = 1;
        // Unpause env task if present
        if ((pGrid->m1C8_flags & 0x10) != 0)
        {
            s_BTL_A3_Env* envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
            p_workArea envSubTask = (p_workArea)envTask->m58;
            if (envSubTask != nullptr)
            {
                envSubTask->getTask()->m14_flags &= ~2;
            }
        }
        pGrid->m1C8_flags &= ~0x40;
        fadeFrom = 0x8002;
        fadeTo = 0xC210;
    }
    else
    {
        g_fadeControls.m_4D = 5;
        return;
    }

    fadePalette(&g_fadeControls.m0_fade0, fadeFrom, fadeTo, duration);
    g_fadeControls.m_4D = 5;
}

// 06086a58 — Draw model 2 (aura/explosion sphere) at battle center with secondary scale + custom lighting
static void sDragonPhoenixTask_Draw2(sDragonPhoenixTask* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    sVec3_FP savedLight;
    if (pThis->mF0_hasLight != 0)
    {
        pushProjectionStack();
        battleEngine_UpdateSub7Sub1Sub0(&pGrid->m280_lightAngle1, savedLight);
        setupLight(savedLight.m0_X, savedLight.m4_Y, savedLight.m8_Z, 0);
        generateLightFalloffMap(0x101010, 0, 0);
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pEngine->mC_battleCenter);
    scaleCurrentMatrixRow0(pThis->m10C_secondaryScale);
    scaleCurrentMatrixRow1(pThis->m10C_secondaryScale);
    scaleCurrentMatrixRow2(pThis->m10C_secondaryScale);
    pThis->m5C_model2.m18_drawFunction(&pThis->m5C_model2);
    popMatrix();

    if (pThis->mF0_hasLight != 0)
    {
        // Restore lighting from grid light colors (extract integer part as RGB bytes)
        u32 lightColor = ((u32)(u8)pGrid->m1CC_lightColor.m8_Z.getInteger() << 16) |
                         ((u32)(u8)pGrid->m1CC_lightColor.m4_Y.getInteger() << 8) |
                         ((u32)(u8)pGrid->m1CC_lightColor.m0_X.getInteger());
        setupLight(savedLight.m0_X, savedLight.m4_Y, savedLight.m8_Z, lightColor);

        u32 falloff1 = ((u32)(u8)pGrid->m1E4_lightFalloff0.m8_Z.getInteger() << 16) |
                       ((u32)(u8)pGrid->m1E4_lightFalloff0.m4_Y.getInteger() << 8) |
                       ((u32)(u8)pGrid->m1E4_lightFalloff0.m0_X.getInteger());
        u32 falloff2 = ((u32)(u8)pGrid->m1FC_lightFalloff1.m8_Z.getInteger() << 16) |
                       ((u32)(u8)pGrid->m1FC_lightFalloff1.m4_Y.getInteger() << 8) |
                       ((u32)(u8)pGrid->m1FC_lightFalloff1.m0_X.getInteger());
        u32 falloff3 = ((u32)(u8)pGrid->m208_lightFalloff2.m8_Z.getInteger() << 16) |
                       ((u32)(u8)pGrid->m208_lightFalloff2.m4_Y.getInteger() << 8) |
                       ((u32)(u8)pGrid->m208_lightFalloff2.m0_X.getInteger());
        generateLightFalloffMap(falloff1, falloff2, falloff3);

        dragonFieldTaskDrawSub3Sub1();

        // Compute light position from rotation angle
        sVec3_FP lightPos;
        lightPos.m0_X = MTH_Mul(getSin(((u32)pThis->mB14_rotation >> 16) & 0xFFF), fixedPoint(0x14000));
        lightPos.m8_Z = MTH_Mul(getCos(((u32)pThis->mB14_rotation >> 16) & 0xFFF), fixedPoint(0x14000));
        lightPos.m0_X = (s32)lightPos.m0_X + (s32)pEngine->mC_battleCenter.m0_X;
        lightPos.m4_Y = pEngine->mC_battleCenter.m4_Y;
        lightPos.m8_Z = (s32)lightPos.m8_Z + (s32)pEngine->mC_battleCenter.m8_Z;

        sVec3_FP viewPos;
        transformAndAddVecByCurrentMatrix(&lightPos, &viewPos);
        dragonFieldTaskDrawSub1Sub1(viewPos.m0_X, viewPos.m4_Y, viewPos.m8_Z, 0x1E000);
    }
}

// 06086836
static void sDragonPhoenixTask_Draw(sDragonPhoenixTask* pThis)
{
    // Draw model 1 at current position with rotation and scale
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m148_currentPos);
    rotateCurrentMatrixYXZ(&pThis->m190_currentRotation);
    scaleCurrentMatrixRow0(pThis->m108_scale);
    scaleCurrentMatrixRow1(pThis->m108_scale);
    scaleCurrentMatrixRow2(pThis->m108_scale);
    pThis->m0C_model1.m18_drawFunction(&pThis->m0C_model1);
    popMatrix();

    // Extract vertex positions from model bones for trail rendering
    // Loop runs once: iVar3=0 then iVar3=2 fails <2
    for (int i = 0; i < 2; i += 2)
    {
        u32 bone0 = (i == 0) ? 2 : 4;
        u32 bone1 = ((i + 1) == 0) ? 2 : 4;
        dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone0, 0, &pThis->mAC0_vertexPos[i / 2]);
        dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone0, 1, &pThis->mAD8_vertexPos2[i / 2]);
        dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone1, 0, &pThis->mAC0_vertexPos[(i / 2) + 1]);
        dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone1, 1, &pThis->mAD8_vertexPos2[(i / 2) + 1]);
    }
    // Bone 0 center vertices
    dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, 0, 0, &pThis->mAF0_vertexCenter[0]);
    dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, 0, 1, &pThis->mAF0_vertexCenter[1]);

    // Draw particles if active
    if (pThis->mD0_hasParticles != 0)
    {
        // Draw trail ribbons
        dragonPhoenix_drawTrails(pThis);

        // Draw active particles (lifetime < 0 means active)
        for (int i = 0; i < 8; i += 4)
        {
            for (int j = 0; j < 4; j++)
            {
                s32 idx = i + j;
                if (idx < 8 && pThis->mB1C_particles[idx].m14_lifetime < 0)
                {
                    drawProjectedParticle(&pThis->mB1C_particles[idx].m0_quad,
                                          &pThis->mB1C_particles[idx].m8_position);
                }
            }
        }
    }
}

// 06086cc0
static void sDragonPhoenixTask_Delete(sDragonPhoenixTask* pThis)
{
    if (pThis->mD8_hasCamera != 0)
    {
        battleGrid_setCameraFov(0x238E38E);
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if (pThis->mF4_hasPaletteFade != 0)
    {
        g_fadeControls.m_4D = 6;
        if ((s8)g_fadeControls.m_4C < 7)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
        }
        fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xC210, 5);
        g_fadeControls.m_4D = 5;
    }
    if (pThis->mCC_hasLightEffect != 0)
    {
        // BTL_A3::0608a58c — restore VDP2 background + palette
        dragonPhoenix_restoreLighting(1, 3);
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// 06086d9c — State 0: Approach
// Phoenix rises from the player dragon and flies toward the enemy side.
//   substate 0: Wait for file load, init models + camera + target list
//   substate 1: Rise animation — camera follows phoenix, wait for anim frame 0x3B
//   substate 2: Fly toward enemies — orbit camera, wait for anim loop, reset anim
//   substate 3: Face enemies — look-at enemy center, init lighting/trails/particles at timer 0x1E,
//               transition to attack state at timer 0x44
static void dragonPhoenix_stateApproach(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    switch (pThis->mAF_subState)
    {
    case 0: // Load files, init models/camera/targets
    {
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
            return;

        // Store file bundle
        pThis->m_fileBundle = new s_fileBundle(pEngine->mA9C_berzerkCustomModel);
        pThis->m_fileBundle->setPatchFilePointers(pEngine->mAA0_berzerkCustomTexture->m4_vdp1Memory);
        pThis->m0_fileBundle = pThis->m_fileBundle;
        s_fileBundle* pBundle = pThis->m0_fileBundle;

        // Init model 1 (phoenix body) with hotpoint data for trail vertex extraction
        if (g_BTL_A3 == NULL)
        {
            g_BTL_A3 = new BTL_A3_data();
        }
        pThis->m_hotpointBundle = new sHotpointBundle(g_BTL_A3->getSaturnPtr(0x060acf2c));
        bool model1OK = init3DModelRawData(pThis, &pThis->m0C_model1, 0, pBundle, 4,
            pBundle->getAnimation(0xA8),
            pBundle->getStaticPose(0x9C, pBundle->getModelHierarchy(4)->countNumberOfBones()),
            nullptr, pThis->m_hotpointBundle);

        // Init model 2 (phoenix aura)
        bool model2OK = init3DModelRawData(pThis, &pThis->m5C_model2, 0, pBundle, 8,
            pBundle->getAnimation(0xAC),
            pBundle->getStaticPose(0xA0, pBundle->getModelHierarchy(8)->countNumberOfBones()),
            nullptr, nullptr);

        if (!model1OK || !model2OK)
        {
            pThis->getTask()->markFinished();
            return;
        }

        pThis->mC0_hasInitModel = 1;
        pThis->mDC_hasCameraInterp = 1;
        pThis->mE0_hasCameraTarget = 1;
        pThis->mD4_lightingRestored = 1;

        // Check if dragon rotation Y is within range for orbit camera
        s32 dragonRotY = signExtend28((s32)pDragon->m14_rotation.m4_Y);
        if (dragonRotY > -0x2000000 && dragonRotY < 0x2000000 &&
            pEngine->m22C_dragonCurrentQuadrant != 0)
        {
            pThis->mC8_doOrbitCamera = 1;
        }

        // Allocate target shuffle array
        s16 numEnemies = pTargetSystem->m20A_numSelectableEnemies;
        if (numEnemies != 0)
        {
            pThis->mBC_targetList = (s32*)allocateHeapForTask(pThis, numEnemies * 4);
        }

        if (pThis->mBC_targetList == nullptr || numEnemies == 0)
        {
            pThis->mC4_hasNoTargets = 1;
        }
        else
        {
            // Init and shuffle target list (Fisher-Yates)
            for (s32 i = 0; i < numEnemies; i++)
                pThis->mBC_targetList[i] = i;
            for (s32 i = 0; i < numEnemies; i++)
            {
                s32 remaining = numEnemies - i;
                s32 j = i + (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(remaining));
                s32 tmp = pThis->mBC_targetList[i];
                pThis->mBC_targetList[i] = pThis->mBC_targetList[j];
                pThis->mBC_targetList[j] = tmp;
            }
        }

        // Init camera velocities and positions
        pThis->m130_cameraVelocity = {};
        pThis->m13C_cameraAccel = {};
        pThis->m148_currentPos = pDragon->m8_position;
        pThis->m178_rotVelocity = {};
        pThis->m184_rotAccel = {};
        pThis->m190_currentRotation = pDragon->m14_rotation;
        pThis->m1B4_cameraTarget = pDragon->m8_position;
        pThis->m1B4_cameraTarget.m4_Y = (s32)pThis->m1B4_cameraTarget.m4_Y + 0xF000;
        pThis->mB08_lookAngle = pDragon->m14_rotation;

        // Camera spring parameters
        pThis->m11C_posDamping = 0x3333;
        pThis->m120_rotDamping = 0x851E;
        pThis->m124_cameraDamping = 0x28F;
        pThis->m128_cameraRotDamping = 0x170A;

        // Set camera position
        if (pThis->mF8_cameraMode == 0)
        {
            // Sin/cos orbit: compute camera from dragon rotation
            s32 sinVal = (s32)getSin(((u32)(s32)pThis->mB14_rotation >> 16) & 0xFFF);
            s32 cosVal = (s32)getCos(((u32)(s32)pThis->mB14_rotation >> 16) & 0xFFF);
            pThis->m19C_cameraPosition.m0_X = MTH_Mul(fixedPoint(sinVal), fixedPoint(0x6000));
            pThis->m19C_cameraPosition.m8_Z = MTH_Mul(fixedPoint(cosVal), fixedPoint(0x6000));
            pThis->m19C_cameraPosition.m4_Y = fixedPoint(0);
            pThis->m19C_cameraPosition += pDragon->m8_position;
        }
        else
        {
            // Overlay camera: read position from overlay data
            pThis->mAD_cameraQuadrant = pEngine->m22C_dragonCurrentQuadrant;
            sVec3_FP camOffset = readOverlayCameraOffset(pEngine, pThis->mAD_cameraQuadrant);
            pThis->m19C_cameraPosition.m0_X = (s32)pEngine->mC_battleCenter.m0_X + (s32)camOffset.m0_X;
            pThis->m19C_cameraPosition.m4_Y = (s32)pEngine->mC_battleCenter.m4_Y + (s32)camOffset.m4_Y;
            pThis->m19C_cameraPosition.m8_Z = (s32)pEngine->mC_battleCenter.m8_Z + (s32)camOffset.m8_Z;
        }

        battleEngine_enableAttackCamera();
        battleGrid_setCameraFov(fixedPoint(0x31C71C7));
        battleEngine_setCurrentCameraPositionPointer(&pThis->m19C_cameraPosition);
        battleEngine_setDesiredCameraPositionPointer(&pThis->m148_currentPos);
        battleEngine_resetCameraInterpolation();
        pThis->mD8_hasCamera = 1;

        pThis->m_DrawMethod = sDragonPhoenixTask_Draw;

        pThis->mAF_subState++;
        break;
    }

    case 1: // Rise animation with camera tracking
    {
        if (pThis->mC4_hasNoTargets != 0)
        {
            // No targets — shrink and die
            pThis->m108_scale -= 0x666;
            if (pThis->m108_scale < 0x1999)
            {
                pThis->getTask()->markFinished();
            }
            return;
        }

        pThis->mB2_subTimer++;
        if (pThis->mB2_subTimer < 0x1F)
        {
            // Keep updating camera in sin/cos mode
            if (pThis->mF8_cameraMode == 0)
            {
                s32 sinVal = (s32)getSin(((u32)(s32)pThis->mB14_rotation >> 16) & 0xFFF);
                s32 cosVal = (s32)getCos(((u32)(s32)pThis->mB14_rotation >> 16) & 0xFFF);
                pThis->m19C_cameraPosition.m0_X = MTH_Mul(fixedPoint(sinVal), fixedPoint(0x6000));
                pThis->m19C_cameraPosition.m8_Z = MTH_Mul(fixedPoint(cosVal), fixedPoint(0x6000));
                pThis->m19C_cameraPosition.m4_Y = fixedPoint(0);
                pThis->m19C_cameraPosition += pDragon->m8_position;
            }
            return;
        }

        // Timer >= 0x1F: check anim state and transition
        s32 animState = dragonPhoenix_checkAnimState(pThis);
        if (animState != 1)
            return;

        dragonPhoenix_resetAndInitAnim(pThis, 0xA4);

        // Set target position for camera (quadrant+2)
        s8 quadrant = (s8)performModulo(4, (u8)pEngine->m22C_dragonCurrentQuadrant + 2);
        sVec3_FP camOffset = readOverlayCameraOffset(pEngine, quadrant);
        pThis->m1B4_cameraTarget.m0_X = (s32)camOffset.m0_X + (s32)pEngine->mC_battleCenter.m0_X;
        pThis->m1B4_cameraTarget.m4_Y = (s32)camOffset.m4_Y + (s32)pEngine->mC_battleCenter.m4_Y;
        pThis->m1B4_cameraTarget.m8_Z = (s32)camOffset.m8_Z + (s32)pEngine->mC_battleCenter.m8_Z;

        // Height offset based on camera mode flags
        s32 heightOffset;
        if (pThis->mFC_cameraHeightMode != 0)
        {
            heightOffset = -0x1000;
            pThis->m1B4_cameraTarget.m0_X = pDragon->m8_position.m0_X;
        }
        else if (pThis->mF8_cameraMode != 0)
        {
            heightOffset = 0x5000;
        }
        else if (pThis->m100_cameraDistMode != 0)
        {
            heightOffset = 0xA000;
        }
        else
        {
            heightOffset = 0x50000;
        }
        pThis->m1B4_cameraTarget.m4_Y = (s32)pThis->m148_currentPos.m4_Y + heightOffset;

        // Orbit camera setup
        if (pThis->mC8_doOrbitCamera != 0)
        {
            pThis->mDC_hasCameraInterp = 0;
            s8 orbitQuad = (s8)performModulo(4, (u8)pEngine->m22C_dragonCurrentQuadrant + 1);
            pThis->mAD_cameraQuadrant = orbitQuad;
            sVec3_FP orbOffset = readOverlayCameraOffset(pEngine, orbitQuad);
            pThis->m19C_cameraPosition.m0_X = (s32)pEngine->mC_battleCenter.m0_X + (s32)orbOffset.m0_X;
            pThis->m19C_cameraPosition.m4_Y = (s32)pEngine->mC_battleCenter.m4_Y + (s32)orbOffset.m4_Y;
            pThis->m19C_cameraPosition.m8_Z = (s32)pEngine->mC_battleCenter.m8_Z + (s32)orbOffset.m8_Z;
            battleEngine_resetCameraInterpolation();
        }

        pThis->mAF_subState++;
        pThis->mB2_subTimer = 0;
        break;
    }

    case 2: // Fly toward enemies, orbit camera, wait for anim loop
    {
        s32 animState = dragonPhoenix_checkAnimState(pThis);
        if ((animState & 0xFF) == 2)
            pThis->mD4_lightingRestored = 0;

        pThis->mB2_subTimer++;
        dragonPhoenix_updateLookAt(pThis);

        if (pThis->mC8_doOrbitCamera == 0 || pThis->mB2_subTimer > 7)
        {
            pThis->mDC_hasCameraInterp = 1;
        }
        else
        {
            // Camera velocity nudge during orbit
            pThis->m130_cameraVelocity.m4_Y = (s32)pThis->m130_cameraVelocity.m4_Y + 0x1000;
            pThis->m130_cameraVelocity.m8_Z = (s32)pThis->m130_cameraVelocity.m8_Z + (-0x4000);
        }

        if (pThis->mB2_subTimer > 0x1E && pThis->mD4_lightingRestored == 0)
        {
            pThis->mD4_lightingRestored = 1;
        }

        if (pThis->m104_animLoopFlag != 1)
            return;

        animState = dragonPhoenix_checkAnimState(pThis);
        if ((animState & 0xFF) != 1)
            return;

        dragonPhoenix_resetAndInitAnim(pThis, 0xA8);

        if (pThis->mC8_doOrbitCamera == 0)
        {
            // Compute random camera target
            u32 rnd = (u8)pEngine->m22C_dragonCurrentQuadrant;
            s32 rndOffset = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(3));
            s8 newQuad = (s8)performModulo(4, rnd + rndOffset + 3);
            pThis->mAD_cameraQuadrant = newQuad;
            sVec3_FP newOffset = readOverlayCameraOffset(pEngine, newQuad);
            pThis->m19C_cameraPosition.m0_X = (s32)newOffset.m0_X + (s32)pEngine->mC_battleCenter.m0_X;
            pThis->m19C_cameraPosition.m4_Y = (s32)newOffset.m4_Y + (s32)pEngine->mC_battleCenter.m4_Y;
            pThis->m19C_cameraPosition.m8_Z = (s32)newOffset.m8_Z + (s32)pEngine->mC_battleCenter.m8_Z;
            battleEngine_resetCameraInterpolation();
        }

        pThis->mB2_subTimer = 0;
        pThis->mAF_subState++;
        break;
    }

    case 3: // Face enemies, init lighting/trails/fire particles at t=0x1E, transition at t=0x44
    {
        // Look at enemy center
        sVec3_FP lookDir;
        lookDir.m0_X = pEngine->mC_battleCenter.m0_X - (s32)pThis->m148_currentPos.m0_X;
        lookDir.m4_Y = pEngine->mC_battleCenter.m4_Y - (s32)pThis->m148_currentPos.m4_Y;
        lookDir.m8_Z = pEngine->mC_battleCenter.m8_Z - (s32)pThis->m148_currentPos.m8_Z;
        sVec2_FP lookAngles;
        computeLookAt(lookDir, lookAngles);
        pThis->mB08_lookAngle.m0_X = lookAngles[0];
        pThis->mB08_lookAngle.m4_Y = lookAngles[1];
        pThis->mB08_lookAngle.m8_Z = fixedPoint(0);

        if (pThis->mB2_subTimer == 0x1E)
        {
            // Enable lighting effect
            dragonPhoenix_restoreLighting(0, 0x3C);

            // Create wing fire particle sub-task
            pThis->mBDC_secondaryTask = (s32)(uintptr_t)dragonPhoenix_createFireSubTask(pThis);

            // Zero trail vertex data
            for (int slot = 0; slot < 16; slot++)
            {
                for (int side = 0; side < 2; side++)
                {
                    pThis->m1C0_trailA[side][slot].m0_velocity = {};
                    pThis->m1C0_trailA[side][slot].mC_velocity2 = {};
                    pThis->m1C0_trailA[side][slot].m18_position = {};
                    pThis->m640_trailB[side][slot].m0_velocity = {};
                    pThis->m640_trailB[side][slot].mC_velocity2 = {};
                    pThis->m640_trailB[side][slot].m18_position = {};

                    // Get vertex positions from model
                    u32 bone = (side == 0) ? 2 : 4;
                    dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone, 0,
                        &pThis->m1C0_trailA[side][slot].m18_position);
                    dragonPhoenix_getModelVertexPos(&pThis->m0C_model1, bone, 0,
                        &pThis->m640_trailB[side][slot].m18_position);
                }
            }

            // Init particles
            ensurePhoenixQuadData();
            u16 vdp1Memory = pEngine->mAA0_berzerkCustomTexture->m4_vdp1Memory;
            for (int i = 0; i < 8; i += 2)
            {
                particleInitSub(&pThis->mB1C_particles[i].m0_quad, vdp1Memory, &s_phoenixParticleQuad);
                pThis->mB1C_particles[i].m14_lifetime = (s8)((s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0xD)) + 3);

                particleInitSub(&pThis->mB1C_particles[i + 1].m0_quad, vdp1Memory, &s_phoenixParticleQuad);
                pThis->mB1C_particles[i + 1].m14_lifetime = (s8)((s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0xD)) + 3);
            }

            pThis->mCC_hasLightEffect = 1;
            pThis->mD0_hasParticles = 1;
        }

        pThis->mB2_subTimer++;
        if (pThis->mB2_subTimer >= 0x44)
        {
            pThis->mAC_state++;
        }
        break;
    }

    default:
        break;
    }
}

// 06087ce8 — State 1: Attack
// Phoenix lunges at each enemy in turn, dealing damage on each pass.
//   substate 0: Init attack — set up retreat velocity, disable camera interp
//   substate 1: Find next valid target, fly toward it with spring physics
//   substate 2: Reached target — apply damage, set up random camera position
//   substate 3: Timer-based transition — apply more damage, create explosion/hit effects,
//               handle palette fade, then loop back to substate 1 for next target
//               or transition to retreat state when all targets hit
static void dragonPhoenix_stateAttack(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s16 numEnemies = pTargetSystem->m20A_numSelectableEnemies;

    switch (pThis->mAF_subState)
    {
    case 0: // Init attack — set up retreat velocity, disable camera interp
    {
        pThis->mB2_subTimer = 0;
        pThis->mDC_hasCameraInterp = 0;
        pThis->mE0_hasCameraTarget = 1;
        pThis->mE4_isReturning = 1;
        pThis->m11C_posDamping = 0;
        pThis->m154_retreatVelocity = {};
        pThis->m160_retreatAccel = {};
        pThis->m16C_retreatPos = pThis->m148_currentPos;
        pThis->m1A8_attackBasePos = pThis->m148_currentPos;
        // Subtract auto-scroll since these are frame-static
        pThis->m1A8_attackBasePos -= pEngine->m1A0_battleAutoScrollDelta;
        pThis->m118_interpSpeed = 0x28F;
        pThis->mAF_subState++;
        break;
    }

    case 1: // Find next valid target, fly toward it with spring physics
    {
        // Skip dead/invalid targets
        while (pThis->mB8_targetIndex < numEnemies)
        {
            s32 targetIdx = pThis->mBC_targetList[pThis->mB8_targetIndex];
            if (dragonPhoenix_isEnemyTargetable(targetIdx))
            {
                // Set target position
                sVec3_FP* enemyPos = getBattleTargetablePosition(
                    *pTargetSystem->m0_enemyTargetables[targetIdx]->m4_targetable);
                pThis->m1B4_cameraTarget = *enemyPos;

                // Apply velocity from previous iteration
                pThis->m160_retreatAccel += pThis->m154_retreatVelocity;

                // Dampen velocity
                fixedPoint dampX = MTH_Mul(pThis->m160_retreatAccel.m0_X, fixedPoint(0xCCC));
                fixedPoint dampY = MTH_Mul(pThis->m160_retreatAccel.m4_Y, fixedPoint(0xCCC));
                fixedPoint dampZ = MTH_Mul(pThis->m160_retreatAccel.m8_Z, fixedPoint(0xCCC));
                pThis->m160_retreatAccel.m0_X = (s32)pThis->m160_retreatAccel.m0_X - (s32)dampX;
                pThis->m160_retreatAccel.m4_Y = (s32)pThis->m160_retreatAccel.m4_Y - (s32)dampY;
                pThis->m160_retreatAccel.m8_Z = (s32)pThis->m160_retreatAccel.m8_Z - (s32)dampZ;

                // Apply to position
                pThis->m16C_retreatPos += pThis->m160_retreatAccel;
                pThis->m154_retreatVelocity = {};

                goto attack_movement;
            }
            pThis->mB8_targetIndex++;
        }
        // All targets exhausted
        pThis->mAF_subState = 0;
        pThis->mAC_state++; // -> retreat
        break;
    }

    case 2: // Reached target — apply damage, random camera position
    {
        pThis->mDC_hasCameraInterp = 1;
        pThis->m124_cameraDamping = 0x28F;
        pThis->m11C_posDamping = 0x3333;
        pThis->mAF_subState++;
        pThis->mB2_subTimer = 0;

        // Random camera target position
        u32 rnd = randomNumber();
        s32 quadrant = (s32)MTH_Mul(fixedPoint(rnd >> 16), fixedPoint(4));
        dragonPhoenix_computeRandomTargetPos((s8)quadrant, &pThis->m1B4_cameraTarget);
        pThis->m1B4_cameraTarget += pEngine->mC_battleCenter;
        // fall through to case 3
    }
    [[fallthrough]];

    case 3: // Timer — more damage, explosions, palette fade, loop or retreat
    {
        pThis->mB2_subTimer++;
        dragonPhoenix_updateLookAtWithTransition(pThis, 0x14, 0x4B, 0);
        return;
    }

    default:
        break;
    }
    return;

attack_movement:
    // Speed ramp
    if (pThis->mE4_isReturning == 0)
    {
        if (pThis->m118_interpSpeed > 0x7AE)
            pThis->m118_interpSpeed -= 0xA3;
    }
    else
    {
        if (pThis->m110_interpProgress < 0x4000)
            pThis->m118_interpSpeed += 0x20;
        else
            pThis->m118_interpSpeed = 0xF5C;
    }
    pThis->m110_interpProgress += pThis->m118_interpSpeed;

    // Interpolate position toward target
    fixedPoint interpT = fixedPoint(pThis->m110_interpProgress);
    sVec3_FP delta;
    delta.m0_X = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m0_X - (s32)pThis->m16C_retreatPos.m0_X), interpT);
    delta.m4_Y = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m4_Y - (s32)pThis->m16C_retreatPos.m4_Y), interpT);
    delta.m8_Z = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m8_Z - (s32)pThis->m16C_retreatPos.m8_Z), interpT);
    pThis->m13C_cameraAccel.m0_X = (s32)(pThis->m16C_retreatPos.m0_X + delta.m0_X) - (s32)pThis->m148_currentPos.m0_X;
    pThis->m13C_cameraAccel.m4_Y = (s32)(pThis->m16C_retreatPos.m4_Y + delta.m4_Y) - (s32)pThis->m148_currentPos.m4_Y;
    pThis->m13C_cameraAccel.m8_Z = (s32)(pThis->m16C_retreatPos.m8_Z + delta.m8_Z) - (s32)pThis->m148_currentPos.m8_Z;

    // Update model look-at
    dragonPhoenix_updateLookAt(pThis);

    // Auto-scroll base positions
    pThis->m16C_retreatPos += pEngine->m1A0_battleAutoScrollDelta;
    pThis->m1A8_attackBasePos += pEngine->m1A0_battleAutoScrollDelta;

    // Spring force toward target
    fixedPoint springX = MTH_Mul(fixedPoint((s32)pThis->m1A8_attackBasePos.m0_X - (s32)pThis->m16C_retreatPos.m0_X), fixedPoint(0x51E));
    fixedPoint springY = MTH_Mul(fixedPoint((s32)pThis->m1A8_attackBasePos.m4_Y - (s32)pThis->m16C_retreatPos.m4_Y), fixedPoint(0x51E));
    fixedPoint springZ = MTH_Mul(fixedPoint((s32)pThis->m1A8_attackBasePos.m8_Z - (s32)pThis->m16C_retreatPos.m8_Z), fixedPoint(0x51E));
    pThis->m154_retreatVelocity.m0_X = (s32)pThis->m154_retreatVelocity.m0_X + (s32)springX;
    pThis->m154_retreatVelocity.m4_Y = (s32)pThis->m154_retreatVelocity.m4_Y + (s32)springY;
    pThis->m154_retreatVelocity.m8_Z = (s32)pThis->m154_retreatVelocity.m8_Z + (s32)springZ;

    if (pThis->m110_interpProgress < 0x10001)
        return;

    // Reached target
    if (pThis->mE4_isReturning == 0)
        return;

    // Apply damage to current target
    s32 currentTarget = pThis->mBC_targetList[pThis->mB8_targetIndex];
    dragonPhoenix_applyDamage((u16)currentTarget, (s16)pThis->m12C_damageBase, &pThis->m1B4_cameraTarget);

    // Proximity damage to nearby enemies
    sVec3_FP diff;
    diff.m0_X = (s32)pThis->m1B4_cameraTarget.m0_X - (s32)pThis->m16C_retreatPos.m0_X;
    diff.m4_Y = (s32)pThis->m1B4_cameraTarget.m4_Y - (s32)pThis->m16C_retreatPos.m4_Y;
    diff.m8_Z = (s32)pThis->m1B4_cameraTarget.m8_Z - (s32)pThis->m16C_retreatPos.m8_Z;
    fixedPoint distToTarget = MTH_Mul(diff.m0_X, diff.m0_X) + MTH_Mul(diff.m4_Y, diff.m4_Y) + MTH_Mul(diff.m8_Z, diff.m8_Z);
    fixedPoint closeThreshold = MTH_Mul(fixedPoint(0x46000), fixedPoint(0x46000));
    if (distToTarget < closeThreshold)
        pThis->m114_interpTarget = 0x28000;
    else
        pThis->m114_interpTarget = 0x1CCCC;

    // Check all enemies for proximity damage
    for (s32 i = 0; i < numEnemies; i++)
    {
        if (i == currentTarget) continue;
        if (!dragonPhoenix_isEnemyTargetable(i)) continue;

        sVec3_FP* enemyPos = getBattleTargetablePosition(*pTargetSystem->m0_enemyTargetables[i]->m4_targetable);
        sVec3_FP eDiff;
        eDiff.m0_X = (s32)enemyPos->m0_X - (s32)pThis->m148_currentPos.m0_X;
        eDiff.m4_Y = (s32)enemyPos->m4_Y - (s32)pThis->m148_currentPos.m4_Y;
        eDiff.m8_Z = (s32)enemyPos->m8_Z - (s32)pThis->m148_currentPos.m8_Z;
        fixedPoint eDist = MTH_Mul(eDiff.m0_X, eDiff.m0_X) + MTH_Mul(eDiff.m4_Y, eDiff.m4_Y) + MTH_Mul(eDiff.m8_Z, eDiff.m8_Z);
        fixedPoint splashThreshold = MTH_Mul(fixedPoint(0x14000), fixedPoint(0x14000));
        if (eDist < splashThreshold)
        {
            s32 rndDmg = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0xBE)) + 10;
            dragonPhoenix_applyDamage((u16)i, (s16)rndDmg, enemyPos);
        }
        playSystemSoundEffect(0x1B);
    }

    // Advance to next target
    pThis->mE4_isReturning = 0;
    pThis->mB8_targetIndex++;
    if (pThis->mB8_targetIndex >= numEnemies)
    {
        // All targets hit — transition to retreat
        pThis->mAF_subState = 0;
        pThis->mAC_state++; // -> retreat
        return;
    }

    // Check if we should switch to retreat or continue attacking
    if ((s32)pThis->m114_interpTarget < pThis->m110_interpProgress)
    {
        pThis->m110_interpProgress = 0;
        u32 rnd = randomNumber();
        if ((rnd & 1) == 0 || pThis->mBA_attackCount > 3)
        {
            // Retreat
            pThis->mBA_attackCount = 0;
            pThis->mAF_subState = 2; // -> camera reposition
        }
        else
        {
            // Continue attacking next target
            pThis->m154_retreatVelocity = {};
            pThis->m160_retreatAccel = {};
            pThis->m16C_retreatPos = pThis->m148_currentPos;
            pThis->m154_retreatVelocity += pThis->m13C_cameraAccel;
            pThis->m1A8_attackBasePos = pThis->m148_currentPos;
            pThis->m1A8_attackBasePos -= pEngine->m1A0_battleAutoScrollDelta;
            pThis->mE4_isReturning = 1;
            pThis->mBA_attackCount++;
        }
    }
}

// 06088760 — phoenix retreat + restore
// 060888c8 — State 2: Retreat
// Phoenix flies up and away, crashes into the ground with an explosion.
//   substate 0: Init — choose retreat path (normal fly-up or fast fly-away if no enemies)
//   substate 1: Fly upward — interpolate toward high altitude, disable particles at peak
//   substate 2: Init crash — set up downward interpolation toward battle center
//   substate 3: Crash down — interpolate toward ground, apply damage on impact,
//               create expanding explosion sphere, trigger palette fade to white
//   substate 4: (unused)
//   substate 5: Fast fly-away — interpolate and shrink when no enemies remain
//   substate 6: Restore lighting
//   substate 7: Wait timer then mark task finished
static void dragonPhoenix_stateRetreat(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s16 numEnemies = pTargetSystem->m20A_numSelectableEnemies;

    switch (pThis->mAF_subState)
    {
    case 0: // Choose path — fly-up if enemies remain, fast fly-away if none
    {
        // Check if any enemies are still targetable
        bool hasTarget = false;
        for (s32 i = 0; i < numEnemies; i++)
        {
            if (dragonPhoenix_isEnemyTargetable(i))
            {
                hasTarget = true;
                break;
            }
        }

        if (hasTarget)
        {
            // Normal retreat — fly up toward camera target
            pThis->mDC_hasCameraInterp = 1;
            pThis->m124_cameraDamping = 0x28F;
            pThis->m11C_posDamping = 0x3333;
            pThis->mB2_subTimer = 0;
            pThis->m1B4_cameraTarget.m0_X = pEngine->mC_battleCenter.m0_X;
            pThis->m1B4_cameraTarget.m4_Y = (s32)pEngine->mC_battleCenter.m4_Y + 0xDC000;
            pThis->m1B4_cameraTarget.m8_Z = pEngine->mC_battleCenter.m8_Z;
            pThis->mAF_subState = 1;
        }
        else
        {
            // No enemies left — fly away quickly
            pThis->mDC_hasCameraInterp = 0;
            pThis->mAF_subState = 5;
            pThis->m118_interpSpeed = 0x11EB;
            pThis->m110_interpProgress = 0;
            pThis->m1B4_cameraTarget = pThis->m148_currentPos;
            s32 rndY = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x14000));
            pThis->m1B4_cameraTarget.m4_Y = (s32)pThis->m1B4_cameraTarget.m4_Y + rndY;
            if ((s32)pThis->m13C_cameraAccel.m8_Z < 1)
                pThis->m1B4_cameraTarget.m8_Z = (s32)pThis->m1B4_cameraTarget.m8_Z - 0x46000;
            else
                pThis->m1B4_cameraTarget.m8_Z = (s32)pThis->m1B4_cameraTarget.m8_Z + 0x46000;

            pThis->m154_retreatVelocity = {};
            pThis->m160_retreatAccel = {};
            pThis->m16C_retreatPos = pThis->m148_currentPos;
            pThis->m154_retreatVelocity += pThis->m13C_cameraAccel;
            pThis->m1A8_attackBasePos = pThis->m148_currentPos;
            pThis->m1A8_attackBasePos -= pEngine->m1A0_battleAutoScrollDelta;
        }
        break;
    }

    case 1: // Fly upward, disable particles at peak
    {
        if (pThis->m118_interpSpeed < 0x11EB)
            pThis->m118_interpSpeed += 0xA3;
        pThis->m110_interpProgress += pThis->m118_interpSpeed;

        // Auto-scroll
        pThis->m16C_retreatPos += pEngine->m1A0_battleAutoScrollDelta;
        pThis->m1A8_attackBasePos += pEngine->m1A0_battleAutoScrollDelta;

        // Interpolate position
        sVec3_FP interpDelta;
        interpDelta.m0_X = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m0_X - (s32)pThis->m16C_retreatPos.m0_X), fixedPoint(pThis->m110_interpProgress));
        interpDelta.m4_Y = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m4_Y - (s32)pThis->m16C_retreatPos.m4_Y), fixedPoint(pThis->m110_interpProgress));
        interpDelta.m8_Z = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m8_Z - (s32)pThis->m16C_retreatPos.m8_Z), fixedPoint(pThis->m110_interpProgress));
        pThis->m13C_cameraAccel.m0_X = (s32)(pThis->m16C_retreatPos.m0_X + interpDelta.m0_X) - (s32)pThis->m148_currentPos.m0_X;
        pThis->m13C_cameraAccel.m4_Y = (s32)(pThis->m16C_retreatPos.m4_Y + interpDelta.m4_Y) - (s32)pThis->m148_currentPos.m4_Y;
        pThis->m13C_cameraAccel.m8_Z = (s32)(pThis->m16C_retreatPos.m8_Z + interpDelta.m8_Z) - (s32)pThis->m148_currentPos.m8_Z;

        dragonPhoenix_updateLookAt(pThis);

        if (pThis->m110_interpProgress > 0x10000 && pThis->mE8_hasCameraDesired == 0)
        {
            pThis->mE8_hasCameraDesired = 1;
            battleEngine_setDesiredCameraPositionPointer(&pEngine->mC_battleCenter);
        }

        if ((s32)pThis->m148_currentPos.m4_Y < pTargetSystem->m204_cameraMaxAltitude ||
            pThis->m110_interpProgress > 0x20000)
        {
            pThis->mD0_hasParticles = 0;
            pThis->m0C_model1.m8 &= ~1;
            pThis->mB2_subTimer = 0;
            pThis->mCC_hasLightEffect = 0;
            if (pThis->mE8_hasCameraDesired == 0)
                battleEngine_setDesiredCameraPositionPointer(&pEngine->mC_battleCenter);
            dragonPhoenix_restoreLighting(1, 0xF);
            pThis->mAF_subState = 2;
        }
        break;
    }

    case 2: // Init downward crash
    {
        pThis->mB2_subTimer = 0;
        pThis->mDC_hasCameraInterp = 0;
        pThis->mE4_isReturning = 0;
        pThis->m11C_posDamping = 0;
        pThis->m118_interpSpeed = 0x3D7;
        pThis->m110_interpProgress = 0;
        pThis->m154_retreatVelocity = {};
        pThis->m160_retreatAccel = {};
        pThis->m16C_retreatPos = pThis->m148_currentPos;
        pThis->m1B4_cameraTarget = pEngine->mC_battleCenter;
        pThis->mAF_subState = 3;
        break;
    }

    case 3: // Crash down, explosion sphere, fade to white
    {
        pThis->mB2_subTimer++;
        if (pThis->mB2_subTimer == 3)
        {
            dragonPhoenix_computeSecondaryScale(pThis);
            pThis->m_DrawMethod = sDragonPhoenixTask_Draw2;
            pThis->mEC_hasSecondaryAnim = 1;
        }
        if (pThis->mB2_subTimer == 0xF)
        {
            // Start fade to white
            g_fadeControls.m_4D = 6;
            if ((s8)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 10;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 10;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xFFFF, 0x2D);
            g_fadeControls.m_4D = 5;
            pThis->mF4_hasPaletteFade = 1;
        }
        if (pThis->mB2_subTimer == 0x10)
        {
            playSystemSoundEffect(0x13);
        }
        // Per-enemy damage ticks at regular intervals
        if (pThis->mB2_subTimer == 0x16 || pThis->mB2_subTimer == 0x1C ||
            pThis->mB2_subTimer == 0x1F || pThis->mB2_subTimer == 0x22 ||
            pThis->mB2_subTimer == 0x23 || pThis->mB2_subTimer == 0x26 ||
            pThis->mB2_subTimer == 0x2D || pThis->mB2_subTimer == 0x30 ||
            pThis->mB2_subTimer == 0x34 || pThis->mB2_subTimer == 0x37 ||
            pThis->mB2_subTimer == 0x3D || pThis->mB2_subTimer == 0x41 ||
            pThis->mB2_subTimer == 0x48 || pThis->mB2_subTimer == 0x49 ||
            pThis->mB2_subTimer == 0x4B)
        {
            for (s32 i = 0; i < numEnemies; i++)
            {
                if (!dragonPhoenix_isEnemyTargetable(i)) continue;

                sVec3_FP* enemyPos = getBattleTargetablePosition(*pTargetSystem->m0_enemyTargetables[i]->m4_targetable);
                sVec3_FP eDiff;
                eDiff.m0_X = (s32)enemyPos->m0_X - (s32)pEngine->mC_battleCenter.m0_X;
                eDiff.m4_Y = (s32)enemyPos->m4_Y - (s32)pEngine->mC_battleCenter.m4_Y;
                eDiff.m8_Z = (s32)enemyPos->m8_Z - (s32)pEngine->mC_battleCenter.m8_Z;
                fixedPoint eDist = MTH_Mul(eDiff.m0_X, eDiff.m0_X) + MTH_Mul(eDiff.m4_Y, eDiff.m4_Y) + MTH_Mul(eDiff.m8_Z, eDiff.m8_Z);
                if (eDist < fixedPoint(0x2000))
                {
                    eDiff.m0_X = fixedPoint(0);
                    eDiff.m4_Y = fixedPoint(-0xA000);
                    eDiff.m8_Z = fixedPoint(0);
                }
                dragonPhoenix_applyDamage((u16)i, 0x19, enemyPos);
            }
        }
        if (pThis->mB2_subTimer == 0x17)
        {
            if (pThis->mBDC_secondaryTask != 0)
            {
                p_workArea pSecondary = (p_workArea)(uintptr_t)pThis->mBDC_secondaryTask;
                if (pSecondary != nullptr)
                    pSecondary->getTask()->markFinished();
            }
        }
        if (pThis->mB2_subTimer == 0x1A)
        {
            // Setup lighting
            pEngine->m188_flags.m20000 = 1;
            syncM68KSoundCPU();
            s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
            sVec3_FP lightColor;
            battleResultScreen_updateSub0(0x1F, 0x19, 0x19, &lightColor);
            battleGrid_setupLightInterpolation(0, *(sVec3_FP*)((u8*)pGrid + 0x1CC), lightColor);
            sVec3_FP ambientColor;
            battleResultScreen_updateSub0(0, 5, 5, &ambientColor);
            battleGrid_setupLightInterpolation2(10, *(sVec3_FP*)((u8*)pGrid + 0x1E4), ambientColor);
            pThis->mF0_hasLight = 1;
        }
        if (pThis->mB2_subTimer == 0x42)
        {
            // Fade back from white
            g_fadeControls.m_4D = 6;
            if ((s8)g_fadeControls.m_4C < 7)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 10;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 10;
            }
            fadePalette(&g_fadeControls.m0_fade0, 0xFFFF, 0xC210, 0x1E);
            g_fadeControls.m_4D = 5;
            pThis->mF4_hasPaletteFade = 0;
        }
        if (pThis->mB2_subTimer == 0x48 || pThis->mB2_subTimer == 0x49)
        {
            // Additional damage ticks at end (reuse same loop above)
        }
        if (pThis->mB2_subTimer == 0x4B)
        {
            // Final damage tick
        }
        if (pThis->mB2_subTimer == 0x4C)
        {
            pThis->mAF_subState = 6;
        }
        break;
    }

    case 4: // Unused/skipped
        break;

    case 5: // Fast fly-away (no enemies) — interpolate and fade
    {
        pThis->m110_interpProgress += pThis->m118_interpSpeed;

        sVec3_FP interpDelta;
        interpDelta.m0_X = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m0_X - (s32)pThis->m16C_retreatPos.m0_X), fixedPoint(pThis->m110_interpProgress));
        interpDelta.m4_Y = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m4_Y - (s32)pThis->m16C_retreatPos.m4_Y), fixedPoint(pThis->m110_interpProgress));
        interpDelta.m8_Z = MTH_Mul(fixedPoint((s32)pThis->m1B4_cameraTarget.m8_Z - (s32)pThis->m16C_retreatPos.m8_Z), fixedPoint(pThis->m110_interpProgress));
        pThis->m13C_cameraAccel.m0_X = (s32)(pThis->m16C_retreatPos.m0_X + interpDelta.m0_X) - (s32)pThis->m148_currentPos.m0_X;
        pThis->m13C_cameraAccel.m4_Y = (s32)(pThis->m16C_retreatPos.m4_Y + interpDelta.m4_Y) - (s32)pThis->m148_currentPos.m4_Y;
        pThis->m13C_cameraAccel.m8_Z = (s32)(pThis->m16C_retreatPos.m8_Z + interpDelta.m8_Z) - (s32)pThis->m148_currentPos.m8_Z;

        // Auto-scroll
        pThis->m16C_retreatPos += pEngine->m1A0_battleAutoScrollDelta;
        pThis->m1A8_attackBasePos += pEngine->m1A0_battleAutoScrollDelta;

        // Damped retreat
        pThis->m160_retreatAccel += pThis->m154_retreatVelocity;
        fixedPoint dampX = MTH_Mul(pThis->m160_retreatAccel.m0_X, fixedPoint(0x3333));
        fixedPoint dampY = MTH_Mul(pThis->m160_retreatAccel.m4_Y, fixedPoint(0x3333));
        fixedPoint dampZ = MTH_Mul(pThis->m160_retreatAccel.m8_Z, fixedPoint(0x3333));
        pThis->m160_retreatAccel.m0_X = (s32)pThis->m160_retreatAccel.m0_X - (s32)dampX;
        pThis->m160_retreatAccel.m4_Y = (s32)pThis->m160_retreatAccel.m4_Y - (s32)dampY;
        pThis->m160_retreatAccel.m8_Z = (s32)pThis->m160_retreatAccel.m8_Z - (s32)dampZ;
        pThis->m16C_retreatPos += pThis->m160_retreatAccel;
        pThis->m154_retreatVelocity = {};

        if (pThis->m110_interpProgress < 0x10000)
            dragonPhoenix_updateLookAt(pThis);

        if (pThis->m110_interpProgress > 0x40000)
        {
            pThis->mCC_hasLightEffect = 0;
            pThis->mD0_hasParticles = 0;
            dragonPhoenix_restoreLighting(1, 0xF);
            pThis->m0C_model1.m8 &= ~1;
            pThis->mAF_subState = 6;
        }
        break;
    }

    case 6: // Restore lighting
    {
        if (pThis->mF0_hasLight != 0)
        {
            s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
            battleGrid_setupLightInterpolation(10, *(sVec3_FP*)((u8*)pGrid + 0x1CC), *(sVec3_FP*)((u8*)pGrid + 0x1D8));
            battleGrid_setupLightInterpolation2(10, *(sVec3_FP*)((u8*)pGrid + 0x1E4), *(sVec3_FP*)((u8*)pGrid + 0x1F0));
            resetProjectVector();
            pThis->mF0_hasLight = 0;
        }
        pThis->mB2_subTimer = 0;
        pThis->mAF_subState = 7;
        break;
    }

    case 7: // Wait and finish
    {
        pThis->mB2_subTimer++;
        if (pThis->mB2_subTimer > 0x24)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }

    default:
        break;
    }

    // Secondary model animation step (runs every frame in retreat)
    if (pThis->mEC_hasSecondaryAnim != 0)
    {
        stepAnimation(&pThis->m5C_model2);
        if (pThis->m5C_model2.m16_previousAnimationFrame == 0x27)
        {
            pThis->mEC_hasSecondaryAnim = 0;
            // Clear draw method for secondary model
        }
    }
}

// 06089580
static void dragonPhoenix_updateTrailParticles(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // Advance trail frame index (circular buffer of 16)
    pThis->mB1_trailFrameIndex++;
    if (pThis->mB1_trailFrameIndex == 0x10)
        pThis->mB1_trailFrameIndex = 0;

    u8 idx = pThis->mB1_trailFrameIndex;

    for (u8 side = 0; side < 2; side++)
    {
        // Zero current slot velocity/velocity2 for trailA
        pThis->m1C0_trailA[side][idx].m0_velocity = {};
        pThis->m1C0_trailA[side][idx].mC_velocity2 = {};
        pThis->m1C0_trailA[side][idx].m18_position = pThis->mAC0_vertexPos[side];

        // Zero current slot velocity/velocity2 for trailB
        pThis->m640_trailB[side][idx].m0_velocity = {};
        pThis->m640_trailB[side][idx].mC_velocity2 = {};
        pThis->m640_trailB[side][idx].m18_position = pThis->mAD8_vertexPos2[side];

        // Compute scaled direction and add to velocity
        sVec3_FP dirA, dirB;
        dragonPhoenix_computeScaledDirection(&dirA, &pThis->mAF0_vertexCenter[0], &pThis->mAC0_vertexPos[side]);
        dragonPhoenix_computeScaledDirection(&dirB, &pThis->mAF0_vertexCenter[1], &pThis->mAD8_vertexPos2[side]);

        pThis->m1C0_trailA[side][idx].m0_velocity += dirA;
        pThis->m640_trailB[side][idx].m0_velocity += dirB;

        // Velocity integration for all 16 trail entries
        for (s32 slot = 0; slot < 16; slot++)
        {
            sTrailEntry& entryA = pThis->m1C0_trailA[side][slot];
            sTrailEntry& entryB = pThis->m640_trailB[side][slot];

            // Auto-scroll positions
            entryA.m18_position += pEngine->m1A0_battleAutoScrollDelta;
            entryB.m18_position += pEngine->m1A0_battleAutoScrollDelta;

            // Integrate: velocity2 += velocity
            entryA.mC_velocity2 += entryA.m0_velocity;
            entryB.mC_velocity2 += entryB.m0_velocity;

            // Dampen velocity2
            entryA.mC_velocity2 -= MTH_Mul(entryA.mC_velocity2, fixedPoint(0x41));
            entryB.mC_velocity2 -= MTH_Mul(entryB.mC_velocity2, fixedPoint(0x41));

            // Apply velocity2 to position
            entryA.m18_position += entryA.mC_velocity2;
            entryB.m18_position += entryB.mC_velocity2;

            // Clear velocity
            entryA.m0_velocity = {};
            entryB.m0_velocity = {};
        }
    }
}

// 06085dd0
static void sDragonPhoenixTask_Update(sDragonPhoenixTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    // Verify overlay camera data exists — if not, finish immediately
    sSaturnPtr camData = getOverlayCameraData(pEngine);
    if (camData.m_offset == 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // Read camera offset for quadrant+1
    s8 quadrant = (s8)performModulo(4, (u8)pEngine->m22C_dragonCurrentQuadrant + 1);
    sVec3_FP camOffset = readOverlayCameraOffset(pEngine, quadrant);

    // Verify camera data is non-zero (distance squared check)
    fixedPoint distSq = MTH_Mul(camOffset.m0_X, camOffset.m0_X) +
                        MTH_Mul(camOffset.m4_Y, camOffset.m4_Y) +
                        MTH_Mul(camOffset.m8_Z, camOffset.m8_Z);
    if ((s32)distSq == 0)
    {
        pThis->getTask()->markFinished();
        return;
    }

    // Global timer + rotation
    pThis->mB4_globalTimer++;
    pThis->mB14_rotation += 0x111111;

    // State change detection
    if (pThis->mAE_prevState != pThis->mAC_state)
    {
        pThis->mAF_subState = 0;
        pThis->mB2_subTimer = 0;
    }
    pThis->mAE_prevState = pThis->mAC_state;

    // Auto-scroll camera source + target positions
    if (pThis->mC0_hasInitModel != 0)
    {
        pThis->m1B4_cameraTarget += pEngine->m1A0_battleAutoScrollDelta;
        pThis->m148_currentPos += pEngine->m1A0_battleAutoScrollDelta;
    }
    if (pThis->mD8_hasCamera != 0)
    {
        pThis->m19C_cameraPosition += pEngine->m1A0_battleAutoScrollDelta;
        if (pThis->mE4_isReturning == 0)
        {
            battleEngine_resetCameraInterpolation();
        }
    }

    // Particle effects update
    if (pThis->mD0_hasParticles != 0)
    {
        dragonPhoenix_updateTrailParticles(pThis);

        // Particle lifetime/position update
        for (int i = 0; i < 8; i++)
        {
            sPhoenixParticle& particle = pThis->mB1C_particles[i];
            particle.m14_lifetime--;

            if (particle.m14_lifetime == 0)
            {
                // Respawn: pick random vertex position from trail
                s32 side = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(2));
                s16 sideOffset = (s16)side * 0x240;
                s32 slot = (s32)MTH_Mul(fixedPoint(randomNumber() >> 16), fixedPoint(0x10));
                // Sample position from trail entry
                particle.m8_position = pThis->m1C0_trailA[side][slot & 0xF].m18_position;
            }

            if (particle.m14_lifetime < 0)
            {
                // Active particle: auto-scroll + animate
                particle.m8_position += pEngine->m1A0_battleAutoScrollDelta;
                sGunShotTask_UpdateSub4(&particle.m0_quad);
            }

            if (particle.m14_lifetime < -0xC)
            {
                particle.m14_lifetime = 1; // will respawn next frame
            }

            // Additional auto-scroll for all particles
            particle.m8_position += pEngine->m1A0_battleAutoScrollDelta;
        }
    }

    // Particle visual index cycling (every 8 frames)
    if ((pThis->mB4_globalTimer & 7) == 0)
    {
        pThis->mB0_particleVisualIndex++;
        if ((u8)pThis->mB0_particleVisualIndex > 0xF)
        {
            pThis->mB0_particleVisualIndex = 0;
        }
    }

    // Rotation angle increment
    pThis->mB18_rotationAngle += 0x71C71C;
    pThis->mB18_rotationAngle &= 0xFFFFFFF;

    // Main state dispatch
    switch (pThis->mAC_state)
    {
    case 0:
        dragonPhoenix_stateApproach(pThis);
        break;
    case 1:
        dragonPhoenix_stateAttack(pThis);
        break;
    case 2:
        dragonPhoenix_stateRetreat(pThis);
        break;
    default:
        break;
    }

    // Post-state camera interpolation
    if (pThis->mC0_hasInitModel != 0)
    {
        // Scale ramp
        if (pThis->m108_scale < 0x18000)
        {
            pThis->m108_scale += 0xCCC;
        }

        // Camera spring interpolation
        if (pThis->mDC_hasCameraInterp != 0)
        {
            pThis->m130_cameraVelocity += MTH_Mul(pThis->m1B4_cameraTarget - pThis->m148_currentPos, fixedPoint(pThis->m124_cameraDamping));
        }

        // Camera rotation interpolation (28-bit angles)
        if (pThis->mE0_hasCameraTarget != 0)
        {
            sVec3_FP rotDelta;
            rotDelta.m0_X = signExtend28((s32)pThis->mB08_lookAngle.m0_X - (s32)pThis->m190_currentRotation.m0_X);
            rotDelta.m4_Y = signExtend28((s32)pThis->mB08_lookAngle.m4_Y - (s32)pThis->m190_currentRotation.m4_Y);
            rotDelta.m8_Z = signExtend28((s32)pThis->mB08_lookAngle.m8_Z - (s32)pThis->m190_currentRotation.m8_Z);
            pThis->m178_rotVelocity += MTH_Mul(rotDelta, fixedPoint(pThis->m128_cameraRotDamping));
        }

        // Integrate camera acceleration -> velocity -> position
        pThis->m13C_cameraAccel += pThis->m130_cameraVelocity;
        pThis->m13C_cameraAccel -= MTH_Mul(pThis->m13C_cameraAccel, fixedPoint(pThis->m11C_posDamping));
        pThis->m148_currentPos += pThis->m13C_cameraAccel;
        pThis->m130_cameraVelocity = {};

        // Integrate rotation
        pThis->m184_rotAccel += pThis->m178_rotVelocity;
        pThis->m184_rotAccel -= MTH_Mul(pThis->m184_rotAccel, fixedPoint(pThis->m120_rotDamping));
        pThis->m190_currentRotation += pThis->m184_rotAccel;
        // Mask to 28-bit
        pThis->m190_currentRotation.m0_X = (s32)pThis->m190_currentRotation.m0_X & 0xFFFFFFF;
        pThis->m190_currentRotation.m4_Y = (s32)pThis->m190_currentRotation.m4_Y & 0xFFFFFFF;
        pThis->m190_currentRotation.m8_Z = (s32)pThis->m190_currentRotation.m8_Z & 0xFFFFFFF;
        pThis->m178_rotVelocity = {};

        // Step animation if active
        if (pThis->mD4_lightingRestored != 0)
        {
            stepAnimation(&pThis->m0C_model1);
        }
    }

    // Debug display
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1e] != 0)
    {
        vdp2DebugPrintSetPosition(0xd, 0xc);
        s32 val = (s32)MTH_Mul(fixedPoint(pThis->m10C_secondaryScale), fixedPoint(0x2a000));
        vdp2PrintfSmallFont("%5d", val >> 0xc);
    }
}

// 06085c3c
void berserk_createDragonPhoenix(s_battleEngine* pThis)
{
    static const sDragonPhoenixTask::TypedTaskDefinition definition = {
        nullptr,
        &sDragonPhoenixTask_Update,
        nullptr,
        &sDragonPhoenixTask_Delete,
    };

    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    sDragonPhoenixTask* pNewTask = createSubTaskWithCopy<sDragonPhoenixTask>(pThis, &definition);
    if (pNewTask == nullptr || pTargetSystem->m20A_numSelectableEnemies == 0)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Zero flags block (0xC0..0xF7) — 14 s32 fields
    pNewTask->mC0_hasInitModel = 0;
    pNewTask->mC4_hasNoTargets = 0;
    pNewTask->mC8_doOrbitCamera = 0;
    pNewTask->mCC_hasLightEffect = 0;
    pNewTask->mD0_hasParticles = 0;
    pNewTask->mD4_lightingRestored = 0;
    pNewTask->mD8_hasCamera = 0;
    pNewTask->mDC_hasCameraInterp = 0;
    pNewTask->mE0_hasCameraTarget = 0;
    pNewTask->mE4_isReturning = 0;
    pNewTask->mE8_hasCameraDesired = 0;
    pNewTask->mEC_hasSecondaryAnim = 0;
    pNewTask->mF0_hasLight = 0;
    pNewTask->mF4_hasPaletteFade = 0;

    // Zero block 2 (0xF8..0x103): 3 sVec3_FP fields
    pNewTask->mF8_cameraMode = 0;
    pNewTask->mFC_cameraHeightMode = 0;
    pNewTask->m100_cameraDistMode = 0;

    // Init scale and speed
    pNewTask->m108_scale = 0x10000;
    pNewTask->m10C_secondaryScale = 0x10000;
    pNewTask->m114_interpTarget = 0x1CCCC;

    // Init state
    pNewTask->mAC_state = 0;
    pNewTask->mAE_prevState = 0;
    pNewTask->mAF_subState = 0;
    pNewTask->mB2_subTimer = 0;
    pNewTask->mBA_attackCount = 0;
    pNewTask->mBC_targetList = nullptr;
    pNewTask->m104_animLoopFlag = 0;
    pNewTask->mB8_targetIndex = 0;
    pNewTask->mBDC_secondaryTask = 0;
    pNewTask->m110_interpProgress = 0;

    // Random initial rotation
    u32 rnd = randomNumber();
    pNewTask->mB14_rotation = (s32)MTH_Mul(fixedPoint(rnd >> 16), fixedPoint(0x10000000));
    pNewTask->mB18_rotationAngle = 0;

    // Set base damage from number of selectable enemies
    s16 numEnemies = pTargetSystem->m20A_numSelectableEnemies;
    if (numEnemies == 1)
    {
        pNewTask->m12C_damageBase = 2000;
    }
    else if (numEnemies == 2)
    {
        pNewTask->m12C_damageBase = 1000;
    }
    else
    {
        pNewTask->m12C_damageBase = 500;
    }

    // Set camera mode flags based on battle type
    s8 battleType = getBattleTypeId();
    if (battleType == 5)
    {
        pNewTask->mFC_cameraHeightMode = 1;
    }
    else if (battleType == 6)
    {
        pNewTask->mF8_cameraMode = 1;
    }
    else if (battleType == 9 || battleType == 10 || battleType == 11 || battleType == 12 || battleType == 13)
    {
        pNewTask->m100_cameraDistMode = 1;
    }
}
