#include "PDS.h"
#include "BTL_A5_env.h"
#include "BTL_A5_data.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "kernel/graphicalObject.h"
#include "kernel/grid.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/cinematicBarsTask.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/battleGrid.h"
#include "battle/battleEngine.h"
#include "battle/battleDragon.h"
#include "town/town.h"
#include "3dModels.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "kernel/vdp1Allocator.h"

struct sBTL_A5_WindParticle : public s_workAreaTemplate<sBTL_A5_WindParticle>
{
    sVec3_FP m8_position;
    sVec3_FP m14_pad;
    sVec3_FP m20_velocity;
    sAnimatedQuad m2C_quad;
    s16 m34_timer;
    u8 m36_lifetime;
    u8 m37_direction;
    u8 m38_windType;
    // size 0x3C
};

// 0605950e
static void BTL_A5_windParticle_initVelocity(sBTL_A5_WindParticle* pThis)
{
    if (pThis->m38_windType == 3)
    {
        u32 r = randomNumber();
        pThis->m8_position.m4_Y = pThis->m8_position.m4_Y + (s32)((r & 0x7FFF) - 0x4000);
    }
    else if (pThis->m38_windType == 4 || pThis->m38_windType == 5 || pThis->m38_windType == 8)
    {
        s32 cx = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter.m0_X;
        u32 r = randomNumber();
        pThis->m8_position.m0_X = cx + (s32)((r & 0x7FFFF) - 0x40000);
        pThis->m8_position.m4_Y = 0;
        s32 cz = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter.m8_Z;
        r = randomNumber();
        pThis->m8_position.m8_Z = cz + (s32)((r & 0x7FFFF) - 0x40000);
    }
}

