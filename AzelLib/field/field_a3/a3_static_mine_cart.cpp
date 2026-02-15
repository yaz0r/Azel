#include "PDS.h"
#include "a3_static_mine_cart.h"
#include "a3_dynamic_mine_cart.h"
#include "kernel/fileBundle.h"
#include "items.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

struct s_A3_3_Obj0 : public s_workAreaTemplate<s_A3_3_Obj0>
{
    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_FP m18;
    sVec3_FP m24;
    sVec3_S16_12_4 m30_rotation;
    s16 m36_delay;
    s16 m38_currentDelay;
    s8 m3A;
    sLCSTarget m3C;
    // size 0x70
};

void A3_3_Obj0_Update1Sub0(sVec3_FP* r4, sVec3_FP* r5)
{
    sVec3_FP var8_dragonPosition;
    getFieldDragonPosition(&var8_dragonPosition);
    var8_dragonPosition -= *r5;

    fixedPoint r12 = FP_Div(var8_dragonPosition[1], sqrt_F(MTH_Product3d_FP(var8_dragonPosition, var8_dragonPosition)));
    if (r12 < 0)
    {
        r12 = MTH_Mul_5_6(sqrt_F(var8_dragonPosition[1].getAbs()), r12, r12);

        sVec2_FP var0;
        computeVectorAngles(var8_dragonPosition, var0);

        (*r4)[0] = (*r5)[0] + MTH_Mul(r12, getSin(var0[1].getInteger() & 0xFFF));
        (*r4)[1] = (*r5)[1] + MTH_Mul(r12, -getSin(var0[0].getInteger() & 0xFFF));
        (*r4)[2] = (*r5)[2] + MTH_Mul(r12, getCos(var0[1].getInteger() & 0xFFF));
    }
    else
    {
        *r4 = *r5;
    }
}

void A3_3_Obj0_Draw(s_A3_3_Obj0* pThis)
{
    if (!checkPositionVisibilityAgainstFarPlane(&pThis->mC_position))
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->mC_position);
        rotateCurrentMatrixZYX_s16(pThis->m30_rotation);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x2C4));
        callGridCellDraw_normalSub2(pThis, 0x2C8);
        popMatrix();
    }
}

void A3_3_Obj0_Draw2(s_A3_3_Obj0* pThis)
{
    if (!checkPositionVisibilityAgainstFarPlane(&pThis->mC_position))
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->mC_position);
        rotateCurrentMatrixZYX_s16(pThis->m30_rotation);
        addObjectToDrawList(pThis->m0.m0_mainMemoryBundle->get3DModel(0x2C4));
        popMatrix();
    }
}

void A3_3_Obj0_Update1Sub1(s_A3_3_Obj0* pThis, sVec3_FP* pVec)
{
    Unimplemented();
}

