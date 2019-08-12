#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "a3_2_crashedImperialShip.h"

void fieldA3_2_createCrashedImpertialShipExplosions(p_workArea)
{
    FunctionUnimplemented();
}

void fieldA3_2_crashedImpertialShip_UpdateSub0(p_workArea, sVec3_FP*)
{
    FunctionUnimplemented();
}

void fieldA3_2_crashedImpertialShip_customScriptCall(fieldA3_2_crashedImpertialShip* r4)
{
    FunctionUnimplemented();
}

struct fieldA3_2_crashedImpertialShip : public s_workAreaTemplate<fieldA3_2_crashedImpertialShip>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &fieldA3_2_crashedImpertialShip::Update, &fieldA3_2_crashedImpertialShip::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(fieldA3_2_crashedImpertialShip* pThis)
    {
        stepAnimation(&pThis->m40_3dModel);
        switch (pThis->m3C_status)
        {
        case 0:
            if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 2)
            {
                assert(0);
            }
            break;
        case 1: // recording activated
            pThis->m90_LCSTarget.m18_diableFlags |= 1;
            if (mainGameState.getBit(0xA, 7))
            {
                startFieldScript(16, 1452);
            }
            else
            {
                startFieldScript(15, 1451);
            }
            fieldA3_2_createCrashedImpertialShipExplosions(pThis);
            fieldA3_2_crashedImpertialShip_UpdateSub0(pThis, &pThis->mC_position);
            pThis->m3C_status++;
            //fall
        case 2: // wait for end of script
            if (!fieldScriptTaskUpdateSub4())
            {
                pThis->m3C_status++;
            }
            break;
        case 3: // done
            break;
        default:
            assert(0);
            break;
        }

        updateLCSTarget(&pThis->m90_LCSTarget);
        pThis->m3D_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
    }

    static void Draw(fieldA3_2_crashedImpertialShip* pThis)
    {
        if (pThis->m3D_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pThis->mC_position);
            rotateCurrentMatrixZYX_s16(pThis->m24_rotation);
            pThis->m40_3dModel.m18_drawFunction(&pThis->m40_3dModel);
            callGridCellDraw_normalSub2(pThis, 1164);
            popMatrix();
        }
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_S16_12_4 m24_rotation;
    sVec3_FP m2C_LCSTargetLocation;
    s8 m3C_status;
    s8 m3D_visible;
    s_3dModel m40_3dModel;
    sLCSTarget m90_LCSTarget;
    // size 0xC4
};

void fieldA3_2_crashedImpertialShip_LCSCallback(p_workArea r4, sLCSTarget*)
{
    fieldA3_2_crashedImpertialShip* pThis = static_cast<fieldA3_2_crashedImpertialShip*>(r4);
    pThis->m3C_status = true;
}

void create_A3_2_crashedImperialShip(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    if (mainGameState.getBit(0x6E, 5))
        return;

    if (mainGameState.getBit(0x6E, 6))
        return;

    fieldA3_2_crashedImpertialShip* pNewTask = createSubTask<fieldA3_2_crashedImpertialShip>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);

    pNewTask->m8 = &r5;
    pNewTask->mC_position = r5.m4_position;
    pNewTask->m24_rotation = r5.m10_rotation;
    pNewTask->m2C_LCSTargetLocation = pNewTask->mC_position + sVec3_FP(0, -0x3C000, 0xA000);

    s_fileBundle* pBundle = pNewTask->m0_memoryArea.m0_mainMemoryBundle;
    sAnimationData* pAnimation = pBundle->getAnimation(0x580);
    u8* pPose = pBundle->getRawFileAtOffset(readSaturnU16(r5.m1C_modelData + 2));

    init3DModelRawData(pNewTask, &pNewTask->m40_3dModel, 0, pBundle, readSaturnS16(r5.m1C_modelData), pAnimation, pPose, nullptr, nullptr);
    stepAnimation(&pNewTask->m40_3dModel);

    createLCSTarget(&pNewTask->m90_LCSTarget, pNewTask, &fieldA3_2_crashedImpertialShip_LCSCallback, &pNewTask->m2C_LCSTargetLocation, nullptr, 0, 0, 140, 1, 0);
    pNewTask->m3C_status = 0;

    getFieldTaskPtr()->mC->m164_A3_2_crashedImperialShipTask = pNewTask;
}

struct fieldA3_2_crashedImpertialShip2 : public s_workAreaTemplate<fieldA3_2_crashedImpertialShip2>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &fieldA3_2_crashedImpertialShip2::Update, &fieldA3_2_crashedImpertialShip2::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(fieldA3_2_crashedImpertialShip2* pThis)
    {
        stepAnimation(&pThis->m34_3dModel);
        pThis->m31_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
    }

    static void Draw(fieldA3_2_crashedImpertialShip2* pThis)
    {
        if (pThis->m31_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pThis->mC_position);
            rotateCurrentMatrixZYX_s16(pThis->m24_rotation);
            pThis->m34_3dModel.m18_drawFunction(&pThis->m34_3dModel);
            callGridCellDraw_normalSub2(pThis, 1136);
            popMatrix();
        }
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_S16_12_4 m24_rotation;
    s8 m30_status;
    s8 m31_visible;
    s_3dModel m34_3dModel;
    // size 0x84
};

void create_A3_2_crashedImperialShip2Sub0(p_workArea)
{
    FunctionUnimplemented();
}

void create_A3_2_crashedImperialShip2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    fieldA3_2_crashedImpertialShip2* pNewTask = createSubTask<fieldA3_2_crashedImpertialShip2>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);
    pNewTask->m8 = &r5;
    pNewTask->mC_position = r5.m4_position;
    pNewTask->m24_rotation = r5.m10_rotation;

    s_fileBundle* pBundle = pNewTask->m0_memoryArea.m0_mainMemoryBundle;
    sAnimationData* pAnimation = pBundle->getAnimation(0x57C);
    u8* pPose = pBundle->getRawFileAtOffset(readSaturnU16(r5.m1C_modelData + 2));

    init3DModelRawData(pNewTask, &pNewTask->m34_3dModel, 0, pBundle, readSaturnS16(r5.m1C_modelData), pAnimation, pPose, nullptr, nullptr);
    stepAnimation(&pNewTask->m34_3dModel);

    pNewTask->m30_status = 0;

    create_A3_2_crashedImperialShip2Sub0(pNewTask);
}

s32 fieldA3_2_crashedImpertialShip_customScriptCall()
{
    getFieldTaskPtr()->mC->m164_A3_2_crashedImperialShipTask->m_DrawMethod = nullptr;
    fieldA3_2_crashedImpertialShip_customScriptCall(getFieldTaskPtr()->mC->m164_A3_2_crashedImperialShipTask);
    cutsceneTaskInitSub2(getFieldTaskPtr()->mC->m164_A3_2_crashedImperialShipTask, gFLD_A3->m6083244, 0, 0, 0);

    return 0;
}



