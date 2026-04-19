#include "PDS.h"
#include "BTL_A5_2_env.h"
#include "BTL_A5_2_data.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleGrid.h"
#include "kernel/graphicalObject.h"
#include "kernel/grid.h"
#include "kernel/loadSavegameScreen.h"
#include "kernel/cinematicBarsTask.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/battleDragon.h"
#include "town/town.h"

// 060627b0
static void BTL_A5_2_buildGroundRotation(s_BTL_A3_Env* pThis, s32 yOffset)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 rotX = (s32)pThis->m18_cameraRotation.m0_X;
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
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38 + yOffset) * 0x10)
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

// 06062954
static s32 BTL_A5_2_computeHorizonAndApplyAlpha(sSaturnPtr alphaTable)
{
    auto& table = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 count = 0x1A8;
    for (int i = 0; i < 0x1A8; i++)
    {
        if ((s32)table[i] >= 0)
        {
            s32 horizon = 0x143 - (0x1A8 - i);
            if (!alphaTable.isNull())
            {
                for (int j = 0x1A8 - i; j > 0; j--)
                {
                    s8 alpha = readSaturnS8(alphaTable + 0x200 + j - 1);
                    table[i + (0x1A8 - i - j)] = (table[i + (0x1A8 - i - j)] & 0x80FFFFFF) | ((u32)(u8)alpha << 24);
                }
            }
            return horizon;
        }
    }
    return 0x143;
}

// 060629ae
static void BTL_A5_2_applyAlphaBelowHorizon(sSaturnPtr alphaTable)
{
    auto& table = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    for (int i = 0; i < 0x1A8; i++)
    {
        if ((s32)table[i] < 0)
        {
            if (alphaTable.isNull()) return;
            for (int j = i; j > 0; j--)
            {
                s8 alpha = readSaturnS8(alphaTable + 0x200 + j);
                table[j] = (table[j] & 0x80FFFFFF) | ((u32)(u8)alpha << 24);
            }
            return;
        }
    }
}

