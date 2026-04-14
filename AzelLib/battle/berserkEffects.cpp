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
#include "menu_dragonMorph.h"
#include "kernel/fileBundle.h"

//=============================================================================
// Shared helpers used by all berserk effect files
//=============================================================================

// BTL_A3::060a2fc8 — inner camera orbit computation
// param_1 = output position, param_2 = 28-bit angle, param_3 = per-quadrant altitude array
void berserkCameraOrbitInner(sVec3_FP* pOut, u32 angle, s32* pPerQuadrantAltitude)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    angle = angle & 0xFFFFFFF;
    u8 cosSpeedIdx;
    u8 sinSpeedIdx;
    s32 quadrantIdx;
    if (angle < 0x4000000)
    {
        cosSpeedIdx = 0; sinSpeedIdx = 1; quadrantIdx = 0;
    }
    else if (angle < 0x8000000)
    {
        cosSpeedIdx = 2; sinSpeedIdx = 1; quadrantIdx = 1;
    }
    else if (angle < 0xC000000)
    {
        cosSpeedIdx = 2; sinSpeedIdx = 3; quadrantIdx = 2;
    }
    else
    {
        cosSpeedIdx = 0; sinSpeedIdx = 3; quadrantIdx = 3;
    }

    s16 angleIdx = (s16)(angle >> 16);

    // X = sin(angle) * speed[sinSpeedIdx] + battleCenter.X
    s32 sinResult = MTH_Mul(getSin(angleIdx), pEngine->m45C_perQuadrantDragonSpeed[sinSpeedIdx]);
    (*pOut)[0] = fixedPoint(sinResult + (s32)pEngine->mC_battleCenter[0]);

    // Z = cos(angle) * speed[cosSpeedIdx] + battleCenter.Z
    s32 cosResult = MTH_Mul(getCos(angleIdx), pEngine->m45C_perQuadrantDragonSpeed[cosSpeedIdx]);
    (*pOut)[2] = fixedPoint(cosResult + (s32)pEngine->mC_battleCenter[2]);

    // Y = perQuadrantAltitude[quadrantIdx] * adjustedAngle + dragonAltitude + cameraMaxAltitude
    u32 adjustedAngle = angle;
    switch (quadrantIdx)
    {
    case 1: adjustedAngle = angle + 0xFC000000; break;
    case 2: adjustedAngle = angle + 0xF8000000; break;
    case 3: adjustedAngle = angle + 0xF4000000; break;
    default: break;
    }

    s32 yResult = MTH_Mul(pPerQuadrantAltitude[quadrantIdx], fixedPoint(adjustedAngle));
    (*pOut)[1] = fixedPoint(yResult + pEngine->m364_perQuadrantDragonAltitude[quadrantIdx]
        + pTargetSystem->m204_cameraMaxAltitude);
}

// BTL_A3::060a3120 — quadrant-based camera orbit wrapper
void berserkCameraOrbit(sVec3_FP* pOut, s32 rotAngle, s32* pPerQuadrantAltitude)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 quadrant = pEngine->m22C_dragonCurrentQuadrant;
    s32 baseAngle;
    switch (quadrant)
    {
    case 0: baseAngle = 0; break;
    case 1: baseAngle = 0x4000000; break;
    case 2: baseAngle = 0x8000000; break;
    case 3: baseAngle = 0xC000000; break;
    default: baseAngle = rotAngle; break;
    }
    berserkCameraOrbitInner(pOut, baseAngle + rotAngle, pPerQuadrantAltitude);
}

// Helper: read camera data from overlay battle data table
// Pattern: overlayBattledata[subBattleId * 0x20 + 0x1C] gives base pointer,
// then index by (quadrantOrIndex * 0xC) + componentOffset within a subBattleId * 0x60 block
sSaturnPtr getOverlayCameraData(s_battleEngine* pEngine)
{
    s8 subBattleId = pEngine->m3B0_subBattleId;
    return readSaturnEA(pEngine->m3A8_overlayBattledata + subBattleId * 0x20 + 0x1C);
}

sVec3_FP readOverlayCameraOffset(s_battleEngine* pEngine, s8 quadrant)
{
    sSaturnPtr base = getOverlayCameraData(pEngine);
    if (base.m_offset == 0) return sVec3_FP();
    s8 subBattleId = pEngine->m3B0_subBattleId;
    s32 offset = subBattleId * 0x60 + quadrant * 0xC;
    return sVec3_FP(readSaturnS32(base + offset), readSaturnS32(base + offset + 4), readSaturnS32(base + offset + 8));
}

// Offset 0x30 in the sub-battle camera block (used for onslaught camera lerp target)
sVec3_FP readOverlayCameraOffset2(s_battleEngine* pEngine)
{
    sSaturnPtr base = getOverlayCameraData(pEngine);
    if (base.m_offset == 0) return sVec3_FP();
    s8 subBattleId = pEngine->m3B0_subBattleId;
    s32 offset = subBattleId * 0x60 + 0x30;
    return sVec3_FP(readSaturnS32(base + offset), readSaturnS32(base + offset + 4), readSaturnS32(base + offset + 8));
}

//=============================================================================
// Shared: hit flash fade task (BTL_A3::060a31a2 / 060a3200)
//=============================================================================

struct sBerserkFadeTask : public s_workAreaTemplate<sBerserkFadeTask>
{
    u16 m0_color1;
    u16 m2_color2;
    u16 m4_duration1;
    u16 m6_color3;
    u16 m8_color4;
    u16 mA_duration2;
    s8 mC_state;
    u8 mD_pad;
    s16 mE_clofsl;
    // size 0x10
};

// BTL_A3::060a3200
static void sBerserkFadeTask_Update(sBerserkFadeTask* pThis)
{
    if (pThis->mC_state == 0)
    {
        g_fadeControls.m_4D = 0;
        if ((s8)g_fadeControls.m_4C < 1)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = pThis->mE_clofsl;
            vdp2Controls.m20_registers[1].m112_CLOFSL = vdp2Controls.m20_registers[0].m112_CLOFSL;
        }
        fadePalette(&g_fadeControls.m0_fade0, pThis->m0_color1, pThis->m2_color2, pThis->m4_duration1);
        g_fadeControls.m_4D = 5;
        pThis->mC_state++;
    }
    else if (pThis->mC_state == 1 && g_fadeControls.m0_fade0.m20_stopped != 0)
    {
        g_fadeControls.m_4D = 0;
        if ((s8)g_fadeControls.m_4C < 1)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = pThis->mE_clofsl;
            vdp2Controls.m20_registers[1].m112_CLOFSL = vdp2Controls.m20_registers[0].m112_CLOFSL;
        }
        fadePalette(&g_fadeControls.m0_fade0, pThis->m6_color3, pThis->m8_color4, pThis->mA_duration2);
        g_fadeControls.m_4D = 5;
        pThis->getTask()->markFinished();
    }
}

// BTL_A3::060a31a2
void berserk_createHitFlashFade(p_workArea pParent, s16 clofsl, u16 color1, u16 color2, u16 duration1, u16 color3, u16 color4, u16 duration2)
{
    static const sBerserkFadeTask::TypedTaskDefinition definition = {
        nullptr,
        &sBerserkFadeTask_Update,
        nullptr,
        nullptr,
    };

    sBerserkFadeTask* pTask = createSiblingTask<sBerserkFadeTask>(pParent, &definition);
    if (pTask == nullptr) return;
    pTask->m0_color1 = color1;
    pTask->m2_color2 = color2;
    pTask->m4_duration1 = duration1;
    pTask->m6_color3 = color3;
    pTask->m8_color4 = color4;
    pTask->mA_duration2 = duration2;
    pTask->mE_clofsl = clofsl;
    pTask->mC_state = 0;
}

