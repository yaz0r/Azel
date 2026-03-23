#include "PDS.h"
#include "a3_background_layer.h"
#include "items.h"
#include "audio/soundDriver.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldRadar.h"
#include "field/exitField.h"
#include "field/fieldItemBox.h"
#include "field/battleStart.h"

extern s32 battleIndex; // TODO: cleanup
s32 playBattleSoundEffect(s32 effectIndex); // TODO: cleanup

void fieldA3_2_exit0(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9F_A3_2_exits[0] = 1;
}

void fieldA3_2_exit1(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9F_A3_2_exits[1] = 1;
}

void fieldA3_2_exit2(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9F_A3_2_exits[2] = 1;
}

void fieldA3_2_exit3(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9F_A3_2_exits[3] = 1;
}

void(*fieldA3_2_exitFuncTable[])(p_workArea, sLCSTarget*) = {
    fieldA3_2_exit0,
    fieldA3_2_exit1,
    fieldA3_2_exit2,
    fieldA3_2_exit3
};

struct fieldA3_2_exitLCSTask : public s_workAreaTemplate<fieldA3_2_exitLCSTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_2_exitLCSTask::Init, &fieldA3_2_exitLCSTask::Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_2_exitLCSTask* pThis)
    {
        for (int r12 = 3; r12 >= 0; r12--)
        {
            pThis->mD0[r12][0] = getSin(readSaturnFP(gFLD_A3->getSaturnPtr(0x6081E84 + 4 * r12)).getInteger() & 0xFFF) >> 4;
            pThis->mD0[r12][1] = 0;
            pThis->mD0[r12][2] = getCos(readSaturnFP(gFLD_A3->getSaturnPtr(0x6081E84 + 4 * r12)).getInteger() & 0xFFF) >> 4;

            static sVec3_FP exitLocations[4];
            exitLocations[r12] = readSaturnVec3(gFLD_A3->getSaturnPtr(0x6081E28 + 4 * 3 * r12));
            createLCSTarget(&pThis->m0[r12], pThis, fieldA3_2_exitFuncTable[r12], &exitLocations[r12], &pThis->mD0[r12], 3, 0, eItems::mMinusOne, 0, 0);

            getFieldSpecificData_A3()->m9F_A3_2_exits[r12] = 0;
        }

        pThis->m0[1].m18_diableFlags |= 1;
    }

    static void UpdateSub0(fieldA3_2_exitLCSTask* pThis)
    {
        for (int r12 = 3; r12 >= 0; r12--)
        {
            updateLCSTarget(&pThis->m0[r12]);
        }
    }

    static void Update(fieldA3_2_exitLCSTask* pThis)
    {
        if (mainGameState.getBit(0x11 * 8 + 5))
        {
            pThis->m0[1].m18_diableFlags = 0;
            pThis->m_UpdateMethod = fieldA3_2_exitLCSTask::UpdateSub0;
        }

        UpdateSub0(pThis);
    }

    std::array<sLCSTarget, 4> m0;
    std::array<sVec3_FP, 4> mD0;
    // size 0x100
};

void fieldA3_2_createExitLCSTask(p_workArea workArea)
{
    createSubTask<fieldA3_2_exitLCSTask>(workArea);
}

// Waterfall decoration task — animated 3D models / billboard sprites
// 06055CC6 (init), 06055B62 (update), 06055AD2 (draw)
struct sWaterfallTask : public s_workAreaTemplateWithArg<sWaterfallTask, sSaturnPtr>
{
    static void Init(sWaterfallTask* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memArea, 4);
        pThis->m8_position = readSaturnVec3(arg);
        pThis->m24_rotAngle = readSaturnS32(arg + 0x1C);
        pThis->m3E_mode = 0;
        // TODO: full init — createLCSTarget, read remaining params from arg
        Unimplemented();
    }
    static void Update(sWaterfallTask* pThis)
    {
        // TODO: animation cycling (mode 0) / billboard anim (mode 1)
    }
    static void Draw(sWaterfallTask* pThis)
    {
        if (pThis->m3E_mode == 0)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(&pThis->m8_position);
            // TODO: addObjectToDrawList from file bundle
            popMatrix();
        }
        // TODO: mode 1 billboard rendering
    }

    s_memoryAreaOutput m0_memArea;
    sVec3_FP m8_position;   // +0x14 in Saturn (offset 5*4)
    s32 m24_rotAngle;       // +0x24
    s16 m3C_animFrame;
    s8 m3E_mode;
    s8 m3F;
    // size: 0x7C
};

