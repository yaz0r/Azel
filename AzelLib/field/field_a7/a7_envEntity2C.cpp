#include "PDS.h"
#include "a7_envEntity2C.h"
#include "o_fld_a7.h"
#include "a7_beam.h"
#include "a7_sceneParticle.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldItemBox.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "trigo.h"
#include "field/fieldDragonInput.h"
#include "field/fieldScriptWaitTask.h"
#include "field/fieldAnimRingSubTask.h"
#include "field/fieldDebrisScatter.h"
#include "mainMenuDebugTasks.h"
#include "field/fieldVisibilityGrid.h"

s32 playBattleSoundEffect(s32 effectIndex);

// 06070ca4 — empty function (Ghidra: skyTransporter_noop). Faithfully called.
static void skyTransporter_noop(s32* /*pScratch*/)
{
}

// Stack-built arg passed to the 0xE0 child task.
// Saturn lays this out as {sSaturnPtr entry; u8* aliveFlag}.
struct sA7EnvEntity2CChildArg
{
    sSaturnPtr m0_entry;       // per-encounter 0x1C-byte table row
    u8*        m4_aliveFlag;   // pointer back into parent->m24_alive[i]
};

// 0xE0-byte child task spawned by sA7EnvEntity2C.
// Task definition at FLD_A7::06085cf0 = {init=06059844, update=06059930,
// draw=06059ab0, delete=null}.
struct sA7EnvEntity2CChild : public s_workAreaTemplateWithArg<sA7EnvEntity2CChild, sA7EnvEntity2CChildArg*>
{
    s_memoryAreaOutput          m0_memoryArea;        // 0x00 — file bundle / character area
    sVec3_FP                    m8_targetPos;         // 0x08 — entry[0..8]
    sVec3_FP                    m14_currentPos;       // 0x14 — drifts from m20
    sVec3_FP                    m20_velocity;         // 0x20 — integrated from m2C
    sVec3_FP                    m2C_accel;            // 0x2C — proportional to (target - current)
    sVec3_FP                    m38_rotation;         // 0x38 — (X,Y,Z) Euler, seeded from entry[0xC..0x14]; used by rotateCurrentMatrixYXZ
    s32                         m44_rotZVelocity;     // 0x44 — per-frame m38_rotation.Z delta (init 0)
    s32                         m48_countdownInitial; // 0x48 — entry[0x18], reload value
    s32                         m4C_countdown;        // 0x4C — counts down each frame
    u8*                         m50_aliveFlag;        // 0x50 — parent alive-slot pointer
    s16                         m54_state;            // 0x54
    s16                         m56_animCounter;      // 0x56
    s_3dModel                   m58_model;            // 0x58 — Saturn size 0x4C
    sFieldModelRenderContext    mA8_renderCtx;        // 0xA8 — size 0x34
    bool                        mDC_visible;          // 0xDC
    u8                          mDD_pad[3];           // 0xDD..0xDF
    // Saturn size 0xE0
};

// 06059844
static void a7EnvEntity2CChild_Init(sA7EnvEntity2CChild* pThis, sA7EnvEntity2CChildArg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);

    pThis->m8_targetPos.m0_X = fixedPoint(readSaturnS32(pArg->m0_entry + 0));
    pThis->m8_targetPos.m4_Y = fixedPoint(readSaturnS32(pArg->m0_entry + 4));
    pThis->m8_targetPos.m8_Z = fixedPoint(readSaturnS32(pArg->m0_entry + 8));

    pThis->m14_currentPos.m0_X = pThis->m8_targetPos.m0_X;
    pThis->m14_currentPos.m8_Z = pThis->m8_targetPos.m8_Z;

    u32 r = (u32)randomNumber();
    pThis->m14_currentPos.m4_Y =
        fixedPoint((s32)(((r & 3) + 1) * 0x1000) + pThis->m8_targetPos.m4_Y.m_value);

    pThis->m38_rotation.m0_X = fixedPoint(readSaturnS32(pArg->m0_entry + 0xC));
    pThis->m38_rotation.m4_Y = fixedPoint(readSaturnS32(pArg->m0_entry + 0x10));
    pThis->m38_rotation.m8_Z = fixedPoint(readSaturnS32(pArg->m0_entry + 0x14));
    pThis->m44_rotZVelocity  = 0;

    s32 countdown       = readSaturnS32(pArg->m0_entry + 0x18);
    pThis->m4C_countdown       = countdown;
    pThis->m48_countdownInitial = countdown;

    pThis->m50_aliveFlag  = pArg->m4_aliveFlag;
    pThis->m54_state      = 0;
    pThis->m56_animCounter = 0;

    // Saturn: init3DModelRawData(this, &m58, 0, bundle, 4,
    //                            *(bundle + 0x31c),    // anim pointer table entry
    //                            *(bundle + 0x248),    // pose pointer table entry
    //                            0, 0);
    //         stepAnimation(&m58);
    //         a7InitModelRenderContext(&mA8, this, 0x0605975a,
    //                                  &m14_currentPos, nullptr, 2, 0, 0xffff, 0, 0);
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(4);
    sStaticPoseData* pPose = pBundle->getStaticPose(0x248, pHierarchy->countNumberOfBones());
    init3DModelRawData(pThis, &pThis->m58_model, 0, pBundle, 4,
                       pBundle->getAnimation(0x31c), pPose, nullptr, nullptr);
    stepAnimation(&pThis->m58_model);
    initFieldModelRenderContext(&pThis->mA8_renderCtx, pThis, (void*)0x0605975a,
                                &pThis->m14_currentPos, nullptr, 2, 0, (s16)0xffff, 0, 0);
}
// Stack-built arg for FUN_06056b90 (spawns a 0x1c4-byte child effect task).
// Layout pulled from the state-0 branch of a7EnvEntity2CChild_Update.
struct sA7EnvEntity2CChild_EffectArg
{
    s32 m0_X;      // child->m14_currentPos.X
    s32 m4_Y;      // child->m14_currentPos.Y
    s32 m8_Z;      // child->m14_currentPos.Z
    s32 mC;        // 0x600000
    s32 m10;       // 0
    s32 m14;       // 0xffa00000
    s32 m18;       // 0xf000
};