// Helper: sign-extend 28-bit angle to 32-bit
s32 signExtend28(s32 v)
{
    if ((v & 0x8000000) == 0)
        return v & 0xFFFFFFF;
    else
        return v | (s32)0xF0000000;
}

// Helper: extract integer byte from fixedPoint for RGB packing
u8 fpToColorByte(fixedPoint v)
{
    return (u8)(((u32)(s32)v >> 16) & 0xFF);
}

// 060073f8 — scale base damage by dragon SPR stat
// formula: baseDamage * (dragonSpr/200 * 0.4 + 0.8)
s16 computeSprScaledDamage(s16 baseDamage)
{
    fixedPoint sprRatio = FP_Div((s32)(s16)mainGameState.gameStats.mC2_dragonSpr, fixedPoint(200));
    fixedPoint multiplier = MTH_Mul(fixedPoint(0x6666), sprRatio); // 0.4 * ratio
    fixedPoint result = MTH_Mul(fixedPoint((s32)baseDamage << 16), multiplier + fixedPoint(0xCCCC)); // * (0.4*ratio + 0.8)
    return (s16)(((s32)result + 0x8000) >> 16);
}

// BTL_A3::0608d7f0 — compute berserk base damage scaled by SPR
s16 phantomSlasher_getBaseDamage(s16 baseDamage)
{
    return computeSprScaledDamage(baseDamage);
}

// BTL_A3::060a2e78
s32 battleTypeId_match(u16 battleId, u8 category)
{
    if (gBattleManager->m4 == (s16)category && (s32)(s16)gBattleManager->m8 == (u32)battleId)
    {
        return 1;
    }
    return 0;
}

// BTL_A3::060a2eb6 — get current battle type ID
// Table at 060b1884: 14 entries of {u8 typeId, pad, u16 battleId(BE), u8 category, pad}
static const u8 battleTypeTable[] = {
    0x00, 0x00, 0x00, 0x0A, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x10, 0x01, 0x00,
    0x02, 0x00, 0x00, 0x11, 0x01, 0x00,
    0x03, 0x00, 0x00, 0x06, 0x02, 0x00,
    0x04, 0x00, 0x00, 0x16, 0x05, 0x00,
    0x05, 0x00, 0x00, 0x04, 0x08, 0x00,
    0x06, 0x00, 0x00, 0x05, 0x08, 0x00,
    0x07, 0x00, 0x00, 0x05, 0x09, 0x00,
    0x08, 0x00, 0x00, 0x0F, 0x0B, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x0D, 0x00,
    0x0A, 0x00, 0x00, 0x01, 0x0D, 0x00,
    0x0B, 0x00, 0x00, 0x02, 0x0D, 0x00,
    0x0C, 0x00, 0x00, 0x03, 0x0D, 0x00,
    0x0D, 0x00, 0x00, 0x04, 0x0D, 0x00,
};

s8 getBattleTypeId()
{
    const u8* p = battleTypeTable;
    for (s32 i = 0; i < 14; i++)
    {
        u16 battleId = READ_BE_U16(p + 2);
        u8 category = p[4];
        if (battleTypeId_match(battleId, category) == 1)
        {
            return (s8)p[0];
        }
        p += 6;
    }
    return 0xE;
}

// BTL_A3::060a2efc — random spherical offset for bolt scatter
sVec3_FP randomSphericalOffset(s32 maxRadius)
{
    u32 rnd1 = randomNumber();
    s32 r = MTH_Mul(rnd1 >> 16, fixedPoint(maxRadius));
    u32 rnd2 = randomNumber();
    s32 angle1 = MTH_Mul(rnd2 >> 16, fixedPoint(0x10000000));
    s32 sinR = MTH_Mul(getSin(((u32)angle1 >> 16) & 0xFFF), r);

    u32 rnd3 = randomNumber();
    s32 r2 = MTH_Mul(rnd3 >> 16, fixedPoint(maxRadius));
    u32 rnd4 = randomNumber();
    s32 angle2 = MTH_Mul(rnd4 >> 16, fixedPoint(0x10000000));
    s32 cosR = MTH_Mul(getCos(((u32)angle2 >> 16) & 0xFFF), r2);

    u32 rnd5 = randomNumber();
    s32 angle3 = MTH_Mul(rnd5 >> 16, fixedPoint(0x10000000));
    s32 sinCos = MTH_Mul(getSin(((u32)angle3 >> 16) & 0xFFF), cosR);
    u32 rnd6 = randomNumber();
    s32 angle4 = MTH_Mul(rnd6 >> 16, fixedPoint(0x10000000));
    s32 cosCos = MTH_Mul(getCos(((u32)angle4 >> 16) & 0xFFF), cosR);

    return sVec3_FP(fixedPoint(cosCos), fixedPoint(sinR), fixedPoint(sinCos));
}

// BTL_A3::060a2efc — init per-quadrant camera altitude deltas
// Each entry = (altitude[nextQuadrant] - altitude[thisQuadrant]) / 0x4000000
void initPerQuadrantCameraAltitude(s32* pOut)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    for (u32 i = 0; i < 4; i++)
    {
        s32 current = pEngine->m364_perQuadrantDragonAltitude[i];
        s32 next = pEngine->m364_perQuadrantDragonAltitude[(i + 1) & 3];
        pOut[i] = FP_Div(next - current, fixedPoint(0x4000000));
    }
}

// BTL_A3::06096048 — particle effect for certain berserks (0x95, 0x96, 0xA1)
void FUN_06096048(s_battleEngine* pThis)
{
    Unimplemented();
}

//=============================================================================
// Shared VDP2 background + palette fade helpers
// Each berserk uses a specific palette color for its visual tint.
// These are separate functions in the Saturn binary — keep boundaries.
//=============================================================================

// BTL_A3::0607f720
static void plasmaVortex_initVisual()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xA988, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::0607f76c
static void plasmaVortex_restoreVisual()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xA988, 0xC210, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::06095660
static void plasmaSwarm_restoreVisual()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xB187, 0xC210, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::0608c964
static void energyPrism_initLightEffect()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xA8E7, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::0608c9b0
static void energyPrism_restoreLighting()
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    g_fadeControls.m_4D = 6;
    if ((s8)g_fadeControls.m_4C < 7)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0x4A;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0x4A;
    }
    fadePalette(&g_fadeControls.m0_fade0, 0xA8E7, 0xC210, 10);
    g_fadeControls.m_4D = 5;
}

// BTL_A3::0607df10 — Plasma Vortex
// Task (0x204 createSubTask) with 16 particle entries, dual-mode,
// target tracking, computeLookAt orientation, camera orbit, homing toward targets.
//
// FIXME audit findings (63 issues across 5 berserks in this file):
//
// --- Plasma Vortex ---
// 1. [CRITICAL] Missing attackType param — clobbering m1FC flags
// 2. [CRITICAL] Missing particle trail system
//
// --- Plasma Swarm (line ~701) ---
// 3. [CRITICAL] Missing all 4 init sub-calls
// 4. [CRITICAL] Missing core particle tracking
//
// --- Hunting Scythe (line ~878) ---
// 5. [CRITICAL] States 1-2 almost entirely stubbed
//
// --- Energy Prism (line ~1052) ---
// 6. [CRITICAL] Damage formula completely wrong (constant 0x50 vs curve-type table)
//
// --- Vengeance Orbs (line ~1375) ---
// 7. [CRITICAL] Init severely incomplete
// 8. [CRITICAL] State 5 fabricated timer

