#include "PDS.h"
#include "o_fld_a5.h"

// Detects the dragon wandering through 3 triangular zones around a
// specific landmark in subfields 0/7 and sets mainGameState bit 0xA2.8
// when all three have been visited within a ~150-frame window.
struct sA5FieldEventCheckTask : public s_workAreaTemplate<sA5FieldEventCheckTask>
{
    s16 m0_timer;
    s16 m2_flags;
    // Saturn size 0x4
};

// 060656F8 — empty init
static void a5FieldEventCheckInit(sA5FieldEventCheckTask* /*pThis*/)
{
    // empty — confirmed from Ghidra
}

// 060656FC — return 1 iff the dragon's (oldPos+pos) pair forms a
// barycentric-valid triangle with the two vertices stored at `pData`.
// Each vertex is a sVec3_FP; only X and Z components participate.
static s32 a5CheckProximityToPoint(sSaturnPtr pData)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    s32 v0x = readSaturnS32(pData);
    s32 v0z = readSaturnS32(pData + 8);
    s32 v1x = readSaturnS32(pData + 12);
    s32 v1z = readSaturnS32(pData + 20);

    s32 refX = pDragon->m14_oldPos.m0_X.asS32();
    s32 refZ = pDragon->m14_oldPos.m8_Z.asS32();

    s32 e0x = v0x - refX;
    s32 e0z = v0z - refZ;
    s32 e1x = v1x - refX;
    s32 e1z = v1z - refZ;

    s32 px = pDragon->m8_pos.m0_X.asS32() - refX;
    s32 pz = pDragon->m8_pos.m8_Z.asS32() - refZ;

    s32 cross = MTH_Mul(fixedPoint(e0x), fixedPoint(e1z)).asS32()
              - MTH_Mul(fixedPoint(e0z), fixedPoint(e1x)).asS32();

    s32 u = FP_Div(
        MTH_Mul(fixedPoint(e1z), fixedPoint(px)).asS32() - MTH_Mul(fixedPoint(e1x), fixedPoint(pz)).asS32(),
        fixedPoint(cross)).asS32();

    s32 v = FP_Div(
        MTH_Mul(fixedPoint(e0x), fixedPoint(pz)).asS32() - MTH_Mul(fixedPoint(e0z), fixedPoint(px)).asS32(),
        fixedPoint(cross)).asS32();

    if (u < 0 || v < 0 || (u + v) < 0x10000)
    {
        return 0;
    }
    return 1;
}

// 0606580C — field event check: detects dragon in 3 landmark zones,
// latches each into m2_flags, and sets mainGameState bit 0xA2.8 when
// all 3 latches are set within the 0x96 countdown window.
static void a5FieldEventCheckUpdate(sA5FieldEventCheckTask* pThis)
{
    s16 prevFlags = pThis->m2_flags;

    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (dragonPos.m4_Y.asS32() < 0x2E000
        && dragonPos.m0_X.asS32() > 0x100000 && dragonPos.m0_X.asS32() < 0x300000
        && dragonPos.m8_Z.asS32() > -0x400000 && dragonPos.m8_Z.asS32() < -0x200000)
    {
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D548)) != 0)
        {
            pThis->m2_flags |= 1;
        }
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D560)) != 0)
        {
            pThis->m2_flags |= 2;
        }
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D578)) != 0)
        {
            pThis->m2_flags |= 4;
        }
    }

    if (prevFlags == 0 && pThis->m2_flags != 0)
    {
        pThis->m0_timer = 0x96;
    }

    pThis->m0_timer--;
    if (pThis->m0_timer < 0)
    {
        pThis->m0_timer = 0;
        pThis->m2_flags = 0;
    }

    if (pThis->m2_flags == 7 && pThis->m0_timer != 0)
    {
        mainGameState.bitField[0xA2] |= 8;
    }
}

// 060658c4 — create field event check task (subfields 0, 7)
void createA5_fieldEventCheck(p_workArea parent)
{
    static sA5FieldEventCheckTask::TypedTaskDefinition td = { &a5FieldEventCheckInit, &a5FieldEventCheckUpdate, nullptr, nullptr };
    createSubTask<sA5FieldEventCheckTask>(parent, &td);
}