// 0x1c4-byte effect sub-task spawned every `m48_countdownInitial` frames
// while `m54_state == 0`. Task definition at FLD_A7::060848b0 = {init=060569a0,
// update=06056938, draw=0605660c, delete=null}.
struct sA7EnvEntity2CEffectTask : public s_workAreaTemplateWithArg<sA7EnvEntity2CEffectTask, sA7EnvEntity2CChild_EffectArg*>
{
    s_memoryAreaOutput m0_memoryArea;    // 0x00..0x07
    // 0x08..0x193 — a 32-slot ring of sVec3_FP trail positions (0x180 bytes).
    // m8_trail[0] is the Init-time source position. Entries 0..31 start at
    // byte offsets 0x08, 0x14, ..., 0x17C; the draw wraps back to entry 0
    // after visiting entry 31. The init zeroes the m4_Y component of every
    // slot so the Draw's sentinel check can distinguish populated entries.
    sVec3_FP           m8_trail[32];      // 0x08..0x187
    u8                 m188_pad0[0xC];    // 0x188..0x193
    s32                m194_trailLength;  // 0x194 — number of valid trail entries
    sVec3_FP*          m198_pPosition;    // 0x198 — newest-trail pointer, wraps to &m8_trail[0]
    sVec3_FP           m19C_srcPos;       // 0x19C — arg[0..2] source position copy
    sVec3_FP           m1A8_tgtPos;       // 0x1A8 — arg[3..5] target position copy
    sVec3_FP           m1B4_offset;       // 0x1B4 — arg[6] * (cos(atan2(dZ,dX)), sin(atan2(dY,dX)), sin(atan2(dZ,dX)))
    s16                m1C0_counter;      // 0x1C0 — Update countdown (init = 0x2A)
    s16                m1C2;              // 0x1C2 — non-zero disables the per-segment width table
    // Saturn size 0x1c4
};