struct sPlasmaVortexTask : public s_workAreaTemplate<sPlasmaVortexTask>
{
    sVec3_FP m0_position;              // 0x00 — current position
    u8 mC_particles[0xB4];             // 0x0C..0xBF: 16 particle entries (various data)
    u8 mC0_particleState[0x40];        // 0xC0..0xFF: particle state arrays
    u8 m100_particleData2[0x40];       // 0x100..0x13F
    u8 m140_particleData3[0x40];       // 0x140..0x17F
    u8 m180_pad[0x48];                 // 0x180..0x1C7
    s32 m1C8_targetIndex;              // 0x1C8
    sVec3_FP m1CC_sourcePosition;      // 0x1CC — source position (minus autoScroll)
    sVec3_FP m1D8_targetPosition;      // 0x1D8 — target position
    sVec3_FP* m1E4_pSourcePos;         // 0x1E4 — pointer to source (dragon or battleCenter)
    sVec3_FP* m1E8_pTargetPos;         // 0x1E8 — pointer to target
    s32 m1EC_trackSpeed;               // 0x1EC — ramps up, damage when >= 0x10000
    s32 m1F0_yGravity;                 // 0x1F0 — Y velocity component
    s8 m1F4_state;                     // 0x1F4
    u8 m1F5_pad;
    u16 m1F6_hitCount;                 // 0x1F6
    u16 m1F8_timer;                    // 0x1F8
    u16 m1FA_pad;
    u32 m1FC_flags;                    // 0x1FC — bit 0x8: camera active, 0x10: single target, 0x20: has camera setup
    s32 m200_attackType;               // 0x200 — 0=dragon→enemy, 1=enemy→dragon
    // size 0x204
};

// BTL_A3::0607f090
static void sPlasmaVortexTask_Delete(sPlasmaVortexTask* pThis)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::060acc1c — per-attack-type base damage for Plasma Vortex
static const s16 plasmaVortexDamageTable[] = { 0x50, 0x3C }; // [0]=dragon→enemy, [1]=enemy→dragon

// BTL_A3::0607e5f4
static void sPlasmaVortexTask_Update(sPlasmaVortexTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    // Auto-scroll source position
    pThis->m1CC_sourcePosition += pEngine->m1A0_battleAutoScrollDelta;

    // Track target position
    if ((pThis->m1FC_flags & 0x10) != 0)
    {
        // Single target mode — read from pointer
        pThis->m1D8_targetPosition = *pThis->m1E8_pTargetPos;
    }

    // Camera auto-scroll
    if ((pThis->m1FC_flags & 0x20) != 0)
    {
        // TODO: camera position auto-scroll and interpolation
        if ((pThis->m1FC_flags & 8) != 0)
        {
            battleEngine_resetCameraInterpolation();
        }
    }

    switch (pThis->m1F4_state)
    {
    case 0:
    {
        // State 0: init — wait for file loading, setup lighting, init draw
        pThis->m0_position = pThis->m1CC_sourcePosition;
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
            break;

        pEngine->m188_flags.m20000 = 1;
        syncM68KSoundCPU();

        sVec3_FP lightColor;
        battleResultScreen_updateSub0(0x1F, 0x1F, 0x1F, &lightColor);
        battleGrid_setupLightInterpolation(10, pGrid->m1CC_lightColor, lightColor);

        plasmaVortex_initVisual();

        pThis->m1F4_state++;
        pThis->m1F8_timer = 0;
        break;
    }
    case 1:
    {
        // State 1: main loop — trail shift, homing, damage

        // TODO: 16-particle trail shifting (complex stride-based array operations)

        // Rotation and tracking ramp
        // param_1[0x7b] ramps up tracking speed
        pThis->m1EC_trackSpeed += 0xB85;
        pThis->m1CC_sourcePosition[1] += fixedPoint(pThis->m1F0_yGravity);

        // Homing interpolation toward target
        sVec3_FP toTarget;
        toTarget[0] = MTH_Mul(pThis->m1D8_targetPosition[0] - pThis->m1CC_sourcePosition[0],
                              fixedPoint(pThis->m1EC_trackSpeed));
        toTarget[1] = MTH_Mul(pThis->m1D8_targetPosition[1] - pThis->m1CC_sourcePosition[1],
                              fixedPoint(pThis->m1EC_trackSpeed));
        toTarget[2] = MTH_Mul(pThis->m1D8_targetPosition[2] - pThis->m1CC_sourcePosition[2],
                              fixedPoint(pThis->m1EC_trackSpeed));

        pThis->m0_position[0] = pThis->m1CC_sourcePosition[0] + toTarget[0];
        pThis->m0_position[1] = pThis->m1CC_sourcePosition[1] + toTarget[1];
        pThis->m0_position[2] = pThis->m1CC_sourcePosition[2] + toTarget[2];

        pThis->m1F8_timer++;

        // Check for convergence — damage when tracking speed >= 0x10000
        if (pThis->m1EC_trackSpeed >= 0x10000 && pThis->m1F6_hitCount <= 0xE)
        {
            pThis->m1F6_hitCount++;

            if ((pThis->m1FC_flags & 0x10) == 0)
            {
                // Multi-target mode
                pThis->m1FC_flags |= 8;
                pThis->m1D8_targetPosition += pEngine->m1A0_battleAutoScrollDelta;

                if (pThis->m200_attackType == 1)
                {
                    // Enemy attacking dragon
                    if (pThis->m1F6_hitCount == 1)
                    {
                        berserk_createHitFlashFade(pThis, 0x4A, 0xA988, 0xFBD4, 2, 0xFBD4, 0xA988, 0xF);
                    }
                    applyDamageToDragon(pDragon->m8C,
                        plasmaVortexDamageTable[pThis->m200_attackType],
                        pDragon->m8_position, 2,
                        toTarget, 0x1000);
                }
                else
                {
                    // Dragon attacking enemy
                    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->m1C8_targetIndex];
                    if (enemy->m4_targetable != nullptr &&
                        (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                        enemy->m0_isActive > 0)
                    {
                        if (pThis->m1F6_hitCount == 1)
                        {
                            berserk_createHitFlashFade(pThis, 0x4A, 0xA988, 0xFBD4, 2, 0xFBD4, 0xA988, 0xF);
                        }

                        sVec3_FP dir;
                        sVec3_FP* enemyPos = getBattleTargetablePosition(*enemy->m4_targetable);
                        dir[0] = (*enemyPos)[0] - (*pThis->m1E4_pSourcePos)[0];
                        dir[1] = (*enemyPos)[1] - (*pThis->m1E4_pSourcePos)[1];
                        dir[2] = (*enemyPos)[2] - (*pThis->m1E4_pSourcePos)[2];

                        s16 dmg = phantomSlasher_getBaseDamage(plasmaVortexDamageTable[0]);
                        dmg = computeResistanceDamage(enemy->m4_targetable, dmg, 2);
                        applyDamageToEnnemy(enemy->m4_targetable, dmg, enemyPos, 3,
                            dir, fixedPoint(0x2000));
                    }
                }
            }
        }

        // Check for completion — restore lighting when tracking speed > 0x34000
        if (pThis->m1EC_trackSpeed > 0x34000)
        {
            battleGrid_setupLightInterpolation(0x1E, pGrid->m1CC_lightColor,
                pGrid->m1D8_newLightColor);
            resetProjectVector();
            plasmaVortex_restoreVisual();

            // Clear draw method
            if ((pThis->m1FC_flags & 0x20) != 0)
            {
                pThis->m1FC_flags &= ~0x20;
                battleEngine_restoreCameraAfterEnemyAttack();
            }
            pThis->m1F4_state++;
            pThis->m1F8_timer = 0;
        }
        break;
    }
    case 2:
    {
        // State 2: wait then finish
        u16 t = pThis->m1F8_timer;
        pThis->m1F8_timer = t + 1;
        if (t > 0x2D)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

    // Target tracking (runs every frame)
    if ((pThis->m1FC_flags & 0x10) == 0)
    {
        if (pThis->m200_attackType == 0 && (pThis->m1FC_flags & 8) == 0)
        {
            // Track live enemy position
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->m1C8_targetIndex];
            if (enemy->m4_targetable != nullptr)
            {
                sVec3_FP* pos = getBattleTargetablePosition(*enemy->m4_targetable);
                pThis->m1D8_targetPosition = *pos;
            }
        }
    }
}

void berserk_createPlasmaVortex(s_battleEngine* pThis)
{
    static const sPlasmaVortexTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sPlasmaVortexTask*))&sPlasmaVortexTask_Update,
        nullptr,
        (void(*)(sPlasmaVortexTask*))&sPlasmaVortexTask_Delete,
    };

    sPlasmaVortexTask* pNewTask = createSubTask<sPlasmaVortexTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    // Attack type 0: dragon attacks enemies
    pNewTask->m200_attackType = 0;
    pNewTask->m1E4_pSourcePos = &pDragon->m8_position;
    pNewTask->m1E8_pTargetPos = &pThis->mC_battleCenter;

    pNewTask->m1EC_trackSpeed = 0;

    // Init source position from dragon position minus auto-scroll
    pNewTask->m1CC_sourcePosition = pDragon->m8_position;
    pNewTask->m1CC_sourcePosition -= pThis->m1A0_battleAutoScrollDelta;
    pNewTask->m1CC_sourcePosition[1] += fixedPoint(0x1E000); // Y offset

    // Quadrant-based offset
    sVec3_FP offset = {};
    switch (pThis->m22C_dragonCurrentQuadrant)
    {
    case 0: offset[2] = fixedPoint(0x1E000); break;
    case 1: offset[0] = fixedPoint(0x1E000); break;
    case 2: offset[2] = fixedPoint(-0x1E000); break;
    case 3: offset[0] = fixedPoint(-0x1E000); break;
    }

    pNewTask->m1CC_sourcePosition += offset;

    // Init current position from source
    pNewTask->m0_position = pNewTask->m1CC_sourcePosition;

    // Find first valid target
    pNewTask->m1C8_targetIndex = 0;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    if (pTargetSystem->m20A_numSelectableEnemies > 0)
    {
        for (s32 i = 0; i < pTargetSystem->m20A_numSelectableEnemies; i++)
        {
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[i];
            if (enemy->m4_targetable != nullptr &&
                (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                enemy->m0_isActive > 0)
            {
                sVec3_FP* pos = getBattleTargetablePosition(*enemy->m4_targetable);
                pNewTask->m1D8_targetPosition = *pos;
                pNewTask->m1C8_targetIndex = i;
                break;
            }
        }
    }
    else
    {
        pNewTask->m1FC_flags |= 0x10; // single-target mode
    }

    // Camera setup from overlay data
    sSaturnPtr camData = getOverlayCameraData(pThis);
    if (camData.m_offset == 0)
    {
        // No camera data — debug message
    }
    else
    {
        // TODO: full camera setup with berserkCameraOrbit, overlay data, quadrant selection
        // For now mark camera as not set up
    }

    // Init 16 particle positions to source position
    // TODO: init particle arrays at stride 0x0C within mC_particles
    // Each particle starts at m1CC_sourcePosition

    pNewTask->m1FC_flags = 0;
    pNewTask->m1F0_yGravity = {};
}

