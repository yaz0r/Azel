#include "PDS.h"
#include "o_fld_a3_0.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

void subfieldA3_4_sub0()
{
    FunctionUnimplemented();
}

void vdp2FieldTask_init(s_fieldPaletteTaskWorkArea* pThis)
{
    getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask = pThis;
    reinitVdp2();
    fieldPaletteTaskInitSub0();
    
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x31FF75FF;

    // setup line color screen
    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    // setup back screen color
    *(u16*)getVdp2Vram(0x2A600) = 0x38E5;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x0;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 0x3;

    vdp2Controls.m_isDirty = 1;
}

void vdp2FieldTask_update(s_fieldPaletteTaskWorkArea* pThis)
{
    // Nothing
}

void createVDP2FieldTask(p_workArea workArea)
{
    s_fieldPaletteTaskWorkArea::TypedTaskDefinition definition = {&vdp2FieldTask_init, &vdp2FieldTask_update, nullptr, nullptr};

    createSubTask<s_fieldPaletteTaskWorkArea>(workArea, &definition);
}

void setupFieldCameraConfig_A3_4()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x60822B4, gFLD_A3 }), 1);
}

struct fieldA3_4_exitTask : public s_workAreaTemplate<fieldA3_4_exitTask>
{
    static void Update(fieldA3_4_exitTask* pThis)
    {
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[0] < 0x40000)
        {
            mainGameState.setBit566(A3_0_exitsVars[0]);
            exitCutsceneTaskUpdateSub0(0, 0, -1);
            pThis->m8 = 0;
        }
        else if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] < -0x1A2000)
        {
            mainGameState.setBit566(A3_0_exitsVars[1]);
            exitCutsceneTaskUpdateSub0(0, 1, -1);
            pThis->m8 = 0;
        }
    }

    s32 m8;
    //size C
};

void fieldA3_4_createExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<fieldA3_4_exitTask>(workArea, &fieldA3_4_exitTask::Update);
}

void fieldA3_4_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092304, gFLD_A3 }));
}

void fieldA3_4_startTasks(p_workArea workArea)
{
    fieldA3_4_createExitTask(workArea);
    fieldA3_4_createItemBoxes(workArea);
}

void subfieldA3_4(p_workArea workArea)
{
    soundFunc(100);
    soundFunc(101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608EDA8, gFLD_A3 });
    setupField2(pDataTable3, fieldA3_4_startTasks);

    if (!getFieldTaskPtr()->m30_fieldEntryPoint)
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B0C, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C20, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }
    else
    {
        sVec3_FP position = readSaturnVec3({ 0x6081B0C + 0xC, gFLD_A3 });
        sVec3_FP rotation = readSaturnVec3({ 0x6081C08, gFLD_A3 });
        setupDragonPosition(&position, &rotation);
    }

    subfieldA3_4_sub0();

    setupFieldCameraConfig_A3_4();

    createVDP2FieldTask(workArea);
}