// 06059a68
static void BTL_A5_windParticle_Update(sBTL_A5_WindParticle* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    bool outOfBounds = false;

    if (pEngine->mC_battleCenter.m4_Y < pThis->m8_position.m4_Y - 0x40000 ||
        pThis->m8_position.m4_Y + 0x40000 < pEngine->mC_battleCenter.m4_Y ||
        pEngine->mC_battleCenter.m8_Z < pThis->m8_position.m8_Z - 0x80000)
    {
        outOfBounds = true;
    }

    pThis->m36_lifetime++;
    if (pThis->m36_lifetime > 0x100 || outOfBounds)
    {
        pThis->getTask()->markFinished();
    }

    pThis->m34_timer++;
    if (pThis->m34_timer > 0x31)
    {
        pThis->m34_timer = 0;
    }

    // 060595b8 — wind movement per type
    {
        s32 dX = 0, dY = 0, dZ = 0;
        s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
        switch (pThis->m38_windType)
        {
        case 1:
        {
            s32 r = randomNumber();
            dX = (s32)performModulo2(0x888, r) - 0x444;
            r = randomNumber();
            dY = (s32)performModulo2(0x1111, r) - 0x888;
            r = randomNumber();
            dZ = (s32)performModulo2(0x222, r);
            break;
        }
        case 2:
        {
            fixedPoint sinVal = getSin(0x71c71c >> 16);
            fixedPoint cosVal = getCos(0x71c71c >> 16);
            s32 px = (s32)pThis->m8_position.m0_X;
            s32 pz = (s32)pThis->m8_position.m8_Z;
            pThis->m8_position.m0_X = MTH_Mul(px, cosVal) - MTH_Mul(pz, sinVal);
            pThis->m8_position.m8_Z = MTH_Mul(px, sinVal) + MTH_Mul(pz, cosVal);
            s32 diffX = (s32)pEngine->mC_battleCenter.m0_X - (s32)pThis->m8_position.m0_X;
            s32 diffZ = (s32)pEngine->mC_battleCenter.m8_Z - (s32)pThis->m8_position.m8_Z;
            u32 dist = sqrt_F(FP_Pow2(diffX) + FP_Pow2(diffZ));
            if ((s32)dist < 0x11)
            {
                dX = 0; dY = 0x444; dZ = 0;
            }
            else
            {
                dX = (diffX < 0) ? -(s32)performModulo2(0x222, randomNumber()) : (s32)performModulo2(0x222, randomNumber());
                dZ = (diffZ < 0) ? -(s32)performModulo2(0x222, randomNumber()) : (s32)performModulo2(0x222, randomNumber());
                dY = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            }
            break;
        }
        case 3:
        {
            s32 r = randomNumber();
            dX = (s32)performModulo2(0x1111, r) - 0x888;
            r = randomNumber();
            dY = (s32)performModulo2(0x1111, r) - 0x888;
            r = randomNumber();
            dZ = (s32)performModulo2(0x888, r);
            break;
        }
        case 4:
        {
            u32 r = randomNumber();
            if ((r & 0xF) == 0)
            {
                dX = (s32)performModulo2(0x1111, randomNumber()) - 0x888;
                dZ = (s32)performModulo2(0x1111, randomNumber()) - 0x888;
            }
            else
            {
                dX = (s32)performModulo2(0x111, randomNumber()) - 0x88;
                dZ = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            }
            dY = (s32)performModulo2(0x88, randomNumber());
            break;
        }
        case 5:
        {
            dX = (s32)performModulo2(0x222, randomNumber()) - 0x111;
            dZ = (s32)performModulo2(0x222, randomNumber()) - 0x111;
            if ((s32)pThis->m8_position.m4_Y < 0x4001)
                dY = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            else
                dY = 0x88 - (s32)performModulo2(0x222, randomNumber());
            break;
        }
        case 8:
        {
            u32 r = randomNumber();
            if ((r & 0xF) == 0)
            {
                dX = -(s32)performModulo2(0x222, randomNumber());
                dZ = (s32)performModulo2(0x111, randomNumber());
                dY = (s32)performModulo2(0x222, randomNumber()) - 0x111;
            }
            else
            {
                dX = -(s32)performModulo2(0x1111, randomNumber());
                dZ = (s32)performModulo2(0x888, randomNumber());
                dY = (s32)performModulo2(0x1111, randomNumber()) - 0x888;
            }
            break;
        }
        case 9:
        {
            fixedPoint sinVal = getSin(0x38e38e >> 16);
            fixedPoint cosVal = getCos(0x38e38e >> 16);
            s32 diffX = (s32)pEngine->mC_battleCenter.m0_X - (s32)pThis->m8_position.m0_X;
            s32 diffZ = (s32)pEngine->mC_battleCenter.m8_Z - (s32)pThis->m8_position.m8_Z;
            s32 dist = sqrt_F(FP_Pow2(diffX) + FP_Pow2(diffZ));
            if (dist < 9)
                pThis->getTask()->markFinished();
            dX = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            dZ = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            if ((s32)pThis->m8_position.m4_Y < 0x8001)
                dY = (s32)performModulo2(0x111, randomNumber()) - 0x88;
            else
                dY = 0x666 - (s32)performModulo2(0x888, randomNumber());
            s32 px = (s32)pThis->m8_position.m0_X;
            s32 pz = (s32)pThis->m8_position.m8_Z;
            pThis->m8_position.m0_X = MTH_Mul(px, cosVal) - MTH_Mul(pz, sinVal) + diffX;
            pThis->m8_position.m8_Z = MTH_Mul(px, sinVal) + MTH_Mul(pz, cosVal) + diffZ;
            break;
        }
        default:
            break;
        }
        pThis->m20_velocity.m0_X = pThis->m20_velocity.m0_X + dX;
        pThis->m20_velocity.m4_Y = pThis->m20_velocity.m4_Y + dY;
        pThis->m20_velocity.m8_Z = pThis->m20_velocity.m8_Z + dZ;
    }
    pThis->m8_position.m0_X = pThis->m8_position.m0_X + pThis->m20_velocity.m0_X;
    pThis->m8_position.m4_Y = pThis->m8_position.m4_Y + pThis->m20_velocity.m4_Y;
    pThis->m8_position.m8_Z = pThis->m8_position.m8_Z + pThis->m20_velocity.m8_Z;

    sGunShotTask_UpdateSub4(&pThis->m2C_quad);
}