void A3_3_Obj0_Update1(s_A3_3_Obj0* pThis)
{
    updateLCSTarget(&pThis->m3C);
    switch (pThis->m3A)
    {
    case 0:
        A3_3_Obj0_Update1Sub0(&pThis->mC_position, &pThis->m8->m4_position);
        break;
    case 1:
        playSystemSoundEffect(0x17);
        A3_3_Obj0_Update1Sub1(pThis, &pThis->mC_position);
        pThis->m24[1] = -327;
        pThis->m18[1] = 0x1800;
        pThis->m3A++;
        //fall through
    case 2:
        pThis->m18 += pThis->m24;
        pThis->mC_position += pThis->m18;
        pThis->m30_rotation[1] += 5;
        pThis->m30_rotation[2] -= 28;
        if (pThis->mC_position[1] <= -0x640000)
        {
            pThis->m_UpdateMethod = NULL;
            pThis->m_DrawMethod = NULL;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void A3_3_Obj0_Update2(s_A3_3_Obj0* pThis)
{
    A3_3_Obj0_Update1(pThis);
    if (pThis->m3A > 1)
    {
        pThis->m38_currentDelay--;
        if (pThis->m38_currentDelay <= 0)
        {
            playSystemSoundEffect(0x17);
            pThis->m18[0] = -pThis->m18[0];
            pThis->m18[2] = -pThis->m18[2];

            pThis->m_UpdateMethod = A3_3_Obj0_Update1;
        }
    }
}

void A3_3_Obj0_Update0(s_A3_3_Obj0* pThis)
{
    updateLCSTarget(&pThis->m3C);
    switch (pThis->m3A)
    {
    case 0: // standing still
        break;
    case 1: // falling
        pThis->m18.m_value[0] += pThis->m24.m_value[0];
        pThis->m18.m_value[2] += pThis->m24.m_value[2];

        pThis->mC_position.m_value[0] += pThis->m18.m_value[0];
        pThis->mC_position.m_value[2] += pThis->m18.m_value[2];

        if (pThis->m38_currentDelay >= pThis->m36_delay)
        {
            playSystemSoundEffect(0x17);
            pThis->m18[0] = -pThis->m18[0];
            pThis->m18[2] = pThis->m18[2] / 2;

            pThis->m24.m_value[1] = -327;
            pThis->m18[1] = 0x1800;

            pThis->m38_currentDelay = 0;
            pThis->m3A++;
            pThis->m_UpdateMethod = A3_3_Obj0_Update1;
        }
        else
        {
            //605DF88
            if (pThis->m38_currentDelay == 0)
            {
                A3_3_Obj0_Update1Sub1(pThis, &pThis->mC_position);
            }

            A3_0_Obj3Update1Sub2(pThis->mC_position);

            pThis->m38_currentDelay++;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void A3_0_Obj0_Callback(p_workArea r4, sLCSTarget* r5)
{
    s_A3_3_Obj0* pThis = static_cast<s_A3_3_Obj0*>(r4);

    pThis->m_DrawMethod = A3_3_Obj0_Draw2;
    pThis->m3C.m18_diableFlags |= 1;

    static const std::array<int, 15> cartFlags =
    {
        1731,
        1732,
        1733,
        1734,
        1735,
        1736,
        1737,
        1738,
        1739,
        1740,
        1741,
        1742,
        1743,
        1744,
        1745,
    };

    mainGameState.setBit566(cartFlags[pThis->m8->m18]);

    startFieldScript(0, 1437);

    pThis->m3A = 1;
}

void create_A3_0_Obj0(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s8 r12 = readSaturnS8(sSaturnPtr({ 0x60925E8, gFLD_A3 }) + r5.m18);

    s_A3_3_Obj0* pNewObj = createSubTaskFromFunction<s_A3_3_Obj0>(r4, NULL);
    getMemoryArea(&pNewObj->m0, r6);
    pNewObj->m8 = &r5;

    s32 variable = readSaturnS32(sSaturnPtr({ 0x60925AC, gFLD_A3 }) + 4 * r5.m18);
    if (mainGameState.getBit566(variable))
    {
        pNewObj->getTask()->markFinished();
        return;
    }

    pNewObj->m_DrawMethod = &A3_3_Obj0_Draw;
    pNewObj->mC_position = r5.m4_position;
    pNewObj->m30_rotation[0] = r5.m10_rotation[0];
    pNewObj->m30_rotation[1] = r5.m10_rotation[1];
    pNewObj->m30_rotation[2] = r5.m10_rotation[2];
    pNewObj->m18.zeroize();
    pNewObj->m24.zeroize();
    pNewObj->m3A = 0;

    eItems param = (eItems)readSaturnS32(sSaturnPtr({ 0x6092570, gFLD_A3 }) + 4 * r5.m18);
    if (param == eItems::mMinusOne)
    {
        createLCSTarget(&pNewObj->m3C, pNewObj, &A3_0_Obj0_Callback, &pNewObj->mC_position, 0, 0, 0, eItems::mMinusOne, 0, 0);
    }
    else
    {
        createLCSTarget(&pNewObj->m3C, pNewObj, &A3_0_Obj0_Callback, &pNewObj->mC_position, 0, 0, 0, param, 1, 0);
    }

    //0605E162
    switch (r12)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update0;
        pNewObj->m24[0] = readSaturnS32(sSaturnPtr({ 0x609262C, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m24[2] = readSaturnS32(sSaturnPtr({ 0x609262C, gFLD_A3 }) + 8 * r12 + 4);
        pNewObj->m36_delay = readSaturnS16(sSaturnPtr({ 0x609264C, gFLD_A3 }) + 2 * r12);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        r12 -= 4;
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update1;
        pNewObj->m18[0] = readSaturnS32(sSaturnPtr({ 0x60925FC, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m18[2] = readSaturnS32(sSaturnPtr({ 0x60925FC, gFLD_A3 }) + 8 * r12 + 4);
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        r12 -= 10;
        pNewObj->m_UpdateMethod = &A3_3_Obj0_Update2;
        pNewObj->m18[0] = readSaturnS32(sSaturnPtr({ 0x6092654, gFLD_A3 }) + 8 * r12 + 0);
        pNewObj->m18[2] = readSaturnS32(sSaturnPtr({ 0x6092654, gFLD_A3 }) + 8 * r12 + 4);
        pNewObj->m36_delay = pNewObj->m38_currentDelay = readSaturnS16(sSaturnPtr({ 0x6092674, gFLD_A3 }) + 2 * r12);
        break;
    default:
        assert(0);
        break;
    }
}