// Simpler waterfall element
// 06055D82 (init), 06055D4E (update), 06055D3A (draw)
struct sWaterfallSimpleTask : public s_workAreaTemplateWithArg<sWaterfallSimpleTask, sSaturnPtr>
{
    static void Init(sWaterfallSimpleTask* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memArea, 4);
        pThis->m8_position = readSaturnVec3(arg);
        Unimplemented();
    }
    static void Update(sWaterfallSimpleTask*) {}
    static void Draw(sWaterfallSimpleTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_position);
        // TODO: render waterfall billboard
        popMatrix();
    }

    s_memoryAreaOutput m0_memArea;
    sVec3_FP m8_position;
    // size: 0x28
};

// 06055e22
void fieldA3_2_createWaterfallTasks(p_workArea workArea)
{
    static const sWaterfallTask::TypedTaskDefinition waterfallDef = {
        &sWaterfallTask::Init, &sWaterfallTask::Update, &sWaterfallTask::Draw, nullptr
    };
    static const sWaterfallSimpleTask::TypedTaskDefinition waterfallSimpleDef = {
        &sWaterfallSimpleTask::Init, &sWaterfallSimpleTask::Update, &sWaterfallSimpleTask::Draw, nullptr
    };

    // Create 13 waterfall decoration tasks (iterate backwards like Ghidra)
    for (int i = 0xC; i >= 0; i--)
    {
        createSubTaskWithArg<sWaterfallTask, sSaturnPtr>(
            workArea, gFLD_A3->getSaturnPtr(0x0608335C + i * 0x20), &waterfallDef);
    }
    // Create 1 simple waterfall task
    for (int i = 0; i >= 0; i--)
    {
        createSubTaskWithArg<sWaterfallSimpleTask, sSaturnPtr>(
            workArea, gFLD_A3->getSaturnPtr(0x06083354 + i * 8), &waterfallSimpleDef);
    }
}

const std::array<s16, 4> A3_2_exitsVars =
{
    1270, 1271, 1272, 1273
};

static const std::array<s8, 4> A3_2_exitsLocations =
{
    0xA, 0xB, 0xB, 0xC
};

struct sfieldA3_2_checkExitsTask : public s_workAreaTemplate<sfieldA3_2_checkExitsTask>
{
    static void Update(sfieldA3_2_checkExitsTask* pThis)
    {
        for (int i = 0; i < 4; i++)
        {
            if (getFieldSpecificData_A3()->m9F_A3_2_exits[i])
            {
                mainGameState.setBit566(A3_2_exitsVars[i]);

                // Hack: this is in the function to be initialized after gFLD_A3
                static const std::array<sSaturnPtr, 8> A3_2_exitsCutscenes =
                {
                    {
                        {0x609122C, gFLD_A3},
                        {0x6091294, gFLD_A3},
                        {0x60912FC, gFLD_A3},
                        {0x6091364, gFLD_A3},
                    }
                };

                startExitFieldCutscene(pThis, readCameraScript(A3_2_exitsCutscenes[i]), A3_2_exitsLocations[i], i, 0x8000);
                pThis->m_UpdateMethod = NULL;
            }
        }
    }
};

void fieldA3_2_createCheckExitTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_2_checkExitsTask>(workArea, &sfieldA3_2_checkExitsTask::Update);
}

void fieldA3_2_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x609219C, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60921E4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609222C, gFLD_A3 }));
}

// 0606848a — waits for hasEncounterData, then sets battleIndex
struct sFlagBattleTask : public s_workAreaTemplate<sFlagBattleTask>
{
    static void Update(sFlagBattleTask* pThis)
    {
        if (hasEncounterData != 0)
        {
            battleIndex = pThis->m0_enemyId;
            pThis->getTask()->markFinished();
        }
    }
    s8 m0_enemyId;
};

// 06068728 — waits for m28_status to clear, then creates battle loading task
struct sDelayedBattleTask : public s_workAreaTemplate<sDelayedBattleTask>
{
    static void Update(sDelayedBattleTask* pThis)
    {
        if ((getFieldTaskPtr()->m28_status & 0xFFFE) == 0)
        {
            sBattleLoadingTask* pNewTask = createSubTaskWithArg<sBattleLoadingTask, s32>(
                getFieldTaskPtr(), pThis->m0_enemyId, &battleStartTaskDefinition);
            if (pNewTask)
            {
                pNewTask->m4 = pThis->m4_param2;
            }
            pThis->getTask()->markFinished();
        }
    }
    s32 m0_enemyId;
    s32 m4_param2;
};

