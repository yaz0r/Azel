#include "PDS.h"
#include "o_fld_b2.h"
#include "field/field_a3/o_fld_a3.h"
#include "battle/battleEngine.h"
#include "3dEngine.h"

FLD_B2_data* gFLD_B2 = nullptr;

static const s_MCB_CGB fieldFileList_B2[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },     // 0
    { nullptr,      "FLD_B2.CGB" },      // 1
    { "FLD_B2_1.MCB", "FLD_B2_1.CGB" },  // 2
    { "FLD_B2_3.MCB", "FLD_B2_3.CGB" },  // 3
    { "FLD_B2_4.MCB", "FLD_B2_4.CGB" },  // 4
    { "FLD_B2_1.MCB", "FLD_B2_1.CGB" },  // 5
    { (const char*)-1, nullptr }
};

// --- Dragon light parameter setup functions ---

// 060540c4 — dragon light params for subfields 0 and 3
void initDragonParams_B2_desert()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC8_normalLightColor = { 0xA, 0xA, 0xA };
    p->mCB_falloffColor0 = { 9, 9, 9 };
    p->mCE_falloffColor1 = { (s8)0xF1, 5, 5 };
    p->mD1_falloffColor2 = { 0xC, 0xC, 0xC };
}

// 06054160 — dragon light params for subfield 1
void initDragonParams_B2_oasis()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC0_lightRotationAroundDragon = fixedPoint(0xB82D82D);
    p->mC4 = fixedPoint(0x71C71C);
    p->mC8_normalLightColor = { 0xF, 0x10, 0xF };
    p->mCB_falloffColor0 = { 4, 4, 4 };
    p->mCE_falloffColor1 = { (s8)0xE0, (s8)0xE0, (s8)0xE0 };
    p->mD1_falloffColor2 = { 1, 1, 1 };
}

// 060541b8 — dragon light params for subfield 1 storm variant
void initDragonParams_B2_oasisStorm()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    syncM68KSoundCPU();
    p->mC8_normalLightColor = { 0x10, 0x1F, 0x1F };
    p->mCB_falloffColor0 = { 3, 3, 2 };
    p->mCE_falloffColor1 = { (s8)0xEC, (s8)0xEC, (s8)0xEC };
    p->mD1_falloffColor2 = { 7, 7, 7 };
}

// 06054210 — dragon light params for subfield 2
void initDragonParams_B2_underground()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
    p->mC4 = fixedPoint(0);
    p->mC8_normalLightColor = { 0x10, 0x10, 0x10 };
    p->mCB_falloffColor0 = { 8, 8, 8 };
    p->mCE_falloffColor1 = { 0x14, 0x14, 0x14 };
    p->mD1_falloffColor2 = { 0xA, 0xA, 0xA };
}

// --- Shared field data creation ---

// 0605490c — create field-specific data task for B2 (size 0xA4)
void createFieldSpecificDataTask_B2(p_workArea parent)
{
    s_fieldSpecificData_B2* p = createSubTaskFromFunction<s_fieldSpecificData_B2>(parent, nullptr);
    if (p != nullptr)
    {
        getFieldTaskPtr()->mC = (p_workArea)p;
    }
}

// --- Shared camera/scene setup ---

// 06072f0e — setup field grid + camera visibility from overlay data
void setupB2CameraConfigs(sSaturnPtr configEA, sSaturnPtr visibilityEA, void(*startTasks)(p_workArea))
{
    s_DataTable3* pDT3 = readDataTable3(configEA);
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility(visibilityEA, pDT3);
    setupField3(pDT3, startTasks, pVisibility);
}

// 06056e7c — camera config setup (desert)
void setupCameraVisibility_B2_desert()
{
    setupFieldCameraConfigs(readCameraConfig(gFLD_B2->getSaturnPtr(0x06083de4)), 1);
}

// 06056e88 — camera config setup (underground)
void setupCameraVisibility_B2_underground()
{
    setupFieldCameraConfigs(readCameraConfig(gFLD_B2->getSaturnPtr(0x06083e94)), 1);
}

// 06057084 — empty post-battle sound callback (subfields 0, 3)
void postBattleSound_B2_empty()
{
    // empty — confirmed from Ghidra
}

// 06076b80 — load camera script and activate camera script mode on dragon
void activateCameraScript_B2(sSaturnPtr scriptDataEA)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->m1D0_cameraScript = readCameraScript(scriptDataEA);
    pDragon->m104_dragonScriptStatus = 0;
    pDragon->mF0 = &dragonScriptMovement;
    pDragon->mF8_Flags &= ~0x400;
}

// --- Entry point ---

// 06054000
p_workArea overlayStart_FLD_B2(p_workArea workArea, u32 arg)
{
    if (gFLD_B2 == nullptr)
    {
        gFLD_B2 = new FLD_B2_data();
    }

    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    // Set field globals (shared infrastructure from A3)
    gFieldCameraConfigEA = { 0x6092EF0, gFLD_A3 };
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;
    gCurrentFieldOverlay = gFLD_A3;

    // 0606cfe4 — check if coming from field 6, subfield 0, entry 0, previous -1
    // If so, redirect to subfield 3
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 6)
    {
        pFieldTask = getFieldTaskPtr();
        if (pFieldTask->m2E_currentSubFieldIndex == 0 &&
            pFieldTask->m30_fieldEntryPoint == 0 &&
            pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            // 0606d01e — redirect to subfield 3
            pFieldTask = getFieldTaskPtr();
            pFieldTask->m2E_currentSubFieldIndex = 3;
            pFieldTask = getFieldTaskPtr();
            pFieldTask->m30_fieldEntryPoint = 0;
        }
    }

    if (!initField(workArea, fieldFileList_B2, arg))
    {
        return nullptr;
    }

    loadFileFromFileList(1);

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    // Set camera follow mode index
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;

    // Load scripts
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_B2->getSaturnPtr(0x0608CE90));

    // Dispatch to subfield
    static void (*subfieldTable[])(p_workArea) = {
        subfieldB2_0,
        subfieldB2_1,
        subfieldB2_2,
        subfieldB2_3,
    };

    s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    if (subfield < (s16)(sizeof(subfieldTable) / sizeof(subfieldTable[0])))
    {
        subfieldTable[subfield](workArea);
    }

    // TODO: Set depth range table at m348->m2C (s_visibilityGridWorkArea is incomplete in C++)

    return nullptr;
}