// BTL_A3::06094f68 — Plasma Swarm

struct sPlasmaSwarmTask : public s_workAreaTemplate<sPlasmaSwarmTask>
{
    u8 m0_pad[0xBC];               // 0x00..0xBB: particle data, camera, targets
    s32 mBC_cameraFlags[2];        // 0xBC..0xC3: camera/visual flags
    s32 mC4_cameraActive;          // 0xC4
    u8 mC8_pad[4];
    s32 mCC_visualFlags;           // 0xCC
    s16 mD0_timer;                 // 0xD0
    s16 mD2_globalTimer;           // 0xD2
    s32 mD4_rotationAngle;         // 0xD4
    s32 mD8_rotationSpeed;         // 0xD8
    sVec3_FP mDC_lookAtRotation;   // 0xDC
    s8 mE8_state;                  // 0xE8
    u8 mE9_pad[3];
    // size 0xEC
};

// BTL_A3::0609524a
static void sPlasmaSwarmTask_Delete(sPlasmaSwarmTask* pThis)
{
    if (pThis->mBC_cameraFlags[0] != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
        battleGrid_setCameraFov(0x238E38E);
    }
    if (pThis->mCC_visualFlags != 0)
    {
        plasmaSwarm_restoreVisual();
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::060950ac
static void sPlasmaSwarmTask_Update(sPlasmaSwarmTask* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    // Global timer
    pThis->mD2_globalTimer++;

    // Oscillate rotation angle (ping-pong)
    pThis->mD4_rotationAngle += pThis->mD8_rotationSpeed;
    if (((u32)pThis->mD4_rotationAngle & 0xFFFFFFF) < 0x1555555)
        pThis->mD8_rotationSpeed = 0xB60B6;
    if (((u32)pThis->mD4_rotationAngle & 0xFFFFFFF) > 0xEAAAAAA)
        pThis->mD8_rotationSpeed = (s32)0xFFF49F4A;

    // Camera orbit — output to byte 0x40 (within m0_pad), altitude from byte 0x00
    // NOTE: these fields are within m0_pad blob — using raw cast since struct covers 0x00..0xBB
    sVec3_FP* pCameraPos = (sVec3_FP*)((u8*)pThis + 0x40);
    berserkCameraOrbit(pCameraPos, pThis->mD4_rotationAngle, (s32*)pThis);

    // Compute look-at rotation from dragon hotpoint to camera
    sVec3_FP dir;
    dir[0] = pDragon->mFC_hotpoints[2][0] - (*pCameraPos)[0];
    dir[1] = pDragon->mFC_hotpoints[2][1] - (*pCameraPos)[1];
    dir[2] = pDragon->mFC_hotpoints[2][2] - (*pCameraPos)[2];
    sVec2_FP lookAt;
    computeLookAt(dir, lookAt);
    pThis->mDC_lookAtRotation[0] = lookAt[0];
    pThis->mDC_lookAtRotation[1] = lookAt[1];
    pThis->mDC_lookAtRotation[2] = 0;

    // Activate camera tracking once particles start moving
    // NOTE: m2C is within m0_pad (byte 0x2C) — particle first-move flag
    if (pThis->mC4_cameraActive == 0 && *((s8*)pThis + 0x2C) != 0)
    {
        pThis->mC4_cameraActive = 1;
        battleEngine_setDesiredCameraPositionPointer((sVec3_FP*)((u8*)pThis + 0x80));
    }

    // FUN_060956aa — update swarm particle visual state
    // TODO: implement faithfully (updates particle positions/velocities)

    // FUN_06095872 — update per-target tracking
    // TODO: implement faithfully (updates target positions, checks hits)

    // State machine
    switch (pThis->mE8_state)
    {
    case 0:
    {
        // State 0: particles active — check for completion
        // FUN_060955c4 checks if all 8 particles have hit their targets
        // TODO: implement faithfully
        // For now, use timer as fallback
        if (pThis->mD2_globalTimer > 300)
        {
            pThis->mE8_state++;
            pThis->mD0_timer = 0x24;
        }
        break;
    }
    case 1:
    {
        // State 1: timer countdown then finish
        s16 t = pThis->mD0_timer;
        pThis->mD0_timer = t - 1;
        if (t < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }
}

void berserk_createPlasmaSwarm(s_battleEngine* pThis)
{
    static const sPlasmaSwarmTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sPlasmaSwarmTask*))&sPlasmaSwarmTask_Update,
        nullptr,
        (void(*)(sPlasmaSwarmTask*))&sPlasmaSwarmTask_Delete,
    };

    sPlasmaSwarmTask* pNewTask = createSubTask<sPlasmaSwarmTask>(pThis, &definition);
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    if (pNewTask == nullptr || pTargetSystem->m20A_numSelectableEnemies == 0)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    // Zero flags block (bytes 0xC0..0xCF: 2 iterations × 8 bytes)
    pNewTask->mBC_cameraFlags[0] = 0;
    pNewTask->mBC_cameraFlags[1] = 0;
    pNewTask->mC4_cameraActive = 0;
    pNewTask->mCC_visualFlags = 0;

    // FUN_060952b4 — init target positions from enemies
    // TODO: implement faithfully (allocates target array at 0xBC, fills with enemy positions)

    // FUN_06095548 — init camera from overlay data
    // TODO: implement faithfully (sets up camera orbit, overlay data check)

    // FUN_06095446 — init particle spawn positions
    // TODO: implement faithfully (initializes 8 particle positions around dragon)

    // FUN_060955f0 — init visual state (lighting, palette)
    // TODO: implement faithfully

    // Set m_shouldSetM100 flag (big-endian s32 = 1 at byte 0xCC)
    pNewTask->mCC_visualFlags = 1;

    // Compute initial look-at rotation from dragon hotpoint to camera
    sVec3_FP* pCameraPos = (sVec3_FP*)((u8*)pNewTask + 0x10);
    sVec3_FP lookDir;
    lookDir[0] = pDragon->mFC_hotpoints[2][0] - (*pCameraPos)[0];
    lookDir[1] = pDragon->mFC_hotpoints[2][1] - (*pCameraPos)[1];
    lookDir[2] = pDragon->mFC_hotpoints[2][2] - (*pCameraPos)[2];
    sVec2_FP lookAt;
    computeLookAt(lookDir, lookAt);
    pNewTask->mDC_lookAtRotation[0] = lookAt[0];
    pNewTask->mDC_lookAtRotation[1] = lookAt[1];
    pNewTask->mDC_lookAtRotation[2] = 0;

    // Init timers and state
    pNewTask->mD2_globalTimer = 0;
    pNewTask->mE8_state = 0;

    // Create 8 swarm particle sub-tasks
    for (s32 i = 0; i < 8; i++)
    {
        // FUN_060959bc — creates individual particle sibling task
        // TODO: implement faithfully (creates WithCopy sub-task with particle homing + damage)
    }
}

// BTL_A3::06083b08 — Hunting Scythe

struct sHuntingScytheTask : public s_workAreaTemplateWithCopy<sHuntingScytheTask>
{
    u8 m8_pad[0x794];              // 0x08..0x79B: scythe model + 16 trail entries at 0x6C stride
    s32 m79C_timer;                // 0x79C — countdown timer (starts 0x147)
    s32 m7A0_frameCounter;         // 0x7A0
    s8 m7A4_state;                 // 0x7A4
    u8 m7A5_subState;              // 0x7A5
    u16 m7A6_targetIndex;          // 0x7A6 — current enemy target index
    u8 m7A8_pad[2];
    u16 m7AA_globalTimer;          // 0x7AA
    u16 m7AC_hitCount;             // 0x7AC
    u8 m7AE_pad[2];
    s32 m7B0_finished;             // 0x7B0 — set to 1 when no valid targets remain
    s32 m7B4_shouldSetM100;        // 0x7B4
    s32 m7B8_hasCamera;            // 0x7B8
    // size 0x7BC
};

// BTL_A3::06083e4a
static void sHuntingScytheTask_Delete(sHuntingScytheTask* pThis)
{
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::06083bee
static void sHuntingScytheTask_Update(sHuntingScytheTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    pThis->m7AA_globalTimer++;

    switch (pThis->m7A4_state)
    {
    case 0:
    {
        // State 0: wait for file loading to complete
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
            return;

        pThis->m7A5_subState = 0;
        pThis->m7A4_state++;
        break;
    }
    case 1:
    {
        // State 1: model init + camera setup
        // TODO: init scythe 3D model from mA9C, set up camera with overlay data,
        // enable attack camera, init lighting
        // NOTE: this is where init3DModelRawData, battleEngine_enableAttackCamera,
        // battleEngine_setCurrentCameraPositionPointer etc. are called
        pThis->m7A4_state++;

        // Fall through to state 2
        [[fallthrough]];
    }
    case 2:
    {
        // State 2: main animation loop
        // Update camera positions with auto-scroll
        // TODO: auto-scroll camera source + target positions (param_1[0x1c..0x1e], param_1[0x25..0x27])

        // Step scythe model animation
        // TODO: stepAnimation(&pThis->model)

        // Update 16 trail entries (stride 0x6C each)
        // Each trail: position, rotation, sub-model (itemBoxType1InitSub0)
        // TODO: per-trail update loop

        // Sequential enemy targeting
        if (pThis->m7B0_finished == 0)
        {
            // Find next valid target
            while ((u32)pThis->m7A6_targetIndex < (u32)(s32)pTargetSystem->m20A_numSelectableEnemies)
            {
                s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->m7A6_targetIndex];
                if (enemy->m4_targetable != nullptr &&
                    (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                    enemy->m0_isActive > 0)
                {
                    // Valid target — scythe homes toward it
                    // TODO: homing physics, collision detection
                    // On hit: phantomSlasher_getBaseDamage + computeResistanceDamage + applyDamageToEnnemy
                    break;
                }
                pThis->m7A6_targetIndex++;
            }

            if ((u32)pThis->m7A6_targetIndex >= (u32)(s32)pTargetSystem->m20A_numSelectableEnemies)
            {
                pThis->m7B0_finished = 1;
            }
        }

        // Timer check — finish when expired
        if (pThis->m7B0_finished != 0)
        {
            pThis->m79C_timer--;
            if (pThis->m79C_timer < 0)
            {
                pThis->getTask()->markFinished();
            }
        }
        break;
    }
    default:
        break;
    }
}

// BTL_A3::06083b08
void berserk_createHuntingScythe(s_battleEngine* pThis)
{
    static const sHuntingScytheTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sHuntingScytheTask*))&sHuntingScytheTask_Update,
        nullptr,
        (void(*)(sHuntingScytheTask*))&sHuntingScytheTask_Delete,
    };

    sHuntingScytheTask* pNewTask = createSubTaskWithCopy<sHuntingScytheTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    // Init fields — faithful to Ghidra
    pNewTask->m7B8_hasCamera = 0;
    pNewTask->m7A6_targetIndex = 0;
    pNewTask->m7B0_finished = 0;
    pNewTask->m7B4_shouldSetM100 = 1;
    pNewTask->m7AA_globalTimer = 0;
    pNewTask->m7AC_hitCount = 0;
    pNewTask->m7A4_state = 0;
    pNewTask->m79C_timer = 0x147;
    pNewTask->m7A0_frameCounter = 0;
    pNewTask->m7AE_pad[0] = 0; // *(u16*)(0x7AE) = 0

    if (pTargetSystem->m20A_numSelectableEnemies == 0)
    {
        // No enemies — mark as finished immediately
        pNewTask->m7B0_finished = 1;
    }
    else
    {
        // Find first valid enemy target
        s32 idx = 0;
        while (idx < pTargetSystem->m20A_numSelectableEnemies)
        {
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[idx];
            if (enemy->m4_targetable != nullptr &&
                (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                enemy->m0_isActive > 0)
            {
                break; // valid target found
            }
            pNewTask->m7A6_targetIndex++;
            idx++;
        }

        // If no valid target found, mark finished
        if ((u32)pNewTask->m7A6_targetIndex == (u32)(s32)pTargetSystem->m20A_numSelectableEnemies)
        {
            pNewTask->m7B0_finished = 1;
        }
    }
}