// 060625e8
static void BTL_A5_2_env_Update(s_BTL_A3_Env* pThis)
{
    vdp2Controls.m20_registers[0].m108_CCRNA = (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | pThis->m5D_pad[0];
    vdp2Controls.m20_registers[1].m108_CCRNA = vdp2Controls.m20_registers[0].m108_CCRNA;
    pThis->m38 = 0;
}

// 06062678
static void BTL_A5_2_env_Draw(s_BTL_A3_Env* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    BTL_A5_2_buildGroundRotation(pThis, 0);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = BTL_A5_2_computeHorizonAndApplyAlpha(g_BTL_A5_2->getSaturnPtr(0x060b0ff8));

    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = pThis->m34 * 0x10000 + -0x20000;

    beginRotationPass(1, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    BTL_A5_2_buildGroundRotation(pThis, (s32)0xFFF9C000);
    drawCinematicBar(7);
    commitRotationPass();

    BTL_A5_2_applyAlphaBelowHorizon(g_BTL_A5_2->getSaturnPtr(0x060b0ff8));

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

struct sBTL_A5_2_DebrisParticle : public s_workAreaTemplate<sBTL_A5_2_DebrisParticle>
{
    sVec3_FP m8_position;
    u8 m14_pad[0x3C - 0x14];
    s32 m3C_rotationAngle;
    s32 m44_scaleX;
    s32 m48_scaleY;
    s32 m4C_scaleFactorX;
    s32 m50_scaleFactorY;
    // size 0x54
};

// 06062a98
static void BTL_A5_2_debrisParticle_Update(sBTL_A5_2_DebrisParticle* pThis)
{
    s32 dragonZ = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;
    if (dragonZ < (s32)pThis->m8_position.m8_Z - 0x200000)
    {
        pThis->getTask()->markFinished();
    }
}

// 06062b00
static void BTL_A5_2_debrisParticle_Draw(sBTL_A5_2_DebrisParticle* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixShiftedZ(pThis->m3C_rotationAngle);
    popMatrix();
    Unimplemented();
}

static const s32 s_debrisAngles[] = { 0x71c71c, 0x9f49f4, 0xcccccc, 0xfa4fa4 };

// 06062a28
static void BTL_A5_2_createDebrisParticle(npcFileDeleter* pFileBundle, sVec3_FP* position)
{
    static const sBTL_A5_2_DebrisParticle::TypedTaskDefinition def = {
        nullptr,
        &BTL_A5_2_debrisParticle_Update,
        &BTL_A5_2_debrisParticle_Draw,
        nullptr,
    };
    sBTL_A5_2_DebrisParticle* p = createSubTask<sBTL_A5_2_DebrisParticle>(pFileBundle, &def);
    if (p)
    {
        p->m8_position = *position;
        p->m44_scaleX = 0x10000;
        p->m48_scaleY = 0x10000;
        p->m4C_scaleFactorX = 0x3000;
        p->m50_scaleFactorY = 0xA000;
        p->m3C_rotationAngle = s_debrisAngles[randomNumber() & 3];
    }
}

// 06062be0
static void BTL_A5_2_debrisTask_Init(s_BTL_A3_Env* pThis)
{
    allocateNPC(pThis, 6);

    sVec3_FP dragonPos;
    dragonPos.m0_X = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m0_X;
    dragonPos.m4_Y = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m4_Y;
    dragonPos.m8_Z = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;

    npcFileDeleter* pFile = dramAllocatorEnd[6].mC_fileBundle;
    sSaturnPtr offsetTable = g_BTL_A5_2->getSaturnPtr(0x060b1874);

    for (int group = 3; group >= 0; group--)
    {
        sVec3_FP pos;
        pos.m0_X = dragonPos.m0_X + readSaturnS32(offsetTable + group * 12);
        pos.m4_Y = readSaturnS32(offsetTable + group * 12 + 4);
        pos.m8_Z = dragonPos.m8_Z + readSaturnS32(offsetTable + group * 12 + 8);
        BTL_A5_2_createDebrisParticle(pFile, &pos);

        pos.m0_X = dragonPos.m0_X + readSaturnS32(offsetTable + (group - 3) * 12);
        pos.m4_Y = readSaturnS32(offsetTable + (group - 3) * 12 + 4);
        pos.m8_Z = dragonPos.m8_Z + readSaturnS32(offsetTable + (group - 3) * 12 + 8);
        BTL_A5_2_createDebrisParticle(pFile, &pos);
    }

    sSaturnPtr spawnTable = g_BTL_A5_2->getSaturnPtr(0x060b1820);
    s8 dirIdx = (s8)(((s16)pThis->m38 >> 8) & 0xFF);

    sVec3_FP pos;
    pos.m0_X = dragonPos.m0_X + readSaturnS32(spawnTable + dirIdx * 12);
    pos.m4_Y = readSaturnS32(spawnTable + dirIdx * 12 + 4);
    pos.m8_Z = dragonPos.m8_Z - 0x280000;
    BTL_A5_2_createDebrisParticle(pFile, &pos);

    pThis->m38 = 7;
}

// 06062d50
static void BTL_A5_2_debrisTask_Update(s_BTL_A3_Env* pThis)
{
    if (gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags & 0x40)
        return;

    s32 dragonZ = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;
    if (dragonZ < (s32)pThis->mC_cameraPosition.m8_Z - 0x100000)
    {
        pThis->mC_cameraPosition = gBattleManager->m10_battleOverlay->m18_dragon->m8_position;

        sSaturnPtr spawnTable = g_BTL_A5_2->getSaturnPtr(0x060b1820);
        npcFileDeleter* pFile = dramAllocatorEnd[6].mC_fileBundle;

        s16 cycle = pThis->m38 - 1;
        pThis->m38 = cycle;
        if (cycle < 0)
            pThis->m38 = 7;

        s8 dirIdx = (s8)((pThis->m38 >> 8) & 0xFF);
        sVec3_FP pos;
        pos.m0_X = (s32)pThis->mC_cameraPosition.m0_X + readSaturnS32(spawnTable + dirIdx * 12);
        pos.m4_Y = readSaturnS32(spawnTable + dirIdx * 12 + 4);
        pos.m8_Z = (s32)pThis->mC_cameraPosition.m8_Z - 0x280000;
        BTL_A5_2_createDebrisParticle(pFile, &pos);
    }
}

static void BTL_A5_2_debrisTask_Delete(s_BTL_A3_Env*)
{
    decreaseNPCRefCount(6);
}

// 060622e4
static void BTL_A5_2_env_Init(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(g_BTL_A5_2->getSaturnPtr(0x060b09d8), getVdp2Cram(0x000), 0x200, 0);
    asyncDmaCopy(g_BTL_A5_2->getSaturnPtr(0x060b0bd8), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_A5_2->getSaturnPtr(0x060b0dd8), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(g_BTL_A5_2->getSaturnPtr(0x060b0fd8), getVdp2Cram(0x440), 0x20, 0);

    static const sLayerConfig rgb0Setup[] = {
        m1_TPEN,  0,
        m2_CHCN,  1,
        m5_CHSZ,  1,
        m6_PNB,   1,
        m7_CNSM,  0,
        m27_RPMD, 2,
        m11_SCN,  8,
        m34_W0E,  1,
        m37_W0A,  1,
        m44_CCEN, 1,
        m43_BKEN, 1,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotationParams[] = {
        m28_RxKDE, 1,
        m31_RxKTE, 1,
        m0_END,
    };
    setupRotationParams(rotationParams);

    static const sLayerConfig rotationParams2[] = {
        m28_RxKDE, 1,
        m31_RxKTE, 1,
        m0_END,
    };
    setupRotationParams2(rotationParams2);

    loadFile("SCRBTLA5.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCRBTLA5.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A5_2->getSaturnPtr(0x060b1690));
    setupRotationMapPlanes(1, g_BTL_A5_2->getSaturnPtr(0x060b16d0));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

    *(u16*)getVdp2Vram(0x2A400) = 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    *(u16*)getVdp2Vram(0x2A600) = 0x0000;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;

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

    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 0x3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x0F;
    pThis->m51 = 0x0F;
    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;
    vdp2Controls.m_isDirty = 1;

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_A5_2->getSaturnPtr(0x060b1794));

    allocateNPC(pThis, 10);
    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;
    initGridForBattle(dramAllocatorEnd[10].mC_fileBundle, readGrid(g_BTL_A5_2->getSaturnPtr(0x060b1680), 2, 2), 2, 2, 0x200000);
    pThis->m58 = dramAllocatorEnd[10].mC_fileBundle;

    // debris particle spawner task
    {
        static const s_BTL_A3_Env::TypedTaskDefinition debrisDef = {
            (void(*)(s_BTL_A3_Env*))&BTL_A5_2_debrisTask_Init,
            (void(*)(s_BTL_A3_Env*))&BTL_A5_2_debrisTask_Update,
            nullptr,
            &BTL_A5_2_debrisTask_Delete,
        };
        createSubTask<s_BTL_A3_Env>(pThis, &debrisDef);
    }
}

// 060629fc
p_workArea Create_BTL_A5_2_env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A5_2_env_Init,
        &BTL_A5_2_env_Update,
        &BTL_A5_2_env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