// 060687f0
void startBaldorQueenBattle(int param_1, int param_2) {
    if ((fieldTaskPtr->m28_status & 1) == 0) {
        // Battle encounters disabled — set battleIndex when encounter data becomes available
        s_randomBattleWorkArea* pRandomBattleTask = getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask;
        sFlagBattleTask* pTask = createSubTaskFromFunction<sFlagBattleTask>((p_workArea)pRandomBattleTask, &sFlagBattleTask::Update);
        if (pTask)
        {
            pRandomBattleTask->m4 = 1;
            pTask->m0_enemyId = (s8)param_1;
        }
    }
    else if ((fieldTaskPtr->m28_status & ~1) == 0) {
        // Normal — create battle loading task directly
        sBattleLoadingTask* pNewTask = createSubTaskWithArg<sBattleLoadingTask, s32>(getFieldTaskPtr(), param_1, &battleStartTaskDefinition);
        if (pNewTask)
        {
            pNewTask->m4 = param_2;
        }
    }
    else {
        // Other status bits set — delay until they clear
        s_randomBattleWorkArea* pRandomBattleTask = getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask;
        sDelayedBattleTask* pTask = createSubTaskFromFunction<sDelayedBattleTask>((p_workArea)pRandomBattleTask, &sDelayedBattleTask::Update);
        if (pTask)
        {
            pTask->m0_enemyId = param_1;
            pTask->m4_param2 = param_2;
        }
    }
}

struct sBaldorQueenTrigger : public s_workAreaTemplate<sBaldorQueenTrigger> {
    // 06057874
    static void Update(sBaldorQueenTrigger* pThis) {
        switch (pThis->m0_State) {
        case 0:
            if (fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] < -0xED8000) {
                startBaldorQueenBattle(8, 0x8d);
                pThis->m0_State++;
            }
            break;
        case 1:
            // Wait for battle completion (bit 0x8D = bitField[0x11] & 0x04)
            if (mainGameState.getBit(0x11 * 8 + 5) != 0) {
                startFieldScript(8, 1445);
                pThis->m0_State++;
            }
            break;
        default:
            pThis->getTask()->markFinished();
            break;
        }
    }

    u32 m0_State = 0;
    //size: 4
};

void fieldA3_2_createsBaldorQueenTrigger(p_workArea workArea) {
    if (mainGameState.getBit(0x11 * 8 + 5) == 0) {
        createSubTaskFromFunction<sBaldorQueenTrigger>(workArea, sBaldorQueenTrigger::Update);
    }
}

// 060578C8
struct sFieldA2_2_task7 : public s_workAreaTemplate<sFieldA2_2_task7> {
    static void Update(sFieldA2_2_task7* pThis) {
        if (fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] < -0x1E78000) {
            startBaldorQueenBattle(9, 0x8e);
            pThis->m_UpdateMethod = nullptr;
        }
    }
    //size: 0
};


void fieldA3_2_createTask7(p_workArea workArea) {
    if (mainGameState.getBit(0x11 * 8 + 6) == 0) {
        createSubTaskFromFunction<sFieldA2_2_task7>(workArea, sFieldA2_2_task7::Update);
    }
}

// 060579d8
struct sFieldA3_2_task8 : public s_workAreaTemplate<sFieldA3_2_task8> {
    static void Update(sFieldA3_2_task8* pThis) {
        // bitField[0x11] & 1 = getBit(0x11 * 8 + 7)
        if (mainGameState.getBit(0x11 * 8 + 7) == 0) {
            // Not yet triggered — set update to no-op
            pThis->m_UpdateMethod = nullptr;
        }
        else {
            pThis->getTask()->markFinished();
        }
    }
    s32 m0;
    // size: 0xC (Saturn)
};

// 06057ab6
void fieldA3_2_createPreBattleWaitTask(p_workArea workArea) {
    createSubTaskFromFunction<sFieldA3_2_task8>(workArea, &sFieldA3_2_task8::Update);
}

// Water region bounds data from 0x0608FA54
struct sWaterRegion { s32 xMin; s32 xMax; s32 zMin; s32 zMax; };
static const sWaterRegion waterRegions[] = {
    { 0x004FE000, 0x0068A000, (s32)0xFF1B4000, (s32)0xFF30E000 },
    { 0x003BE000, 0x0055E000, (s32)0xFF72C000, (s32)0xFF85E000 },
    { 0, 0, 0, 0 }, // sentinel
};