//=============================================================================
// Energy Prism
//=============================================================================

struct sEnergyPrismTask : public s_workAreaTemplateWithCopy<sEnergyPrismTask>
{
    // 0x00: m0_fileBundle (inherited)
    // 0x04: m4_vd1Allocation (inherited)
    std::array<s32, 4> m8_perQuadrantAltitude; // 0x08 — per-quadrant camera altitude (param_3 to orbit)
    void (*m18_transformFunc)(sVec3_FP*, sVec3_FP*); // 0x18 — model transform function
    sVec3_FP m1C_transformOutput;   // 0x1C — transform result
    u8 m28_pad[4];
    fixedPoint m2C_trackSpeedSource; // 0x2C — base tracking speed
    u8 m30_pad[0x40];              // 0x30..0x6F: model data
    u8 m70_trailData[0x18];        // 0x70 — trail rendering data (passed to FUN_0609d57c)
    sVec3_FP m88_beamSource;       // 0x88 — beam source position (auto-scrolled)
    sVec3_FP m94_targetPosition;   // 0x94 — target position (tracked or auto-scrolled)
    sVec3_FP mA0_beamEndpoint;     // 0xA0 — interpolated beam endpoint
    sVec3_FP mAC_cameraPosition;   // 0xAC — camera orbit output
    fixedPoint mB8_trackProgress;  // 0xB8 — tracking progress (0→0x10000)
    fixedPoint mBC_trackSpeed;     // 0xBC — current tracking speed
    s32 mC0_rotationAngle;         // 0xC0 — oscillating rotation angle
    s32 mC4_rotationSpeed;         // 0xC4 — rotation speed (flips at boundaries)
    s8 mC8_state;                  // 0xC8
    u8 mC9_pad[7];
    s16 mD0_timer;                 // 0xD0
    u16 mD2_targetIndex;           // 0xD2
    s32 mD4_hasHit;                // 0xD4
    s32 mD8_isActive;              // 0xD8
    s32 mDC_hasCamera;             // 0xDC
    s32 mE0_hasLightEffect;        // 0xE0
    // size 0xE4
};

