#include "PDS.h"
#include "o_fld_a3_0.h"
#include "o_fld_a3_4.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"

struct fieldA3_8_exitTask : public s_workAreaTemplate<fieldA3_8_exitTask>
{
    static void Update(fieldA3_8_exitTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] > -0x48000)
        {
            mainGameState.setBit566(1269);
            subfieldA3_1Sub0Sub2(0, -1);
            pThis->m_UpdateMethod = NULL;
        }
    }

    s32 m8;
    //size C
};

void fieldA3_8_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_8_exitTask>(workArea, &fieldA3_8_exitTask::Update);
}

// kernel
void update3dModelDrawFunctionForVertexAnimation(s_3dModel* r4, u8* pData)
{
    FunctionUnimplemented();
}

// kernel
s32 func3dModelSub0(s_3dModel* r4)
{
    if (r4->m38 == nullptr)
        return 0;

    r4->m14++;
    if (r4->m14 >= READ_BE_S16(r4->m38))
    {
        r4->m14 = 0;
    }

    return r4->m14;
}

struct fieldA3_8_generatorTask : public s_workAreaTemplate<fieldA3_8_generatorTask>
{
    static void Update(fieldA3_8_generatorTask* pThis)
    {
        switch (pThis->m9C_status)
        {
            // init running
        case 0:
            update3dModelDrawFunctionForVertexAnimation(&pThis->m18_3dModel, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x300));
            mainGameState.clearBit(0xA2, 3);
            pThis->m9C_status++;
            //fall
        // running
        case 1:
            if (!isSoundPlaying(104))
            {
                playSoundEffect(104);
            }
            if (mainGameState.getBit(0x6D, 2))
            {
                pThis->m68_LCSTarget.m18_diableFlags = 0;
            }
            else
            {
                pThis->m68_LCSTarget.m18_diableFlags |= 1;
            }

            updateLCSTarget(&pThis->m68_LCSTarget);
            stepAnimation(&pThis->m18_3dModel);
            func3dModelSub0(&pThis->m18_3dModel);
            break;
        case 2: // init shutdown
            pThis->m68_LCSTarget.m18_diableFlags |= 1;
            update3dModelDrawFunctionForVertexAnimation(&pThis->m18_3dModel, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x304));
            pThis->m9C_status++;
            //fall
        case 3: //shutting down
            updateLCSTarget(&pThis->m68_LCSTarget);
            stepAnimation(&pThis->m18_3dModel);
            func3dModelSub0(&pThis->m18_3dModel);
            if (pThis->m18_3dModel.m38)
            {
                if (pThis->m18_3dModel.m14 == READ_BE_S16(pThis->m18_3dModel.m38) - 1)
                {
                    pThis->m9C_status++;
                }
            }
            else
            {
                if (pThis->m18_3dModel.m14 == -1)
                {
                    pThis->m9C_status++;
                }
            }
            break;
        case 4: // finish shutdown
            soundFunc(104);
            update3dModelDrawFunctionForVertexAnimation(&pThis->m18_3dModel, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x304));
            if (pThis->m18_3dModel.m38)
            {
                pThis->m18_3dModel.m14 = READ_BE_S16(pThis->m18_3dModel.m38) - 1;
            }
            else
            {
                pThis->m18_3dModel.m14 = -1;
            }
            mainGameState.setBit(0xA2, 3);
            pThis->m68_LCSTarget.m18_diableFlags = 0;
            pThis->m9C_status++;
            // fall
        case 5: // Off
            updateLCSTarget(&pThis->m68_LCSTarget);
            break;
        case 6: // init startup
            pThis->m68_LCSTarget.m18_diableFlags |= 1;
            update3dModelDrawFunctionForVertexAnimation(&pThis->m18_3dModel, pThis->m0_memoryArea.m0_mainMemory + READ_BE_U32(pThis->m0_memoryArea.m0_mainMemory + 0x308));
            pThis->m9C_status++;
            //fall
        case 7: // starting up
            updateLCSTarget(&pThis->m68_LCSTarget);
            stepAnimation(&pThis->m18_3dModel);
            func3dModelSub0(&pThis->m18_3dModel);
            if (pThis->m18_3dModel.m38)
            {
                if (pThis->m18_3dModel.m14 == READ_BE_S16(pThis->m18_3dModel.m38) - 1)
                {
                    pThis->m9C_status = 0;
                }
            }
            else
            {
                if (pThis->m18_3dModel.m14 == -1)
                {
                    pThis->m9C_status = 0;
                }
            }
            break;
        default:
            assert(0);
        }

    }

    static void Draw(fieldA3_8_generatorTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->m8_position);
        rotateCurrentMatrixShiftedY(pThis->m14_rotation);
        pThis->m18_3dModel.m18_drawFunction(&pThis->m18_3dModel);
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    s32 m14_rotation;
    s_3dModel m18_3dModel;
    sLCSTarget m68_LCSTarget;
    s8 m9C_status;
    // size: 0xA0
};