// 060585d8 — check if dragon is inside a water region
static bool isDragonInWaterRegion(const sVec3_FP& pos, s32 regionIndex)
{
    const sWaterRegion& r = waterRegions[regionIndex];
    return (r.xMin < pos.m0_X && pos.m0_X < r.xMax &&
            r.zMin < pos.m8_Z && pos.m8_Z < r.zMax);
}

// 0605867a — check if dragon is looking sideways (at the water)
static bool isDragonLookingSideways(s_dragonTaskWorkArea* pDragon)
{
    s32 angle = pDragon->m20_angle[1];
    // Sign-extend 28-bit angle
    if (angle & 0x8000000)
        angle |= (s32)0xF0000000;
    else
        angle &= 0x0FFFFFFF;

    // Check if angle is in sideways ranges
    if ((angle >= 0x3555555 && angle <= 0x4AAAAAA) ||
        (angle >= (s32)0xFB555556 && angle <= (s32)0xFCAAAAAB))
        return true;
    return false;
}

// 060586d8
struct sFieldA3_2_waterEffect : public s_workAreaTemplate<sFieldA3_2_waterEffect> {
    // 0605860a — fade water intensity down (decrease)
    bool fadeDown()
    {
        m6_intensity--;
        if (m6_intensity < 0x19)
            m6_intensity = 0x19;
        vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA =
            (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | m6_intensity;
        vdp2Controls.m_isDirty = 1;
        return m6_intensity != 0x19;
    }

    // 06058642 — fade water intensity up (increase)
    bool fadeUp()
    {
        m6_intensity++;
        if (m6_intensity > 0x1F)
            m6_intensity = 0x1F;
        vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA =
            (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | m6_intensity;
        vdp2Controls.m_isDirty = 1;
        return m6_intensity != 0x1F;
    }

    static void Update(sFieldA3_2_waterEffect* pThis) {
        s_dragonTaskWorkArea* pDragon = fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask;

        switch (pThis->m4_state)
        {
        case 0: // idle — scan for dragon entering water region
            for (s32 i = 2; i >= 0; i--)
            {
                if (isDragonInWaterRegion(pDragon->m8_pos, i))
                {
                    pThis->m5_regionIndex = (s8)i;
                    pThis->m6_intensity = 0x1F;
                    vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA =
                        (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | pThis->m6_intensity;
                    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
                    vdp2Controls.m_isDirty = 1;
                    pThis->m4_state = 1;
                    return;
                }
            }
            break;
        case 1: // in water — wait for dragon to look sideways
            if (!isDragonInWaterRegion(pDragon->m8_pos, pThis->m5_regionIndex))
            {
                pThis->m4_state = 5;
                return;
            }
            if (isDragonLookingSideways(pDragon))
            {
                pThis->m4_state = 2;
            }
            break;
        case 2: // fade water in
            if (!isDragonInWaterRegion(pDragon->m8_pos, pThis->m5_regionIndex))
            {
                pThis->m4_state = 5;
                return;
            }
            if (!pThis->fadeDown())
            {
                pThis->m4_state = 3;
            }
            break;
        case 3: // fully visible — wait for dragon to stop looking sideways
            if (!isDragonInWaterRegion(pDragon->m8_pos, pThis->m5_regionIndex))
            {
                pThis->m4_state = 5;
                return;
            }
            if (!isDragonLookingSideways(pDragon))
            {
                pThis->m4_state = 4;
            }
            break;
        case 4: // fade water out
            if (!isDragonInWaterRegion(pDragon->m8_pos, pThis->m5_regionIndex))
            {
                pThis->m4_state = 5;
                return;
            }
            if (!pThis->fadeUp())
            {
                pThis->m4_state = 1;
            }
            break;
        case 5: // exiting water — fade out fully
            if (!pThis->fadeUp())
            {
                vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
                vdp2Controls.m_isDirty = 1;
                pThis->m4_state = 0;
            }
            break;
        }

        // VDP2 scroll update
        pauseEngine[4] = 0;
        s_visibilityGridWorkArea* pFieldCamera = fieldTaskPtr->m8_pSubFieldData->m348_pFieldCameraTask1;
        setupVDP2CoordinatesIncrement2(
            pDragon->m8_pos[2] << 4,
            (0x2AA - *(s32*)((u8*)pFieldCamera + 0x34)) * 0x10000);
        pauseEngine[4] = 4;
    }

    s8 m4_state = 0;
    s8 m5_regionIndex;
    s8 m6_intensity;
    // size: 8
};

// 06058898
void fieldA3_2_createWaterReflectionTask(p_workArea workArea) {
    if (mainGameState.getBit(0xA * 8 + 7) != 0) {
        createSubTaskFromFunction<sFieldA3_2_waterEffect>(workArea, &sFieldA3_2_waterEffect::Update);
    }
}

// Sound task for A3_2 — handles positional waterfall sounds
struct sFieldA3_2_soundTask : public s_workAreaTemplate<sFieldA3_2_soundTask> {
    // 0605C3B0
    static void Init(sFieldA3_2_soundTask*) {
        getFieldSpecificData_A3()->m134 = 0;
        getFieldSpecificData_A3()->m138 = 0;
    }
    // 0605C296
    static void Update(sFieldA3_2_soundTask*) {
        if (getFieldSpecificData_A3()->m134 == 0) {
            playBattleSoundEffect(0x6A);
        }
        else {
            // TODO: positional sound update for sound 0x6A
            Unimplemented();
            getFieldSpecificData_A3()->m134 = 0;
        }
        if (getFieldSpecificData_A3()->m138 == 0) {
            playBattleSoundEffect(0x6B);
        }
        else {
            // TODO: positional sound update for sound 0x6B
            Unimplemented();
            getFieldSpecificData_A3()->m138 = 0;
        }
    }
    // 0605C286
    static void Delete(sFieldA3_2_soundTask*) {
        Unimplemented();
    }
};

// 0605C3D2
void fieldA3_2_createWaterfallSoundTask(p_workArea workArea) {
    static const sFieldA3_2_soundTask::TypedTaskDefinition definition = {
        &sFieldA3_2_soundTask::Init,
        &sFieldA3_2_soundTask::Update,
        nullptr,
        &sFieldA3_2_soundTask::Delete,
    };
    createSubTask<sFieldA3_2_soundTask>(workArea, &definition);
}

void fieldA3_2_startTasks(p_workArea workArea)
{
    fieldA3_0_createTask0(workArea);

    getFieldSpecificData_A3()->m168 = createParticlePoolTask(workArea, 4, 0x50);

    fieldA3_2_createExitLCSTask(workArea);
    create_fieldA3_backgroundLayer2(workArea);
    fieldA3_2_createWaterfallTasks(workArea);
    fieldA3_2_createCheckExitTask(workArea);

    fieldA3_2_createsBaldorQueenTrigger(workArea);
    fieldA3_2_createTask7(workArea);
    fieldA3_2_createPreBattleWaitTask(workArea);
    fieldA3_2_createWaterReflectionTask(workArea);
    fieldA3_2_createWaterfallSoundTask(workArea);

    fieldA3_2_createItemBoxes(workArea);
}

// 06058452 - adjusts dragon min Y height for the water section of A3_2
void updateDragonHeightLimitForWater_A3_2(s_dragonTaskWorkArea* pThis)
{
    if ((s32)pThis->m8_pos.m8_Z < (s32)0xFEFF2000) // Z < -256.875 (over water)
    {
        pThis->m134_minY = 0x5000; // +0.3125 (stay above water surface)
    }
    else
    {
        pThis->m134_minY = 0xFFFAC000; // -5.25 (normal height limit over land)
    }
}

void subfieldA3_2(p_workArea workArea)
{
    s16 r13 = getFieldTaskPtr()->m30_fieldEntryPoint;

    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608BE04, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F8BC, gFLD_A3 }, pDataTable3);
    s_DataTable2* pDataTable2 = readDataTable2({ 0x6088E8C, gFLD_A3 });
    setupField(pDataTable3, pDataTable2, fieldA3_2_startTasks, pVisibility);

    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = updateDragonHeightLimitForWater_A3_2;

    switch (getFieldTaskPtr()->m32_previousSubField)
    {
    case 0xB:
        if (getFieldTaskPtr()->m30_fieldEntryPoint == 0)
        {
            // back from cave
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x60912C8, gFLD_A3 });
        }
        else
        {
            // back from next zone
            getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091330, gFLD_A3 });
        }
        break;
    case 0xC:
        // back from next zone
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091398, gFLD_A3 });
        break;
    default:
        getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x6091260, gFLD_A3 });
        break;
    }

    startFieldScript(19, -1);

    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    adjustVerticalLimits(-0x54000, 0x76000);

    fieldRadar_enableAltitudeGauge();

    //subfieldA3_1_Sub1();

    PDS_unimplemented("subfieldA3_2");
}