// BTL_A3::0608b56a
static void sEnergyPrismTask_Delete(sEnergyPrismTask* pThis)
{
    if (pThis->mDC_hasCamera != 0)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }
    if (pThis->mE0_hasLightEffect != 0)
    {
        energyPrism_restoreLighting();
    }
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
}

// BTL_A3::0608b9d0 — apply damage to target (Energy Prism specific)
static void energyPrism_applyDamage(sEnergyPrismTask* pThis)
{
    // TODO: faithful implementation of FUN_0608b9d0
    // Applies SPR-scaled damage to the target at mD2_targetIndex
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mD2_targetIndex];
    if (enemy->m4_targetable != nullptr &&
        (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
        enemy->m0_isActive > 0)
    {
        s16 dmg = computeSprScaledDamage(0x50); // NOTE: exact base damage needs r4 trace
        dmg = computeResistanceDamage(enemy->m4_targetable, dmg, 2);
        sVec3_FP* targetPos = getBattleTargetablePosition(*enemy->m4_targetable);
        sVec3_FP dir;
        dir[0] = pThis->m94_targetPosition[0] - pThis->m88_beamSource[0];
        dir[1] = pThis->m94_targetPosition[1] - pThis->m88_beamSource[1];
        dir[2] = pThis->m94_targetPosition[2] - pThis->m88_beamSource[2];
        applyDamageToEnnemy(enemy->m4_targetable, dmg, targetPos, 3, dir, fixedPoint(0x2000));
    }
}

