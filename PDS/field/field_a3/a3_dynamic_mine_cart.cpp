#include "PDS.h"
#include "a3_dynamic_mine_cart.h"

// This is confusing. This adjust the vertical position of the cart based on the dragon proximity.
// Visually, this just move the cart slightly up when colliding with the dragon. But is pretty much invisible most of the time since the dragon can't get close enough to the cart.
static void A3_0_Obj3Update2Sub0(sVec3_FP& r4, fixedPoint r5)
{
    sVec3_FP varC_dragonPosition;
    getFieldDragonPosition(&varC_dragonPosition);
    r4[1] = r5;

    sVec3_FP var0 = varC_dragonPosition - r4;
    fixedPoint r12 = FP_Div(var0[1], sqrt_F(MTH_Product3d_FP(var0, var0)));
    r4[1] += MTH_Mul_5_6(sqrt_F(var0[1].getAbs()) / 4, r12, r12);
}

static void A3_0_Obj3Update1Sub2(sVec3_FP& r12)
{
    sVec3_FP& r13 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

    s32 r9 = abs(r13[0] - getFieldTaskPtr()->mC->m158[0]) + abs(r13[2] - getFieldTaskPtr()->mC->m158[2]);
    s32 r3 = abs(r13[0] - r12[0]) + abs(r13[2] - r12[2]);

    if (r9 > r3)
    {
        getFieldTaskPtr()->mC->m158 = r12;
    }

    getFieldTaskPtr()->mC->m154 = 1;
}

// moving carts
struct s_A3_0_Obj3 : public s_workAreaTemplate<s_A3_0_Obj3>
{
    static void Draw(s_A3_0_Obj3* pThis)
    {
        if (pThis->m6E_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m10_position);
            rotateCurrentMatrixZYX_s16(&pThis->m2C_rotation[0]);
            LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemory, 0x10, 0x2E4);
            popMatrix();
        }
    }

    static void Draw2(s_A3_0_Obj3* pThis)
    {
        if (pThis->m6E_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m10_position);
            rotateCurrentMatrixZYX_s16(&pThis->m2C_rotation[0]);
            addObjectToDrawList(pThis->m0.m0_mainMemory, READ_BE_U32(pThis->m0.m0_mainMemory + 0x2C4));
            popMatrix();
        }
    }

    static void Update1Sub0(s_A3_0_Obj3* pThis)
    {
        pThis->m6E_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->m10_position);
        switch (pThis->m6C)
        {
        case 0: // init
            pThis->m28_visibilityDelay = 120;
            pThis->m6D_currentWaypoint = 0;
            pThis->m10_position[0] = readSaturnS32(pThis->mC + 0);
            pThis->m10_position[2] = readSaturnS32(pThis->mC + 4);
            pThis->m6C++;
            //fall
        case 1: // invisible
            if (pThis->m28_visibilityDelay > 0)
            {
                pThis->m6E_visible = 0;
                pThis->m28_visibilityDelay--;
                break;
            }
            pThis->m6C++;
            break;
        case 2: // new waypoint
            pThis->m6D_currentWaypoint++;
            if (readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8) == 0)
            {
                pThis->m6C = 0;
                break;
            }
            else
            {
                sSaturnPtr var0 = pThis->mC + pThis->m6D_currentWaypoint * 8;
                pThis->m32_targetAngle = atan2(readSaturnS32(var0 + 0) - pThis->m10_position[0], readSaturnS32(var0 + 4) - pThis->m10_position[2]);
                pThis->m1C = getSin(pThis->m32_targetAngle & 0xFFF) >> 5;
                pThis->m24 = getCos(pThis->m32_targetAngle & 0xFFF) >> 5;
                pThis->m32_targetAngle = 0x800 - pThis->m32_targetAngle;
                if (pThis->m6D_currentWaypoint > 1)
                {
                    //0605E38C
                    if (readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8 + 8) == 0)
                    {
                        createSmokePufTask(pThis, &pThis->m10_position, pThis->m1C);
                    }
                    if (pThis->m32_targetAngle >= pThis->m2C_rotation[1])
                    {
                        pThis->m34_deltaAngle = 5;
                    }
                    else
                    {
                        pThis->m34_deltaAngle = -5;
                    }
                }
                else
                {
                    //605E3C4
                    createSmokePufTask(pThis, &pThis->m10_position, pThis->m1C);
                    pThis->m2C_rotation[1] = pThis->m32_targetAngle;
                    pThis->m34_deltaAngle = 0;
                }
                //605E3DE
                pThis->m6C++;
                break;
            }
        case 3: // move to next point
            pThis->m2C_rotation[1] += pThis->m34_deltaAngle;
            if (std::abs(pThis->m2C_rotation[1] - pThis->m32_targetAngle) < 5)
            {
                pThis->m34_deltaAngle = 0;
            }
            pThis->m10_position[0] += pThis->m1C;
            pThis->m10_position[2] += pThis->m24;
            //Are we there yet?
            if (((pThis->m1C >= 0) && (pThis->m10_position[0] < readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8)))
                || ((pThis->m1C < 0) && (pThis->m10_position[0] >= readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8))))
                break;
            if (((pThis->m24 >= 0) && (pThis->m10_position[0] < readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8 + 4)))
                || ((pThis->m24 < 0) && (pThis->m10_position[2] >= readSaturnS32(pThis->mC + pThis->m6D_currentWaypoint * 8 + 4))))
                break;
            pThis->m6C = 2;
            break;
        default:
            assert(0);
        }

        A3_0_Obj3Update2Sub0(pThis->m10_position, pThis->m8->m4_position[1]);
        if (pThis->m6E_visible)
        {
            A3_0_Obj3Update1Sub2(pThis->m10_position);
        }
    }

    // spawn other mine carts?
    static void Update1Sub1(s_A3_0_Obj3* pThis, s_DataTable2Sub0* r5)
    {
        static const s_A3_0_Obj3::TypedTaskDefinition definition = { NULL, &s_A3_0_Obj3::Update1Sub0, &s_A3_0_Obj3::Draw2, NULL };
        s_A3_0_Obj3* pNewTask = createSubTaskZeroWorkArea<s_A3_0_Obj3>(pThis, &definition);
        memcpy(&pNewTask->m0, &pThis->m0, sizeof(s_memoryAreaOutput));

        pNewTask->m8 = r5;
        pNewTask->m10_position = r5->m4_position;
        pNewTask->m2C_rotation = r5->m10_rotation;
        pNewTask->m1C = 0;
        pNewTask->m20 = 0;
        pNewTask->m24 = 0;

        s8 r6 = readSaturnS8(sSaturnPtr({ 0x60925F7, gFLD_A3 }) + r5->m18);
        pNewTask->mC = readSaturnEA(sSaturnPtr({ 0x609270C, gFLD_A3 }) + (r6-1) * 4);

        pNewTask->m6C = 0;
    }

    static void Update1(s_A3_0_Obj3* pThis)
    {
        Update1Sub0(pThis);
        if (pThis->m36 == 0x2A)
        {
            Update1Sub1(pThis, pThis->m8);
        }
        else if (pThis->m36 == 0x54)
        {
            Update1Sub1(pThis, pThis->m8);
            pThis->m_UpdateMethod = &s_A3_0_Obj3::Update1Sub0;
        }
        pThis->m36++;
    }

    static void Update2(s_A3_0_Obj3* pThis)
    {
        updateLCSTarget(&pThis->m38_LCSTarget);
        switch (pThis->m6C)
        {
        case 0:
            break;
        default:
            assert(0);
            break;
        }

        A3_0_Obj3Update2Sub0(pThis->m10_position, pThis->m8->m4_position[1]);
        pThis->m6E_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->m10_position);
    }

    s_memoryAreaOutput m0;
    s_DataTable2Sub0* m8;
    sSaturnPtr mC;
    sVec3_FP m10_position;
    s32 m1C;
    s32 m20;
    s32 m24;
    s32 m28_visibilityDelay;
    std::array<s16, 3> m2C_rotation;
    s16 m32_targetAngle;
    s16 m34_deltaAngle;
    s16 m36;
    sLCSTarget m38_LCSTarget;
    s8 m6C;
    s8 m6D_currentWaypoint;
    s8 m6E_visible;
    // size 0x70
};