// 06059afa
static void BTL_A5_windParticle_Draw(sBTL_A5_WindParticle* pThis)
{
    drawProjectedParticle(&pThis->m2C_quad, &pThis->m8_position);
}

static std::vector<sVdp1Quad> s_BTL_A5_windQuadData;

// 0605947c
static void BTL_A5_createWindParticle(npcFileDeleter* pFileBundle, sVec3_FP* position, u8 windType)
{
    static const sBTL_A5_WindParticle::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_windParticle_Update,
        &BTL_A5_windParticle_Draw,
        nullptr,
    };

    sBTL_A5_WindParticle* pParticle = createSubTask<sBTL_A5_WindParticle>(pFileBundle, &def);
    if (pParticle)
    {
        pParticle->m8_position = *position;
        pParticle->m8_position.m4_Y = gBattleManager->m10_battleOverlay->m4_battleEngine->m270_enemyAltitude.m4_Y;

        pParticle->m34_timer = (s16)(randomNumber() % 0x32);
        pParticle->m36_lifetime = (u8)(randomNumber() & 0x7F);
        pParticle->m37_direction = (u8)(randomNumber() % 3);
        pParticle->m38_windType = windType;

        BTL_A5_windParticle_initVelocity(pParticle);

        if (s_BTL_A5_windQuadData.empty())
        {
            s_BTL_A5_windQuadData = initVdp1Quad(g_BTL_A5->getSaturnPtr(0x060b048c));
        }
        particleInitSub(&pParticle->m2C_quad, pFileBundle->m4_vd1Allocation ? pFileBundle->m4_vd1Allocation->m4_vdp1Memory : 0, &s_BTL_A5_windQuadData);
    }
}

struct sBTL_A5_WindTask : public s_workAreaTemplate<sBTL_A5_WindTask>
{
    sVec3_FP m8_position;
    u8 m74_pad[0x74 - 0x14];
    u16 m74_flags;
    s16 m76_directionCycle;
    s16 m78_spawnTimer;
    u8 m7A_windType;
    // size 0x7C
};

// 06058e44
static void BTL_A5_windTask_Init(sBTL_A5_WindTask* pThis)
{
    asyncDmaCopy(g_BTL_A5->getSaturnPtr(0x060adf20), getVdp2Cram(0x240), 0x20, 0);
    allocateNPC(pThis, 6);

    switch (gBattleManager->m6_subBattleId)
    {
    case 0: case 1: case 2: case 3:
        pThis->m7A_windType = 3;
        break;
    case 4: case 5: case 6: case 7: case 8: case 0xF:
        pThis->m7A_windType = 1;
        break;
    case 9: case 10: case 0xD: case 0xE:
        pThis->m7A_windType = 4;
        break;
    case 0xB:
        pThis->m7A_windType = 2;
        break;
    case 0xC:
        pThis->m7A_windType = 8;
        break;
    default:
        pThis->getTask()->markFinished();
        break;
    }

    pThis->m8_position.m0_X = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter.m0_X;
    pThis->m8_position.m4_Y = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter.m4_Y;
    pThis->m8_position.m8_Z = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter.m8_Z;

    pThis->m76_directionCycle = 3;

    sSaturnPtr offsetTable = g_BTL_A5->getSaturnPtr(0x060adf40);
    npcFileDeleter* pFile = dramAllocatorEnd[6].mC_fileBundle;
    s32 baseX = pThis->m8_position.m0_X + readSaturnS32(offsetTable + (s8)((s8)(pThis->m76_directionCycle >> 8) * 12));
    s32 baseZ = pThis->m8_position.m8_Z + -0x30000;

    for (int i = 0; i < 8; i++)
    {
        sVec3_FP pos;
        pos.m0_X = baseX + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
        pos.m4_Y = 0;
        pos.m8_Z = baseZ + (s32)((randomNumber() & 0x7FFFF) - 0x60000);
        BTL_A5_createWindParticle(pFile, &pos, pThis->m7A_windType);
    }
}