// BTL_A3::0608b080
static void sEnergyPrismTask_Update(sEnergyPrismTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;

    // Update beam source with auto-scroll
    pThis->m88_beamSource += pEngine->m1A0_battleAutoScrollDelta;

    // Track target position (live if valid, else auto-scroll)
    if (pThis->mD4_hasHit == 0)
    {
        s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mD2_targetIndex];
        if (enemy->m4_targetable != nullptr &&
            (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
            enemy->m0_isActive > 0)
        {
            sVec3_FP* pos = getBattleTargetablePosition(*enemy->m4_targetable);
            pThis->m94_targetPosition = *pos;
        }
        else
        {
            pThis->m94_targetPosition += pEngine->m1A0_battleAutoScrollDelta;
        }
    }
    else
    {
        pThis->m94_targetPosition += pEngine->m1A0_battleAutoScrollDelta;
    }

    // Oscillate rotation angle (ping-pong between 0x1555555 and 0xEAAAAAB)
    pThis->mC0_rotationAngle += pThis->mC4_rotationSpeed;
    if ((s32)pThis->mC0_rotationAngle >= (s32)0xEAAAAAB)
    {
        pThis->mC4_rotationSpeed = (s32)0xFFEEEEEF; // -0x111111
    }
    else if ((s32)pThis->mC0_rotationAngle < 0x1555555)
    {
        pThis->mC4_rotationSpeed = 0x111111;
    }

    // Camera orbit
    berserkCameraOrbit(&pThis->mAC_cameraPosition, pThis->mC0_rotationAngle,
        pThis->m8_perQuadrantAltitude.data());

    // State machine
    switch (pThis->mC8_state)
    {
    case 0:
    {
        // State 0: beam approaches target
        pThis->mD8_isActive = 1;

        // Check if beam reached target (progress > 0xFFFF) or timer expired
        if ((s32)pThis->mB8_trackProgress > 0xFFFF)
        {
            // Beam hit — apply damage
            s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mD2_targetIndex];
            if (enemy->m4_targetable != nullptr &&
                (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                enemy->m0_isActive > 0)
            {
                energyPrism_applyDamage(pThis);
                pThis->mD4_hasHit = 1;
            }

            pThis->mC8_state++;
            pThis->mD0_timer = 0x29;
            battleEngine_setDesiredCameraPositionPointer(&pThis->m94_targetPosition);
            berserk_createHitFlashFade(pThis, 0x4A, 0xA8E7, 0xFE94, 2, 0xFE94, 0xA8E7, 0xF);
        }
        else
        {
            s16 t = pThis->mD0_timer;
            pThis->mD0_timer = t - 1;
            if (t < 0)
            {
                // Timer expired — same as hit
                s_battleEnemy* enemy = pTargetSystem->m0_enemyTargetables[pThis->mD2_targetIndex];
                if (enemy->m4_targetable != nullptr &&
                    (enemy->m4_targetable->m50_flags & 0x140003) == 0 &&
                    enemy->m0_isActive > 0)
                {
                    energyPrism_applyDamage(pThis);
                    pThis->mD4_hasHit = 1;
                }
                pThis->mC8_state++;
                pThis->mD0_timer = 0x29;
                battleEngine_setDesiredCameraPositionPointer(&pThis->m94_targetPosition);
                berserk_createHitFlashFade(pThis, 0x4A, 0xA8E7, 0xFE94, 2, 0xFE94, 0xA8E7, 0xF);
            }
        }
        break;
    }
    case 1:
    {
        // State 1: beam fades, restore camera
        if ((s32)pThis->mB8_trackProgress > 0x20000 && pThis->mDC_hasCamera != 0)
        {
            pThis->mD8_isActive = 0;
            battleEngine_restoreCameraAfterEnemyAttack();
            pThis->mDC_hasCamera = 0;
        }

        s16 t = pThis->mD0_timer;
        pThis->mD0_timer = t - 1;
        if (t < 0)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

    // Interpolate beam endpoint toward target while active
    if (pThis->mD8_isActive != 0)
    {
        // Transform source through model function
        if (pThis->m18_transformFunc != nullptr)
        {
            pThis->m18_transformFunc(&pThis->m1C_transformOutput, &pThis->m88_beamSource);
        }

        // Interpolate endpoint: source + (target - source) * progress
        sVec3_FP diff;
        diff[0] = MTH_Mul(pThis->m94_targetPosition[0] - pThis->m1C_transformOutput[0], pThis->mB8_trackProgress);
        diff[1] = MTH_Mul(pThis->m94_targetPosition[1] - pThis->m1C_transformOutput[1], pThis->mB8_trackProgress);
        diff[2] = MTH_Mul(pThis->m94_targetPosition[2] - pThis->m1C_transformOutput[2], pThis->mB8_trackProgress);

        pThis->mA0_beamEndpoint[0] = pThis->m1C_transformOutput[0] + diff[0];
        pThis->mA0_beamEndpoint[1] = pThis->m1C_transformOutput[1] + diff[1];
        pThis->mA0_beamEndpoint[2] = pThis->m1C_transformOutput[2] + diff[2];

        // Update trail positions
        // FUN_0609d57c(param_1 + 0x1c, param_1 + 0x28, &autoScroll)
        // TODO: implement trail position update

        // Advance tracking
        pThis->mBC_trackSpeed = pThis->m2C_trackSpeedSource;
        pThis->mB8_trackProgress = pThis->mB8_trackProgress + pThis->mBC_trackSpeed;
    }
}

// BTL_A3::0608b52a
static void sEnergyPrismTask_Draw(sEnergyPrismTask* pThis)
{
    // Render beam trail if active
    if (pThis->mDC_hasCamera != 0)
    {
        // FUN_0609d778 — renders beam trail VDP1 quads (at task offset 0x70)
        // TODO: implement beam trail renderer
    }
    // Render lens flare at beam endpoint if light effect active
    if (pThis->mE0_hasLightEffect != 0)
    {
        sVec3_FP screenPos;
        transformAndAddVecByCurrentMatrix(&pThis->mA0_beamEndpoint, &screenPos);
        dragonFieldTaskDrawSub1Sub1(screenPos.m0_X, screenPos.m4_Y, screenPos.m8_Z, 0x1E000);
    }
}

// BTL_A3::0608add8
void berserk_createEnergyPrism(s_battleEngine* pThis)
{
    static const sEnergyPrismTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sEnergyPrismTask*))&sEnergyPrismTask_Update,
        (void(*)(sEnergyPrismTask*))&sEnergyPrismTask_Draw,
        (void(*)(sEnergyPrismTask*))&sEnergyPrismTask_Delete,
    };

    sBattleOverlayTask_C* pTargetSystem = gBattleManager->m10_battleOverlay->mC_targetSystem;
    if (pTargetSystem->m20A_numSelectableEnemies == 0)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    sEnergyPrismTask* pNewTask = createSubTaskWithCopy<sEnergyPrismTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    // Zero flags (loop zeroes 0xD8..0xE3)
    pNewTask->mD8_isActive = 0;
    pNewTask->mDC_hasCamera = 0;
    pNewTask->mE0_hasLightEffect = 0;
    pNewTask->mD4_hasHit = 0;

    // FUN_0608b5ac — find first valid enemy target, stores index in mD2
    // TODO: implement faithfully (iterates enemies, finds first valid)
    pNewTask->mD2_targetIndex = 0;

    // Init beam source from dragon hotpoint (0x114 = mFC_hotpoints[2]) minus auto-scroll
    pNewTask->m88_beamSource = pDragon->mFC_hotpoints[2];
    pNewTask->m88_beamSource -= pThis->m1A0_battleAutoScrollDelta;

    // FUN_0608b7c0 — init beam endpoint tracking (stores target position)
    // TODO: implement faithfully

    // Init beam endpoint at dragon hotpoint
    pNewTask->mA0_beamEndpoint = pDragon->mFC_hotpoints[2];

    // FUN_0609d4dc — init trail segment data at offset 0x70
    // TODO: implement trail init

    // Init rotation and tracking
    pNewTask->mC0_rotationAngle = 0x1555555;
    pNewTask->mC4_rotationSpeed = 0x111111;

    // FUN_060a2efc — init per-quadrant altitude data at m8
    // TODO: implement faithfully

    // Camera orbit setup
    berserkCameraOrbit(&pNewTask->mAC_cameraPosition, pNewTask->mC0_rotationAngle,
        pNewTask->m8_perQuadrantAltitude.data());
    battleEngine_enableAttackCamera();
    battleEngine_setCurrentCameraPositionPointer(&pNewTask->mAC_cameraPosition);
    battleEngine_setDesiredCameraPositionPointer(&pNewTask->mA0_beamEndpoint);
    battleEngine_resetCameraInterpolation();

    // Set flags
    pNewTask->mDC_hasCamera = 1;
    pThis->m188_flags.m20000 = 1;
    syncM68KSoundCPU();

    // Lighting setup
    sVec3_FP lightColor;
    battleResultScreen_updateSub0(0x1F, 0x1F, 0x1F, &lightColor);
    battleGrid_setupLightInterpolation(0, pGrid->m1CC_lightColor, lightColor);

    sVec3_FP lightFalloff;
    battleResultScreen_updateSub0(0x02, 0x08, 0x08, &lightFalloff);
    battleGrid_setupLightInterpolation2(0, pGrid->m1E4_lightFalloff0, lightFalloff);

    energyPrism_initLightEffect();

    pNewTask->mE0_hasLightEffect = 1;
    pNewTask->mB8_trackProgress = 0;
    pNewTask->mBC_trackSpeed = fixedPoint(0x28F);
    pNewTask->mD0_timer = 300;
    pNewTask->mC8_state = 0;

    playSystemSoundEffect(8);
}

//=============================================================================
// Vengeance Orbs shield visual
//=============================================================================