void A3_0_Obj3_LCSCallback(p_workArea task, sLCSTarget*)
{
    s_A3_0_Obj3* pThis = (s_A3_0_Obj3*)task;

    mainGameState.setBit(0x93, 4);
    pThis->m38_LCSTarget.m18 |= 1;
    pThis->m6C = 1;
}

void create_A3_0_Obj3(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    s_A3_0_Obj3* pNewTask = createSubTaskFromFunction<s_A3_0_Obj3>(r4, NULL);
    getMemoryArea(&pNewTask->m0, r6);

    pNewTask->m8 = &r5;
    pNewTask->m_DrawMethod = &s_A3_0_Obj3::Draw;
    pNewTask->m10_position = r5.m4_position;
    pNewTask->m2C_rotation = r5.m10_rotation;
    pNewTask->m1C = 0;
    pNewTask->m20 = 0;

    s8 r12 = readSaturnS8(sSaturnPtr({ 0x60925F7, gFLD_A3 }) + r5.m18);
    switch (r12)
    {
    case 1:
    case 2:
        pNewTask->m_UpdateMethod = &s_A3_0_Obj3::Update1;
        pNewTask->mC = readSaturnEA(sSaturnPtr({ 0x609270C, gFLD_A3 }) + (r12 - 1) * 4);
        break;
    case 3:
        if (mainGameState.getBit(0x93, 4))
        {
            pNewTask->getTask()->markFinished();
        }
        else
        {
            pNewTask->m_UpdateMethod = &s_A3_0_Obj3::Update2;
            pNewTask->m1C = readSaturnS32(sSaturnPtr({ 0x6092714, gFLD_A3 }) + (r12 - 3) * 8);
            pNewTask->m24 = readSaturnS32(sSaturnPtr({ 0x6092714, gFLD_A3 }) + (r12 - 3) * 8 + 4);
            pNewTask->mC = sSaturnPtr({ 0x609271C, gFLD_A3 }) + (r12 - 3) * 4;
            createLCSTarget(&pNewTask->m38_LCSTarget, pNewTask, &A3_0_Obj3_LCSCallback, &pNewTask->m10_position, 0, 0, 0, -1, 0, 0);
        }
        break;
    default:
        break;
    }

    pNewTask->m6C = 0;
}