void fieldA3_8_create_generatorLCSTarget(p_workArea r4, sLCSTarget* r5)
{
    fieldA3_8_generatorTask* pThis = static_cast<fieldA3_8_generatorTask*>(r4);

    if (pThis->m9C_status == 1)
    {
        if (mainGameState.getBit(0x6D, 3))
        {
            startFieldScriptSkippable(0xC, 1449);
        }
        else
        {
            startFieldScript(4, 1441);
        }
        pThis->m9C_status = 2;
    }
    else
    {
        if (mainGameState.getBit(0x6D, 5))
        {
            startFieldScriptSkippable(0xD, 1450);
        }
        else
        {
            startFieldScript(5, 1441);
        }
        pThis->m9C_status = 6;
    }
}

struct fieldA3_8_generatorCameraTask : public s_workAreaTemplate<fieldA3_8_generatorCameraTask>
{
    static void Update(fieldA3_8_generatorCameraTask* pThis)
    {
        if (mainGameState.getBit(0x6D, 2))
        {
            pThis->getTask()->markFinished();
        }
        else
        {
            pThis->m_UpdateMethod = &Update2;
        }
    }

    static void Update2(fieldA3_8_generatorCameraTask* pThis)
    {
        if (vecDistance(getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos, readSaturnVec3(gFLD_A3->getSaturnPtr(0x608EEEC))) < 0x118000)
        {
            static std::vector<s_scriptData1> data1;
            loadScriptData1(data1, gFLD_A3->getSaturnPtr(0x60831CC));
            sVec3_FP position = readSaturnVec3(gFLD_A3->getSaturnPtr(0x608EEEC));
            cutsceneTaskInitSub2(pThis, data1, 0, &position, 0);
            startFieldScript(3, 1440);
            pThis->getTask()->markFinished();
        }
    }

    //size 4
};

void fieldA3_8_create_generatorTask(p_workArea workArea)
{
    static const fieldA3_8_generatorTask::TypedTaskDefinition definition = { NULL, &fieldA3_8_generatorTask::Update, &fieldA3_8_generatorTask::Draw, NULL };
    fieldA3_8_generatorTask* pNewTask = createSubTask<fieldA3_8_generatorTask>(workArea, &definition);

    getMemoryArea(&pNewTask->m0_memoryArea, 2);
    pNewTask->m8_position = readSaturnVec3(gFLD_A3->getSaturnPtr(0x608EEEC));
    pNewTask->m14_rotation = -0x4000000;

    s_fileBundle* pBundle = pNewTask->m0_memoryArea.m0_mainMemoryBundle;
    u8* pAnimation = pBundle->getRawFileAtOffset(0x2FC);
    u8* pPose = pBundle->getRawFileAtOffset(0x300);

    init3DModelRawData(pNewTask, &pNewTask->m18_3dModel, 0, pBundle, 40, NULL, pAnimation, pPose, nullptr);
    stepAnimation(&pNewTask->m18_3dModel);
    func3dModelSub0(&pNewTask->m18_3dModel);

    if (mainGameState.getBit(0xA2, 3))
    {
        pNewTask->m9C_status = 4;
    }
    else
    {
        pNewTask->m9C_status = 0;
    }

    createLCSTarget(&pNewTask->m68_LCSTarget, pNewTask, &fieldA3_8_create_generatorLCSTarget, &pNewTask->m8_position, nullptr, 0, 0, -1, 0, 0);
    createSubTaskFromFunction<fieldA3_8_generatorCameraTask>(pNewTask, &fieldA3_8_generatorCameraTask::Update);
}

//TODO: Really not sure what the point of this is. Maybe check the locations on the map to see what it maps to?
struct fieldA3_8_task2 : public s_workAreaTemplate<fieldA3_8_task2>
{
    static void Update(fieldA3_8_task2* pThis)
    {
        if (mainGameState.getBit(0xA2, 3))
        {
            pThis->m0 = 0x72000;
            pThis->m4 = 0;
            pThis->m8 = -0x2A000;
        }
        else
        {
            pThis->m0 = 0x25000;
            pThis->m4 = 0;
            pThis->m8 = -0x20A000;
        }
    }

    s32 m0;
    s32 m4;
    s32 m8;
    //size C
};

void fieldA3_8_create_task2(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_8_task2>(workArea, &fieldA3_8_task2::Update);
}

void fieldA3_8_startTasks(p_workArea workArea)
{
    fieldA3_8_create_generatorTask(workArea);
    fieldA3_8_createExitTask(workArea);
    fieldA3_8_create_task2(workArea);
}

void subfieldA3_8(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    loadFileFromFileList(2);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EE38, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_8_startTasks);

    sVec3_FP position = readSaturnVec3({ 0x6081B6C, gFLD_A3 });
    sVec3_FP rotation = readSaturnVec3({ 0x6081BD8, gFLD_A3 });
    setupDragonPosition(&position, &rotation);

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    subfieldA3_4_sub1(workArea);
}