struct sVengeanceOrbsTask : public s_workAreaTemplateWithCopy<sVengeanceOrbsTask>
{
    // 0x00: m0_fileBundle (inherited)
    // 0x04: m4_vd1Allocation (inherited)
    u8 m8_pad[4];                 // 0x08..0x0B
    s8 mC_state;                  // 0x0C — main state (0-5)
    u8 mD_pad[5];
    u16 m12_timer;                // 0x12
    s16 m14_damageAccum;          // 0x14 — accumulated damage from dragon
    u8 m16_pad[0x4A];             // 0x16..0x5F
    u8 m60_orbData[0x578];        // 0x60..0x5D7: 7 orbs at stride ~200 bytes each
    // Per-orb: offset base + 0xB9 (=0x119 from task start) = state byte
    //          offset base + 0xBA (=0x11A from task start) = action byte
    //          offset base + 0x54 (=0x114 from task start) = timer
    s32 m5D8_hasCamera;           // 0x5D8
    u8 m5DC_pad2[4];
    s32 m5E0_shouldSetM100;       // 0x5E0
    // size 0x5E4
};

// BTL_A3::0608f738
static void sVengeanceOrbsTask_Update(sVengeanceOrbsTask* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
    u8* taskBase = (u8*)pThis;

    // Accumulate damage from dragon's damage-this-frame field
    pThis->m14_damageAccum += pDragon->m1D6;

    switch (pThis->mC_state)
    {
    case 0:
    {
        // State 0: orbs spawning — count ready orbs
        s32 readyCount = 0;
        for (s32 i = 0; i < 7; i++)
        {
            // Per-orb state at byte 0x119 + i*200 (stride 0xC8)
            if (*(s8*)(taskBase + 0x119 + i * 0xC8) == 0)
            {
                readyCount++;
            }
        }

        if (readyCount == 0)
        {
            // All orbs ready → set m100 and advance
            gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m5E0_shouldSetM100 = 0;
            pThis->mC_state = 1;
            // Set draw method
        }

        // Fall through to shield monitoring
        [[fallthrough]];
    }
    case 1:
    {
        // State 1: shield active — monitor for attack or timeout
        if (battleEngine_isPlayerTurnActive() == 0)
        {
            u16 t = pThis->m12_timer;
            pThis->m12_timer = t + 1;
            if (t > 500)
            {
                // Timeout — start fade out
                pThis->mC_state = 5;
                for (s32 i = 0; i < 7; i++)
                {
                    *(s8*)(taskBase + 0x11A + i * 0xC8) = 1; // set orb action to fade
                }
                break;
            }
        }

        // Check if shield was removed
        if ((pDragon->m1C0_statusModifiers & 0x400) == 0)
        {
            pThis->mC_state = 5;
            for (s32 i = 0; i < 7; i++)
            {
                *(s8*)(taskBase + 0x11A + i * 0xC8) = 1;
            }
            break;
        }

        // Check for incoming attack (dragon targetable flags bit 0x80000)
        if ((pDragon->m8C.m50_flags & 0x80000) != 0)
        {
            pThis->mC_state = 2;
        }
        break;
    }
    case 2:
    {
        // State 2: attack detected — check shield still active, trigger battle mode
        if ((pDragon->m1C0_statusModifiers & 0x400) == 0)
        {
            pThis->mC_state = 5;
            for (s32 i = 0; i < 7; i++)
            {
                *(s8*)(taskBase + 0x11A + i * 0xC8) = 1;
            }
            break;
        }

        // Check for incoming attack
        if ((pDragon->m8C.m50_flags & 0x80000) != 0)
        {
            // FUN_060915f0 — init counterattack visual
            // TODO: implement faithfully
        }

        if (battleEngine_isBattleIntroFinished() != 0)
        {
            battleEngine_SetBattleMode(m4_useBerserk);
            pThis->m5E0_shouldSetM100 = 1;

            // Set all orbs to attack mode
            for (s32 i = 0; i < 7; i++)
            {
                *(s8*)(taskBase + 0x119 + i * 0xC8) = 2;
            }
            pThis->mC_state = 3;
        }
        break;
    }
    case 3:
    {
        // State 3: camera setup + wait for orb convergence
        if (pThis->m5D8_hasCamera == 1 &&
            *(s8*)(taskBase + 0x119) == 3 &&
            *(u16*)(taskBase + 0x114) > 10)
        {
            // FUN_0608fc54 — init counterattack damage camera
            // TODO: implement faithfully
            pThis->mC_state = 4;
        }
        break;
    }
    case 4:
    {
        // State 4: apply damage per orb
        // FUN_0609008e — per-orb damage application
        // TODO: implement faithfully (applies SPR-scaled damage per enemy)
        break;
    }
    case 5:
    {
        // State 5: fade out — clear camera, wait for orbs to finish
        pThis->m5D8_hasCamera = 0;
        // FUN_0608fbf6 — check if all orbs finished fading
        // TODO: implement faithfully
        // When done, finish task
        // For now, finish after a timeout
        pThis->m12_timer++;
        if (pThis->m12_timer > 60)
        {
            pThis->getTask()->markFinished();
        }
        break;
    }
    default:
        break;
    }

    // Per-orb update (runs every frame)
    // FUN_06090546 — per-orb orbit/animation update, called 7 times at stride 0xC8
    // TODO: implement faithfully
}

// BTL_A3::0608fb86
static void sVengeanceOrbsTask_Delete(sVengeanceOrbsTask* pThis)
{
    s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;

    if (pThis->m5D8_hasCamera == 1)
    {
        battleEngine_restoreCameraAfterEnemyAttack();
    }

    if ((pDragon->m1C0_statusModifiers & 0x400) != 0)
    {
        pDragon->m1C0_statusModifiers &= ~0x400;
    }
    pDragon->m1EC_shieldDuration = 0;

    if (pThis->m5E0_shouldSetM100 != 0)
    {
        gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m100_attackAnimationFinished = 1;
    }
}

// BTL_A3::0608f3a0
void berserk_createVengeanceOrbsShield(s_battleEngine* pThis)
{
    static const sVengeanceOrbsTask::TypedTaskDefinition definition = {
        nullptr,
        (void(*)(sVengeanceOrbsTask*))&sVengeanceOrbsTask_Update,
        nullptr,
        (void(*)(sVengeanceOrbsTask*))&sVengeanceOrbsTask_Delete,
    };

    sVengeanceOrbsTask* pNewTask = createSubTaskWithCopy<sVengeanceOrbsTask>(pThis, &definition);
    if (pNewTask == nullptr)
    {
        pThis->m188_flags.m100_attackAnimationFinished = 1;
        return;
    }

    // Init orb parameters
    // Ghidra: psVar1[0xd].m_nextNode = byte 0x68 → orb data table pointer
    // psVar1[0x21].m_nextNode = byte 0x108 → animation data pointer
    // Random timer at byte 0x112: range [25..34]
    u32 rnd = randomNumber();
    s32 timerOffset = (s32)MTH_Mul(rnd >> 16, fixedPoint(10));
    // *(s16*)((u8*)pNewTask + 0x112) = (s16)(timerOffset + 0x19);
    // NOTE: can't access 0x112 through proper members — within m8_pad blob

    // FUN_06090358 — init orb visual state (VDP1 setup, model loading)
    // TODO: implement faithfully

    // Second orb setup
    // psVar1[0x26].m_nextNode = byte 0x130 → second orb data
    // psVar1[0x3a].m_nextNode = byte 0x1D0 → second orb animation
    rnd = randomNumber();
    MTH_Mul(rnd >> 16, fixedPoint(7)); // random variant for second orb

    pNewTask->m5D8_hasCamera = 0;
    pNewTask->m5E0_shouldSetM100 = 1;
}