// 060591a0
static void BTL_A5_windTask_Update(sBTL_A5_WindTask* pThis)
{
    if (gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags & 0x40)
    {
        pThis->m74_flags |= 1;
        return;
    }

    pThis->m74_flags &= ~1;

    pThis->m78_spawnTimer++;
    if (pThis->m78_spawnTimer < 0x21)
        return;

    pThis->m78_spawnTimer = 0;
    pThis->m8_position = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter;

    pThis->m76_directionCycle--;
    if (pThis->m76_directionCycle < 0)
    {
        pThis->m76_directionCycle = 3;
    }

    sSaturnPtr offsetTable = g_BTL_A5->getSaturnPtr(0x060adf40);
    npcFileDeleter* pFile = dramAllocatorEnd[6].mC_fileBundle;
    s32 baseX = pThis->m8_position.m0_X + readSaturnS32(offsetTable + (s8)((s8)((pThis->m76_directionCycle >> 8) & 0xFF) * 12));
    s32 baseZ = pThis->m8_position.m8_Z + -0x30000;

    for (int i = 0; i < 8; i++)
    {
        sVec3_FP pos;
        pos.m0_X = baseX + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
        pos.m4_Y = 0;
        pos.m8_Z = baseZ + (s32)((randomNumber() & 0x7FFFF) - 0x60000);
        BTL_A5_createWindParticle(pFile, &pos, pThis->m7A_windType);
    }
}

// 0605945a
static void BTL_A5_windTask_Delete(sBTL_A5_WindTask*)
{
    decreaseNPCRefCount(6);
}

// 0605a4cc
void BTL_A5_buildGroundRotation(s_BTL_A3_Env* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 rotX = (s32)pThis->m18_cameraRotation.m0_X;
    if (rotX == 0)
    {
        rotX = -0x12345;
    }
    s32 rotY = (s32)pThis->m18_cameraRotation.m4_Y;
    s32 rotZ = (s32)pThis->m18_cameraRotation.m8_Z;

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C);
    writeRotationParams(-rotZ);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m3C, (s32)pThis->mC_cameraPosition.m0_X << 4)
                    - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
                    + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m3C, (s32)pThis->mC_cameraPosition.m8_Z << 4)
                    - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
                    + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = (pThis->mC_cameraPosition.m4_Y - pThis->m38) * 0x10
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