// 06056748 — alternate update installed when the initial countdown expires.
// While m1C2 == 0, each tick grows the reverse ring trail by advancing
// m198_pPosition one slot backward (wrapping from &m8_trail[0] to [31]) and
// writing head + m1B4_offset at the new slot. When the wrap lands back on
// [0], the value is mirrored into the 0xC-byte pad immediately after the
// ring so the Draw wrap logic can read a contiguous "+1" slot. Once the
// newly-written head.Y drops below zero the termination sequence begins:
// m1C2 is incremented and 4 scene particles are spawned at the head
// position with random X/Z jitter. While m1C2 != 0 the function ticks down
// m194_trailLength and marks the task finished when it reaches zero.
static void a7EnvEntity2CEffectTask_UpdateAlt_06056748(sA7EnvEntity2CEffectTask* pThis)
{
    if (pThis->m1C2 != 0)
    {
        if (pThis->m194_trailLength < 1)
        {
            if (pThis != nullptr)
            {
                pThis->getTask()->markFinished();
            }
        }
        else
        {
            pThis->m194_trailLength--;
        }
        return;
    }

    if (pThis->m194_trailLength < 0x1F)
    {
        pThis->m194_trailLength++;
    }

    const s32 newX = pThis->m198_pPosition->m0_X.m_value + pThis->m1B4_offset.m0_X.m_value;
    const s32 newY = pThis->m198_pPosition->m4_Y.m_value + pThis->m1B4_offset.m4_Y.m_value;
    const s32 newZ = pThis->m198_pPosition->m8_Z.m_value + pThis->m1B4_offset.m8_Z.m_value;

    if (pThis->m198_pPosition == &pThis->m8_trail[0])
    {
        pThis->m198_pPosition = &pThis->m8_trail[31];
        pThis->m198_pPosition->m0_X = fixedPoint(newX);
        pThis->m198_pPosition->m4_Y = fixedPoint(newY);
        pThis->m198_pPosition->m8_Z = fixedPoint(newZ);
    }
    else
    {
        pThis->m198_pPosition = pThis->m198_pPosition - 1;
        pThis->m198_pPosition->m0_X = fixedPoint(newX);
        pThis->m198_pPosition->m4_Y = fixedPoint(newY);
        pThis->m198_pPosition->m8_Z = fixedPoint(newZ);
        if (pThis->m198_pPosition == &pThis->m8_trail[0])
        {
            sVec3_FP* pMirror = reinterpret_cast<sVec3_FP*>(&pThis->m188_pad0[0]);
            pMirror->m0_X = fixedPoint(newX);
            pMirror->m4_Y = fixedPoint(newY);
            pMirror->m8_Z = fixedPoint(newZ);
        }
    }

    if (pThis->m198_pPosition->m4_Y.m_value < 0)
    {
        pThis->m1C2++;

        static std::vector<sVdp1Quad> s_deathSparkQuads;
        if (s_deathSparkQuads.empty())
        {
            s_deathSparkQuads = initVdp1Quad(gFLD_A7->getSaturnPtr(0x060804e4));
        }

        sA7SceneParticleDesc desc = {};
        desc.m8_pQuadList = &s_deathSparkQuads;

        sVec3_FP zeroVelocity;
        zeroVelocity.m0_X = fixedPoint(0);
        zeroVelocity.m4_Y = fixedPoint(0);
        zeroVelocity.m8_Z = fixedPoint(0);

        sVec3_FP spawnPos;
        spawnPos.m4_Y = fixedPoint(0);

        for (s32 i = 4; i != 0; i -= 2)
        {
            spawnPos.m0_X = fixedPoint(pThis->m198_pPosition->m0_X.m_value + a7CenteredRandom(0xFFFF));
            spawnPos.m8_Z = fixedPoint(pThis->m198_pPosition->m8_Z.m_value + a7CenteredRandom(0xFFFF));
            a7SceneParticle_spawnProjected(
                (sFieldSceneManager*)getFieldSpecificData_A7()->m280,
                &desc,
                &spawnPos,
                &zeroVelocity);

            spawnPos.m0_X = fixedPoint(pThis->m198_pPosition->m0_X.m_value + a7CenteredRandom(0xFFFF));
            spawnPos.m8_Z = fixedPoint(pThis->m198_pPosition->m8_Z.m_value + a7CenteredRandom(0xFFFF));
            a7SceneParticle_spawnProjected(
                (sFieldSceneManager*)getFieldSpecificData_A7()->m280,
                &desc,
                &spawnPos,
                &zeroVelocity);
        }
    }
}

