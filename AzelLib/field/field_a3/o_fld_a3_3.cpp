#include "PDS.h"
#include "items.h"
#include "audio/soundDriver.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

// Conana's nest

void fieldA3_3_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092274, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60922BC, gFLD_A3 }));
}

void fieldA3_3_exitTaskCallback(p_workArea, sLCSTarget*)
{
    getFieldTaskPtr()->mC->mA3_conanaNestExit = 1;
}

struct fieldA3_3_exitTask : public s_workAreaTemplate<fieldA3_3_exitTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_3_exitTask::Init, &fieldA3_3_exitTask::Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_3_exitTask* pThis)
    {
        static const sVec3_FP position = { 0xC0000, -0x10000, -0x66000 };

        createLCSTarget(&pThis->m0, pThis, fieldA3_3_exitTaskCallback, &position, NULL, 3, 0, eItems::mMinusOne, 0, 0);
        getFieldTaskPtr()->mC->mA3_conanaNestExit = 0;
    }

    static void Update(fieldA3_3_exitTask* pThis)
    {
        updateLCSTarget(&pThis->m0);
    }

    sLCSTarget m0;
    //size 34
};

void fieldA3_3_createExitTask(p_workArea workArea)
{
    createSubTask<fieldA3_3_exitTask>(workArea);
}

void fieldA3_3_createBirdsTask(p_workArea workArea)
{
    FunctionUnimplemented();
}

struct sfieldA3_3_checkExitsTask : public s_workAreaTemplate<sfieldA3_3_checkExitsTask>
{
    static void Update(sfieldA3_3_checkExitsTask* pThis)
    {
        if (getFieldTaskPtr()->mC->mA3_conanaNestExit)
        {
            mainGameState.setBit566(1274);

            startExitFieldCutscene(pThis, readCameraScript({ 0x60913CC, gFLD_A3 }), 9, 0, 0x8000);
            pThis->m_UpdateMethod = nullptr;
        }
    }
};

void fieldA3_3_createCheckExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_3_checkExitsTask>(workArea, &sfieldA3_3_checkExitsTask::Update);
}

void fieldA3_3_startTasks(p_workArea workArea)
{
    fieldA3_0_createTask0(workArea);
    fieldA3_3_createExitTask(workArea);
    fieldA3_3_createBirdsTask(workArea);
    fieldA3_3_createCheckExitTask(workArea);
    fieldA3_3_createItemBoxes(workArea);
}

void setupFieldCameraConfig_A3_3()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x608225C, gFLD_A3 }), 1);
}

void subfieldA3_3(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608C054, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F9E8, gFLD_A3 }, pDataTable3);
    setupField3(pDataTable3, fieldA3_3_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091400, gFLD_A3 });

    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_3();

    adjustVerticalLimits(-0x5C000, 0x76000);

    subfieldA3_1_Sub0();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(20, -1);


}