// 0605a2e8
void BTL_A5_env_Draw(s_BTL_A3_Env* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    BTL_A5_buildGroundRotation(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();

    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = (0xFF - pThis->m34) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(scrollX, scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

struct sBTL_A5_PaletteAnim : public s_workAreaTemplate<sBTL_A5_PaletteAnim>
{
    u8 m0_palette[0x60];
    s8 m60_frameCounter;
    s8 m61_state;
    // size 0x62 on Saturn; C++ adds:
    u32 m_paletteDataAddr;
    u32 m_indexTableAddr;
};

static void BTL_A5_paletteAnimUpdate(sBTL_A5_PaletteAnim* pThis)
{
    sSaturnPtr paletteData = gCurrentBattleOverlay->getSaturnPtr(pThis->m_paletteDataAddr);
    sSaturnPtr indexTable = gCurrentBattleOverlay->getSaturnPtr(pThis->m_indexTableAddr);

    if (pThis->m61_state == 0)
    {
        for (int i = 0; i < 0x60; i++)
            pThis->m0_palette[i] = readSaturnU8(paletteData + i);
        pThis->m61_state++;
    }
    else if (pThis->m61_state == 1)
    {
        s8 frame = pThis->m60_frameCounter++;
        if (frame >= 6)
        {
            pThis->m60_frameCounter -= 6;
        }

        u8* pal = pThis->m0_palette;
        for (int i = 0; i < 6; i++)
        {
            s32 dst = i * 2;
            s32 src = readSaturnS32(indexTable + (pThis->m60_frameCounter + i) * 4) * 2;
            pal[dst + 2]     = readSaturnU8(paletteData + src);
            pal[dst + 3]     = readSaturnU8(paletteData + src + 1);
            pal[dst + 0xE]   = readSaturnU8(paletteData + src + 0xC);
            pal[dst + 0xF]   = readSaturnU8(paletteData + src + 0xD);
            pal[dst + 0x22]  = readSaturnU8(paletteData + src + 0x20);
            pal[dst + 0x23]  = readSaturnU8(paletteData + src + 0x21);
            pal[dst + 0x2E]  = readSaturnU8(paletteData + src + 0x2C);
            pal[dst + 0x2F]  = readSaturnU8(paletteData + src + 0x2D);
            pal[dst + 0x42]  = readSaturnU8(paletteData + src + 0x40);
            pal[dst + 0x43]  = readSaturnU8(paletteData + src + 0x41);
            pal[dst + 0x4E]  = readSaturnU8(paletteData + src + 0x4C);
            pal[dst + 0x4F]  = readSaturnU8(paletteData + src + 0x4D);
        }
        asyncDmaCopy(pThis->m0_palette, getVdp2Cram(0xC00), 0x60, 2);
    }
}

sBTL_A5_PaletteAnim* createPaletteAnimTask(p_workArea parent, u32 paletteDataAddr, u32 indexTableAddr)
{
    static const sBTL_A5_PaletteAnim::TypedTaskDefinition paletteDef = {
        nullptr,
        &BTL_A5_paletteAnimUpdate,
        nullptr,
        nullptr,
    };
    sBTL_A5_PaletteAnim* pTask = createSubTask<sBTL_A5_PaletteAnim>(parent, &paletteDef);
    if (pTask)
    {
        pTask->m_paletteDataAddr = paletteDataAddr;
        pTask->m_indexTableAddr = indexTableAddr;
    }
    return pTask;
}

// 06059d40
static void BTL_A5_env_InitVdp2(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(g_BTL_A5->getSaturnPtr(0x060ae27c), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(g_BTL_A5->getSaturnPtr(0x060ae07c), getVdp2Cram(0xA00), 0x200, 0);

    static const sLayerConfig rgb0Setup[] = {
        m2_CHCN,  1,
        m5_CHSZ,  1,
        m6_PNB,   1,
        m7_CNSM,  0,
        m27_RPMD, 2,
        m11_SCN,  8,
        m34_W0E,  1,
        m37_W0A,  1,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotationParams[] = {
        m31_RxKTE, 1,
        m0_END,
    };
    setupRotationParams(rotationParams);

    static const sLayerConfig rotationParams2[] = {
        m0_END,
    };
    setupRotationParams2(rotationParams2);

    loadFile("FNS_A5_0.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A5_0.PNB", getVdp2Vram(0x1C000), 0);
    loadFile("FRS_A5_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A5_0.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A5->getSaturnPtr(0x060af5f4));
    setupRotationMapPlanes(1, g_BTL_A5->getSaturnPtr(0x060af634));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    *(u16*)getVdp2Vram(0x2A600) = 0xBF3B;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x40000;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = {
        { {0x2C, 0x1} }
    };
    applyLayerDisplayConfig(layerDisplayConfig);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 0x3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m54 = 0x1f;
    pThis->m50 = 0x0D;
    pThis->m51 = 0x18;
    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;

    pThis->m55 = 8;
    vdp2Controls.m4_pendingVdp2Regs->m10C_CCRR = pThis->m55;
    vdp2Controls.m_isDirty = 1;

    createPaletteAnimTask(pThis, 0x060adffc, 0x060af5c4);

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_A5->getSaturnPtr(0x060af6fc));

    {
        static const sBTL_A5_WindTask::TypedTaskDefinition windDef = {
            &BTL_A5_windTask_Init,
            &BTL_A5_windTask_Update,
            nullptr,
            &BTL_A5_windTask_Delete,
        };
        createSubTask<sBTL_A5_WindTask>(pThis, &windDef);
    }
}

struct sBTL_A5_CorridorParticle : public s_workAreaTemplate<sBTL_A5_CorridorParticle>
{
    sVec3_FP m8_position;
    u8 m14_pad[0x9C - 0x14];
    s16 m9C_timer;
    u8 m9E_direction;
    // size 0xA0
};

// 0605a82a
static void BTL_A5_corridorParticle_Update(sBTL_A5_CorridorParticle* pThis)
{
    s32 dragonZ = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;
    if (dragonZ < (s32)pThis->m8_position.m8_Z - 0x200000)
    {
        pThis->getTask()->markFinished();
    }
    pThis->m9C_timer++;
    if (pThis->m9C_timer > 0x31)
    {
        pThis->m9C_timer = 0;
    }
}

// 0605a874
static void BTL_A5_corridorParticle_Draw(sBTL_A5_CorridorParticle* pThis)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    s32 angle = atan2_FP(pGrid->m180_cameraTranslation.m0_X - (s32)pThis->m8_position.m0_X,
                         pGrid->m180_cameraTranslation.m8_Z - (s32)pThis->m8_position.m8_Z);
    rotateCurrentMatrixShiftedY(angle + 0x8000000);

    s_fileBundle* pBundle = dramAllocatorEnd[0x16].mC_fileBundle->m0_fileBundle;
    s16 modelOffset;
    s16 poseOffset;
    if (pThis->m9E_direction == 0)
    {
        modelOffset = readSaturnS16(g_BTL_A5->getSaturnPtr(0x060af240) + pThis->m9C_timer * 2);
        poseOffset = (s16)readSaturnS32(g_BTL_A5->getSaturnPtr(0x060af2a4) + pThis->m9C_timer * 4);
    }
    else if (pThis->m9E_direction == 1)
    {
        modelOffset = readSaturnS16(g_BTL_A5->getSaturnPtr(0x060af36c) + pThis->m9C_timer * 2);
        poseOffset = (s16)readSaturnS32(g_BTL_A5->getSaturnPtr(0x060af434) + pThis->m9C_timer * 4);
    }
    else if (pThis->m9E_direction == 2)
    {
        modelOffset = readSaturnS16(g_BTL_A5->getSaturnPtr(0x060af3d0) + pThis->m9C_timer * 2);
        poseOffset = (s16)readSaturnS32(g_BTL_A5->getSaturnPtr(0x060af4fc) + pThis->m9C_timer * 4);
    }
    else
    {
        popMatrix();
        return;
    }
    LCSItemBox_DrawType0Sub0(pBundle, modelOffset, poseOffset);
    popMatrix();
}

// 0605a7d4
static void BTL_A5_createCorridorParticle(npcFileDeleter* pFileBundle, sVec3_FP* position)
{
    static const sBTL_A5_CorridorParticle::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_corridorParticle_Update,
        &BTL_A5_corridorParticle_Draw,
        nullptr,
    };
    sBTL_A5_CorridorParticle* p = createSubTask<sBTL_A5_CorridorParticle>(pFileBundle, &def);
    if (p)
    {
        p->m8_position = *position;
        p->m8_position.m4_Y = 0;
        p->m9C_timer = (s16)(randomNumber() % 0x32);
        p->m9E_direction = (u8)(randomNumber() % 3);
    }
}

// 0605a954
static void BTL_A5_corridorTask_Init(s_BTL_A3_Env* pThis)
{
    allocateNPC(pThis, 0x16);

    sVec3_FP dragonPos;
    dragonPos.m0_X = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m0_X;
    dragonPos.m4_Y = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m4_Y;
    dragonPos.m8_Z = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;

    npcFileDeleter* pFile = dramAllocatorEnd[0x16].mC_fileBundle;
    sSaturnPtr offsetTable = g_BTL_A5->getSaturnPtr(0x060af8c8);

    for (int group = 3; group >= 0; group--)
    {
        s32 offX = readSaturnS32(offsetTable + group * 12);
        s32 offY = readSaturnS32(offsetTable + group * 12 + 4);
        s32 offZ = readSaturnS32(offsetTable + group * 12 + 8);
        for (int i = 0; i < 8; i++)
        {
            sVec3_FP pos;
            pos.m0_X = dragonPos.m0_X + offX + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
            pos.m4_Y = 0;
            pos.m8_Z = dragonPos.m8_Z + offZ + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
            BTL_A5_createCorridorParticle(pFile, &pos);
        }
    }
}

// 0605ac48
static void BTL_A5_corridorTask_Update(s_BTL_A3_Env* pThis)
{
    if (gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags & 0x40)
    {
        return;
    }

    s32 dragonZ = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;
    if (dragonZ < (s32)pThis->mC_cameraPosition.m8_Z - 0x100000)
    {
        pThis->mC_cameraPosition.m0_X = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m0_X;
        pThis->mC_cameraPosition.m4_Y = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m4_Y;
        pThis->mC_cameraPosition.m8_Z = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;

        sSaturnPtr offsetTable = g_BTL_A5->getSaturnPtr(0x060af8a4);
        npcFileDeleter* pFile = dramAllocatorEnd[0x16].mC_fileBundle;

        s8 dirIdx = (s8)((pThis->m38 >> 8) & 0xFF);
        pThis->m38--;
        if ((s16)pThis->m38 < 0)
        {
            pThis->m38 = 3;
        }

        s32 baseX = (s32)pThis->mC_cameraPosition.m0_X + readSaturnS32(offsetTable + dirIdx * 12);
        s32 baseZ = (s32)pThis->mC_cameraPosition.m8_Z - 0x300000;

        for (int i = 0; i < 8; i++)
        {
            sVec3_FP pos;
            pos.m0_X = baseX + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
            pos.m4_Y = 0;
            pos.m8_Z = baseZ + (s32)((randomNumber() & 0x7FFFF) - 0x40000);
            BTL_A5_createCorridorParticle(pFile, &pos);
        }
    }
}

// 0605ae0a
static void BTL_A5_corridorTask_Delete(s_BTL_A3_Env*)
{
    decreaseNPCRefCount(0x16);
}

// 0605a168
static void BTL_A5_env_Init(s_BTL_A3_Env* pThis)
{
    BTL_A5_env_InitVdp2(pThis);

    static const s_BTL_A3_Env::TypedTaskDefinition corridorDef = {
        (void(*)(s_BTL_A3_Env*))&BTL_A5_corridorTask_Init,
        (void(*)(s_BTL_A3_Env*))&BTL_A5_corridorTask_Update,
        nullptr,
        &BTL_A5_corridorTask_Delete,
    };
    createSubTask<s_BTL_A3_Env>(pThis, &corridorDef);
}

// 0605a014
static void BTL_A5_env_Init_grid(s_BTL_A3_Env* pThis)
{
    BTL_A5_env_InitVdp2(pThis);

    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;
    initGridForBattle(dramAllocatorEnd[6].mC_fileBundle, readGrid(g_BTL_A5->getSaturnPtr(0x060ae6ac), 2, 4), 2, 4, 0x200000);
    pThis->m58 = dramAllocatorEnd[6].mC_fileBundle;
}

// 0605a738
p_workArea Create_BTL_A5_env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A5_env_Init,
        &BTL_A3_Env_Update,
        &BTL_A5_env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}

// 0605a71a
p_workArea Create_BTL_A5_env_grid(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A5_env_Init_grid,
        &BTL_A3_Env_Update,
        &BTL_A5_env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