// 060569a0
static void a7EnvEntity2CEffectTask_Init_060569a0(sA7EnvEntity2CEffectTask* pThis,
                                                  sA7EnvEntity2CChild_EffectArg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);

    pThis->m194_trailLength = 0;
    pThis->m198_pPosition   = &pThis->m8_trail[0];

    // Zero the m4_Y component of all 32 trail slots — this is the Saturn
    // sentinel loop (16 iters × 2 writes, stride 0xC) so Draw can detect
    // unpopulated slots via the negative-Y check.
    for (s32 i = 0; i < 32; i++)
    {
        pThis->m8_trail[i].m4_Y = fixedPoint(0);
    }

    // m8_trail[0] = source position (arg.m0/m4/m8_Z).
    pThis->m8_trail[0].m0_X = fixedPoint(pArg->m0_X);
    pThis->m8_trail[0].m4_Y = fixedPoint(pArg->m4_Y);
    pThis->m8_trail[0].m8_Z = fixedPoint(pArg->m8_Z);

    // Transient: store the (target - source) delta in m19C..m1A4 just long
    // enough to feed atan2 below. Those slots are overwritten at the end
    // with the raw source position.
    pThis->m19C_srcPos.m0_X = fixedPoint(pArg->mC  - pArg->m0_X);
    pThis->m19C_srcPos.m4_Y = fixedPoint(pArg->m10 - pArg->m4_Y);
    pThis->m19C_srcPos.m8_Z = fixedPoint(pArg->m14 - pArg->m8_Z);

    const fixedPoint magnitude(pArg->m18);

    // offset.X = mag * cos(atan2(dZ, dX))

    // offset.Y = mag * sin(atan2(dY, dX))

    // offset.Z = mag * sin(atan2(dZ, dX)) — same angle as offset.X but sin

    pThis->m1C2 = 0;

    // Spawn a scene particle at (source + offset) with zero velocity, using
    // quad data at FLD_A7::060806a4. Matches the Saturn pattern where the
    // caller of a7SceneParticle_spawnProjected only has to set m8_pQuadList
    // — the wrapper fills in m0/m4/m14/m18.
    sVec3_FP spawnPos;
    spawnPos.m0_X = fixedPoint(pArg->m0_X + pThis->m1B4_offset.m0_X.m_value);
    spawnPos.m4_Y = fixedPoint(pArg->m4_Y + pThis->m1B4_offset.m4_Y.m_value);
    spawnPos.m8_Z = fixedPoint(pArg->m8_Z + pThis->m1B4_offset.m8_Z.m_value);
    sVec3_FP zeroVelocity;
    zeroVelocity.m0_X = fixedPoint(0);
    zeroVelocity.m4_Y = fixedPoint(0);
    zeroVelocity.m8_Z = fixedPoint(0);

    sA7SceneParticleDesc desc = {};
    static std::vector<sVdp1Quad> s_sparkQuads;
    if (s_sparkQuads.empty())
    {
        s_sparkQuads = initVdp1Quad(gFLD_A7->getSaturnPtr(0x060806a4));
    }
    desc.m8_pQuadList = &s_sparkQuads;
    a7SceneParticle_spawnProjected(
        (sFieldSceneManager*)getFieldSpecificData_A7()->m280,
        &desc,
        &spawnPos,
        &zeroVelocity);

    // Lifetime / countdown for the Update's m1C0-branch.
    pThis->m1C0_counter = 0x2A;

    if (isWorldPositionOnScreen(&spawnPos))
    {
        playSystemSoundEffect(0x67);
    }

    // Overwrite the transient delta storage with the raw source + target
    // positions as the Ghidra tail does. From here on, m19C_srcPos holds
    // the source position and m1A8_tgtPos holds the target — this is the
    // layout the alt-update (06056748) and subsequent Update ticks assume.
    pThis->m19C_srcPos.m0_X = fixedPoint(pArg->m0_X);
    pThis->m19C_srcPos.m4_Y = fixedPoint(pArg->m4_Y);
    pThis->m19C_srcPos.m8_Z = fixedPoint(pArg->m8_Z);
    pThis->m1A8_tgtPos.m0_X = fixedPoint(pArg->mC);
    pThis->m1A8_tgtPos.m4_Y = fixedPoint(pArg->m10);
    pThis->m1A8_tgtPos.m8_Z = fixedPoint(pArg->m14);
}
// 06056938 — while m1C0_counter > 0, tick it down; when it hits 0, play
// sound 0x68 if the current position is on-screen and swap the update
// method to the 06056748 alternate.
static void a7EnvEntity2CEffectTask_Update_06056938(sA7EnvEntity2CEffectTask* pThis)
{
    if (pThis->m1C0_counter == 0)
    {
        if (isWorldPositionOnScreen(pThis->m198_pPosition) != 0)
        {
            playSystemSoundEffect(0x68);
        }
        pThis->m_UpdateMethod = &a7EnvEntity2CEffectTask_UpdateAlt_06056748;
    }
    else
    {
        pThis->m1C0_counter--;
    }
}
// 0605660c — walks the ring trail of `m194_trailLength - 1` segments. The
// first (head) segment uses sprite @ +0x1158, the middle segments use
// +0x1168 (with a per-segment width override driven by a 4x4 table at
// FLD_A7::06084870 indexed by the initial segment count and the current
// segment index), and the last (tail) segment uses +0x1178.
//
// Saturn constants (pool @ FLD_A7::060566ee / 060567ea):
//   default width = 0x1E66, cmdsize = 0x0210, cmdcolr = 0x2040, colorMode = 0
static void a7EnvEntity2CEffectTask_Draw_0605660c(sA7EnvEntity2CEffectTask* pThis)
{
    sVec3_FP* psVar3 = pThis->m198_pPosition;
    s32       iVar4  = pThis->m194_trailLength - 1;
    if (iVar4 <= 0)
    {
        return;
    }

    const u16 vdp1Base  = (u16)((pThis->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    const u16 cmdcolr   = 0x2040;
    const u16 cmdsize   = 0x0210;
    const u16 colorMode = 0;

    sVec3_FP* const trailLast  = &pThis->m8_trail[31]; // offset 0x17C
    sVec3_FP* const trailFirst = &pThis->m8_trail[0];  // offset 0x08

    // --- Head segment --------------------------------------------------
    vdp1EmitRayVertex_0602e136(
        psVar3,
        0x1E66,
        (u16)(vdp1Base + 0x1158),
        cmdsize,
        cmdcolr,
        colorMode);

    if (psVar3 == trailLast)
    {
        psVar3 = trailFirst;
    }
    else
    {
        psVar3 += 1;
    }

    // --- Width-table row selector, locked to the initial iVar4 --------
    // The Saturn table at FLD_A7::06084870 has 4 rows of 4 s32 entries
    // each (stride 0x10). `iVar2` picks the row based on where the
    // initial count-1 falls:
    //     iVar4 < 4 || iVar4 > 0x15 : 0
    //     iVar4 ∈ {5, 0x14}         : 2
    //     iVar4 ∈ {4, 0x15}         : 1
    //     else (6..0x13)            : 3
    s32 iVar2;
    if (iVar4 < 4 || iVar4 > 0x15)
    {
        iVar2 = 0;
    }
    else if (iVar4 == 5 || iVar4 == 0x14)
    {
        iVar2 = 2;
    }
    else if (iVar4 == 4 || iVar4 == 0x15)
    {
        iVar2 = 1;
    }
    else
    {
        iVar2 = 3;
    }

    // --- Middle loop ---------------------------------------------------
    // Saturn's Ghidra form: `while (iVar5 = iVar4 - 1, 1 < iVar5) { ... iVar4 = iVar5; }`
    // i.e. iVar4 walks down from the initial count-1 to 3 (inclusive); the
    // tail segment below then covers the iVar4 == 2 iteration.
    s32 iVar5;
    while ((iVar5 = iVar4 - 1) > 1)
    {
        s32 width;
        if (iVar5 < 6 && pThis->m1C2 == 0)
        {
            // table[iVar2][iVar4 - 3] : 4 rows (stride 0x10) × 4 columns (stride 4)
            sSaturnPtr tableEntry =
                gFLD_A7->getSaturnPtr(0x06084870 + iVar2 * 0x10 + (iVar4 - 3) * 4);
            width = readSaturnS32(tableEntry);
        }
        else
        {
            width = 0x1E66;
        }

        vdp1EmitRayVertex_0602e136(
            psVar3,
            width,
            (u16)(vdp1Base + 0x1168),
            cmdsize,
            cmdcolr,
            colorMode);

        iVar4 = iVar5;
        if (psVar3 == trailLast)
        {
            psVar3 = trailFirst;
        }
        else
        {
            psVar3 += 1;
        }
    }

    // --- Tail segment --------------------------------------------------
    if (iVar5 > 0)
    {
        vdp1EmitRayVertex_0602e136(
            psVar3,
            0x1E66,
            (u16)(vdp1Base + 0x1178),
            cmdsize,
            cmdcolr,
            colorMode);
    }
}

// 06056b90 — tail-calls createSiblingTaskWithArg(&060848b0, 0x1c4, arg).
static void a7EnvEntity2CChild_spawnEffect_06056b90(p_workArea parent,
                                                    sA7EnvEntity2CChild_EffectArg* pArg)
{
    static sA7EnvEntity2CEffectTask::TypedTaskDefinition td = {
        &a7EnvEntity2CEffectTask_Init_060569a0,
        &a7EnvEntity2CEffectTask_Update_06056938,
        &a7EnvEntity2CEffectTask_Draw_0605660c,
        nullptr,
    };
    createSiblingTaskWithArg<sA7EnvEntity2CEffectTask>(parent, pArg, &td);
}

// 060595a2 — sets up a terminal scene-particle descriptor from a palette
// snapshot (calls FUN_0607c18e / FUN_0607c120), then the caller adds the
// quad-list pointer and tail-calls a7SceneParticle_spawnProjected. The
// descriptor slot layout beyond m8_pQuadList isn't yet mapped, so the
// preparation is stubbed.
static void a7EnvEntity2CChild_setupTerminalSpawn_060595a2(sA7EnvEntity2CChild* pThis,
                                                           sA7SceneParticleDesc* /*pDesc*/)
{
    sDebrisScatterParams params;
    initDebrisScatterConfig(&params, 4, 0x248);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootOffset = READ_BE_U32(pRaw + 4);
    params.m8_spread = fixedPoint(readMaxScalarFromBundleTree(pRaw, treeRootOffset));

    params.m10_pPosition = &pThis->m14_currentPos;
    params.m14_pRotation = &pThis->m38_rotation;
    params.m0_gravity = fixedPoint(0xdc);
    params.m4_bounce = fixedPoint((s32)0xffffffff);
    params.mC_randomMask = fixedPoint(0x7fffff);
    params.m8_spread = MTH_Mul(params.m8_spread, fixedPoint(0x48000));

    // Compute directional velocity toward tower anchor (0x600000, _, -0x600000)
    s32 angle = atan2_FP(-0x600000 - pThis->m14_currentPos.m8_Z.m_value,
                          0x600000 - pThis->m14_currentPos.m0_X.m_value);
    u16 idx = (u16)((u32)(angle + 0xC000000) >> 16) & 0xFFF;
    params.m18_velX = MTH_Mul(fixedPoint(0x4000), getCos(idx));
    params.m1C_velY = fixedPoint(0x555);
    params.m20_velZ = MTH_Mul(fixedPoint(0x4000), getSin(idx));

    params.m_pBundle = pBundle;

    createDebrisScatterTask((p_workArea)pThis, &params, false);
}

// sA7EnvEntity2CChild_AnimArg is now sAnimRingArg
#define sA7EnvEntity2CChild_AnimArg sAnimRingArg

static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

// a7EnvEntity2CChild_spawnAnimSubTask -> spawnAnimRingSubTask (shared)

// 060596a0 — late-tick anim helper: samples a random angle, builds the arg
// struct, and tail-calls spawnAnimSubTask_0607fe24.
static void a7EnvEntity2CChild_animTick_060596a0(sA7EnvEntity2CChild* pThis)
{
    u32 r = (u32)randomNumber();
    u32 angleIdx = (r >> 16) & 0xFFF;
    fixedPoint sinA = getSin(angleIdx);
    fixedPoint cosA = getCos(angleIdx);

    sA7EnvEntity2CChild_AnimArg arg;

    // Three position samples rooted at the current position.
    arg.m00_center = pThis->m14_currentPos;

    arg.m0C_ring0 = pThis->m14_currentPos;
    arg.m0C_ring0.m0_X = fixedPoint(arg.m0C_ring0.m0_X.m_value + MTH_Mul(fixedPoint(0x5000), sinA).m_value);
    arg.m0C_ring0.m8_Z = fixedPoint(arg.m0C_ring0.m8_Z.m_value + MTH_Mul(fixedPoint(0x5000), cosA).m_value);

    arg.m18_ring1 = pThis->m14_currentPos;
    arg.m18_ring1.m0_X = fixedPoint(arg.m18_ring1.m0_X.m_value + MTH_Mul(fixedPoint(0xc000), sinA).m_value);
    arg.m18_ring1.m8_Z = fixedPoint(arg.m18_ring1.m8_Z.m_value + MTH_Mul(fixedPoint(0xc000), cosA).m_value);

    // Direction vector (absolute, not added to current).
    arg.m24_direction.m0_X = MTH_Mul(fixedPoint(0xf000), sinA);
    arg.m24_direction.m4_Y = fixedPoint(0x1e000);
    arg.m24_direction.m8_Z = MTH_Mul(fixedPoint(0xf000), cosA);

    arg.m30_lifetime = 3;
    arg.m34_mode     = 0;

    spawnAnimRingSubTask((p_workArea)pThis, &arg);
}

// 06059930
static void a7EnvEntity2CChild_Update(sA7EnvEntity2CChild* pThis)
{
    stepAnimation(&pThis->m58_model);

    pThis->m38_rotation.m8_Z = fixedPoint(pThis->m38_rotation.m8_Z.m_value + pThis->m44_rotZVelocity);

    pThis->m2C_accel.m0_X =
        MTH_Mul(fixedPoint(pThis->m8_targetPos.m0_X.m_value - pThis->m14_currentPos.m0_X.m_value),
                fixedPoint(0x189));
    pThis->m2C_accel.m4_Y =
        MTH_Mul(fixedPoint(pThis->m8_targetPos.m4_Y.m_value - pThis->m14_currentPos.m4_Y.m_value),
                fixedPoint(0x189));
    pThis->m2C_accel.m8_Z =
        MTH_Mul(fixedPoint(pThis->m8_targetPos.m8_Z.m_value - pThis->m14_currentPos.m8_Z.m_value),
                fixedPoint(0x189));

    pThis->m20_velocity.m0_X += pThis->m2C_accel.m0_X;
    pThis->m20_velocity.m4_Y += pThis->m2C_accel.m4_Y;
    pThis->m20_velocity.m8_Z += pThis->m2C_accel.m8_Z;

    pThis->m14_currentPos.m0_X += pThis->m20_velocity.m0_X;
    pThis->m14_currentPos.m4_Y += pThis->m20_velocity.m4_Y;
    pThis->m14_currentPos.m8_Z += pThis->m20_velocity.m8_Z;

    if (pThis->m4C_countdown == 0)
    {
        if (pThis->m54_state == 0)
        {
            sA7EnvEntity2CChild_EffectArg arg;
            arg.m0_X = pThis->m14_currentPos.m0_X.m_value;
            arg.m4_Y = pThis->m14_currentPos.m4_Y.m_value;
            arg.m8_Z = pThis->m14_currentPos.m8_Z.m_value;
            arg.mC   = 0x600000;
            arg.m10  = 0;
            arg.m14  = (s32)0xffa00000;
            arg.m18  = 0xf000;
            a7EnvEntity2CChild_spawnEffect_06056b90((p_workArea)pThis, &arg);
            pThis->m4C_countdown = pThis->m48_countdownInitial;
        }
        else
        {
            sA7SceneParticleDesc desc = {};
            a7EnvEntity2CChild_setupTerminalSpawn_060595a2(pThis, &desc);
            static std::vector<sVdp1Quad> s_terminalQuads;
            if (s_terminalQuads.empty())
            {
                s_terminalQuads = initVdp1Quad(gFLD_A7->getSaturnPtr(0x06080324));
            }
            desc.m8_pQuadList = &s_terminalQuads;
            a7SceneParticle_spawnProjected(
                (sFieldSceneManager*)getFieldSpecificData_A7()->m280,
                &desc,
                &pThis->m14_currentPos,
                &pThis->m20_velocity);
            playSystemSoundEffect(0x69);
            *pThis->m50_aliveFlag = 0;
            pThis->getTask()->markFinished();
        }
    }
    else
    {
        pThis->m4C_countdown--;
        if (pThis->m56_animCounter < 7)
        {
            pThis->m56_animCounter++;
        }
        else if (pThis->m54_state != 0)
        {
            a7EnvEntity2CChild_animTick_060596a0(pThis);
        }
    }

    updateFieldModelRenderContext(&pThis->mA8_renderCtx);
    s32 vis = checkPositionVisibilityAgainstFarPlane(&pThis->m14_currentPos);
    pThis->mDC_visible = (vis == 0);
}

// 06059ab0
static void a7EnvEntity2CChild_Draw(sA7EnvEntity2CChild* pThis)
{
    if (!pThis->mDC_visible)
    {
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m14_currentPos);
    rotateCurrentMatrixYXZ(&pThis->m38_rotation);

    // Per-frame scale from a 3-dword {sx, sy, sz} table at FLD_A7::06085c90,
    // indexed by (s8)(m56_animCounter.lowByte * 12).
    sSaturnPtr scaleTable = gFLD_A7->getSaturnPtr(0x06085c90);
    s8 scaleByteIdx = (s8)(((s8)(pThis->m56_animCounter & 0xFF)) * 12);
    sSaturnPtr row = scaleTable + (u32)(s32)scaleByteIdx;
    s32 sx = readSaturnS32(row + 0);
    s32 sy = readSaturnS32(row + 4);
    s32 sz = readSaturnS32(row + 8);
    scaleCurrentMatrixRow0(fixedPoint(sx));
    scaleCurrentMatrixRow1(fixedPoint(sy));
    scaleCurrentMatrixRow2(fixedPoint(sz));

    pThis->m58_model.m18_drawFunction(&pThis->m58_model);
    LCSItemBox_UpdateType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, 8, 0x24c, fixedPoint(0x10000));

    popMatrix();
}

// Mirrors `createSiblingTaskWithArg(parent, &06085cf0, 0xe0, &iStack_2c)` —
// the Saturn caller stacks {entry, &aliveFlag} and passes its address.
static p_workArea spawnA7EnvEntity2CChild(p_workArea parent, sSaturnPtr entry, u8* aliveFlag)
{
    static sA7EnvEntity2CChild::TypedTaskDefinition td = {
        &a7EnvEntity2CChild_Init,
        &a7EnvEntity2CChild_Update,
        &a7EnvEntity2CChild_Draw,
        nullptr,
    };
    sA7EnvEntity2CChildArg arg = { entry, aliveFlag };
    return (p_workArea)createSiblingTaskWithArg<sA7EnvEntity2CChild>(parent, &arg, &td);
}

static inline s32 a7_performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

extern void dispatchTutorialMultiChoiceSub2();

// 060732fc, 06073266 — clearDragonScriptFlag, dragonTransitionToNormal

// 06059b8c
void a7EnvEntity2C_Init(sA7EnvEntity2C* pThis, sSaturnPtr arg)
{
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if ((pSub->m370_fieldDebuggerWho & 2) != 0
        && pSub->m37C_debugMenuStatus1[1] == 0
        && pSub->m369 == 0
        && entryPoint != (s16)0xffff)
    {
        if (entryPoint != 0)
        {
            mainGameState.bitField[0x74] |= 0x20;
        }
        mainGameState.setPackedBits(0x599, 5, (u32)entryPoint);
    }

    playPCM((p_workArea)pThis, 0x66);

    pThis->m0_arg = arg;
    pThis->m4_count = (s32)mainGameState.readPackedBits(0x599, 5);
    getFieldSpecificData_A7()->m27C_pad[0] = (u8)pThis->m4_count;

    for (s32 i = 0; i < 5; i++)
    {
        pThis->m24_alive[i] = 0;
    }

    for (s32 i = 0; i < 5 && pThis->m4_count < 0xF; i++)
    {
        sSaturnPtr entry = arg + (u32)pThis->m4_count * 0x1C;
        p_workArea pChild = spawnA7EnvEntity2CChild((p_workArea)pThis, entry, &pThis->m24_alive[i]);
        pThis->m10_children[i] = pChild;
        if (pChild == nullptr)
        {
            pThis->m24_alive[i] = 0;
        }
        else
        {
            pThis->m24_alive[i] = 1;
            pThis->m4_count++;
        }
    }
    pThis->m8_iter = 0;
}

// 06059cd4
void a7EnvEntity2C_Update(sA7EnvEntity2C* pThis)
{
    s32 aliveCount = 0;
    s32 iter = pThis->m8_iter;
    s32 scratch[3] = { 0, 0, 0 }; // mirrors uStack_24/uStack_20/uStack_1c

    for (s32 i = 0; i < 5; i++)
    {
        if (pThis->m24_alive[i] == 0)
        {
            if (pThis->m4_count < 0xF)
            {
                sSaturnPtr entry = pThis->m0_arg + (u32)pThis->m4_count * 0x1C;
                p_workArea pChild = spawnA7EnvEntity2CChild((p_workArea)pThis, entry, &pThis->m24_alive[i]);
                pThis->m10_children[i] = pChild;
                if (pChild != nullptr)
                {
                    pThis->m24_alive[i] = 1;
                    pThis->m4_count++;
                }
                startFieldScript(0x10, 0x5d8);
                dragonTransitionToNormal();
            }
        }
        else
        {
            aliveCount++;
            if (iter == 0)
            {
                // Saturn copies child[i]->m14..m1C (current X/Y/Z) into the
                // scratch buffer.
                sA7EnvEntity2CChild* pChild = (sA7EnvEntity2CChild*)pThis->m10_children[i];
                scratch[0] = pChild->m14_currentPos.m0_X.m_value;
                scratch[1] = pChild->m14_currentPos.m4_Y.m_value;
                scratch[2] = pChild->m14_currentPos.m8_Z.m_value;
            }
            iter--;
        }
    }

    if (aliveCount == 0)
    {
        playBattleSoundEffect(0x66);
        mainGameState.bitField[0x97] |= 0x80;
        pThis->getTask()->markFinished();
    }
    else
    {
        if (pThis->m8_iter < aliveCount)
        {
            skyTransporter_noop(scratch);
        }
        u32 next = (u32)(pThis->mC_phaseCounter + 1) & 3;
        pThis->mC_phaseCounter = (s32)next;
        if (next == 0)
        {
            pThis->m8_iter = (s32)performModulo(aliveCount, pThis->m8_iter + 1);
        }
    }

    // Field-debugger debug overlay status line.
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    if ((pSub->m370_fieldDebuggerWho & 2) != 0
        && pSub->m37C_debugMenuStatus1[1] == 0
        && pSub->m369 == 0)
    {
        vdp2PrintStatus.m10_palette = 0x8000;
        vdp2DebugPrintSetPosition(1, 0x17);
        u32 packed = mainGameState.readPackedBits(0x599, 5);
        vdp2PrintfSmallFont("%02d %02d %01d ", pThis->m4_count, packed);
    }
}
