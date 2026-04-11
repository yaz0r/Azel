#include "PDS.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "a7_cellObj0.h"
#include "a7_cellObj1.h"
#include "a7_beamEmitter.h"
#include "a7_cellObj3.h"
#include "a7_dialogChoiceTask.h"
#include "kernel/loadSavegameScreen.h"

FLD_A7_data* gFLD_A7 = nullptr;

void freeVdp1Block(npcFileDeleter* parent, void* dataToFree);
void fieldPaletteTaskInitSub0Sub0();

void FLD_A7_data::dispatchCellObjectCreation(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    switch (r5.m0_function.m_offset)
    {
    case 0x06054a8c:
        create_A7_CellObj0(r4, r5, r6);
        break;
    case 0x0605ccd0:
        create_A7_CellObj1(r4, r5, r6);
        break;
    case 0x06055da6:
        create_A7_CellObj2(r4, r5, r6);
        break;
    case 0x06055fc6:
        create_A7_CellObj3(r4, r5, r6);
        break;
    default:
        PDS_unimplemented("FLD_A7_data::dispatchCellObjectCreation");
        break;
    }
}

// 06073566
static s32 isDragonCameraScriptInactive()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragon->m1D0_cameraScript == nullptr && pDragon->m1D4_cutsceneData == nullptr)
    {
        return 1;
    }
    return 0;
}

// 060778dc — mark the selected choice entry in the choice table
static void a7MarkMultiChoiceSelected(s32 choice)
{
    s_multiChoice* pChoice = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData;
    if (pChoice != nullptr)
    {
        pChoice->m0_choiceTable[choice] = 9;
    }
}

// 0605e7f4
static s32 getMultiChoiceResult_A7()
{
    s_multiChoice* pChoice = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData;
    s32 result = pChoice->m4_currentChoice;
    if (result == 4)
    {
        pChoice->m0_choiceTable[4] = 0;
    }
    return result;
}

// 0607790a
static void a7ReleaseMultiChoiceBlocks()
{
    s_fieldScriptWorkArea* pScript = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_multiChoice* pChoice = pScript->m44_multiChoiceData;
    if (pChoice != nullptr)
    {
        freeVdp1Block((npcFileDeleter*)pScript, pChoice->m0_choiceTable);
        freeVdp1Block((npcFileDeleter*)pScript, pChoice);
        pScript->m44_multiChoiceData = nullptr;
        fieldPaletteTaskInitSub0Sub0();
    }
}

// 0605e81e
static s32 handleMultiChoiceResult_A7()
{
    s_multiChoice* pChoice = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m44_multiChoiceData;
    s32 result = pChoice->m4_currentChoice;
    if (result == 5)
    {
        if (getFieldTaskPtr()->m2C_currentFieldIndex == 4)
        {
            mainGameState.bitField[0xa3] |= 0x10;
            a7ReleaseMultiChoiceBlocks();
        }
        else
        {
            fadeOutAllSequences();
            dispatchTutorialMultiChoiceSub2();
        }
    }
    else
    {
        pChoice->mC = 0;
        a7DialogChoice_spawn_0605ea52((p_workArea)getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE, result);
        playSystemSoundEffect(3);
        a7MarkMultiChoiceSelected(result);
    }
    return result;
}

s32 FLD_A7_data::executeNative(sSaturnPtr ptr)
{
    switch (ptr.m_offset)
    {
    case 0x06073566:
        return isDragonCameraScriptInactive();
    case 0x0605e7f4:
        return getMultiChoiceResult_A7();
    case 0x0605e81e:
        return handleMultiChoiceResult_A7();
    default:
        PDS_unimplemented("FLD_A7_data::executeNative");
        break;
    }
    return 0;
}

s32 FLD_A7_data::executeNative(sSaturnPtr ptr, s32 arg0)
{
    switch (ptr.m_offset)
    {
    default:
        PDS_unimplemented("FLD_A7_data::executeNative");
        break;
    }
    return 0;
}

static const s_MCB_CGB fieldFileList[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },       // 0
    { nullptr, nullptr },                    // 1 (no FLD_A7.MCB)
    { "FLD_A7_0.MCB", "FLD_A7_0.CGB" },    // 2
    { "FLD_A7_1.MCB", "FLD_A7_1.CGB" },    // 3
    { "FLD_A7_2.MCB", "FLD_A7_2.CGB" },    // 4
    { (const char*)-1, nullptr }
};

// 06054000
p_workArea overlayStart_FLD_A7(p_workArea workArea, u32 arg)
{
    if (gFLD_A7 == nullptr)
    {
        gFLD_A7 = new FLD_A7_data();
    }

    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    gFieldCameraConfigEA = gFLD_A7->getSaturnPtr(0x060866d4);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;
    gCurrentFieldOverlay = gFLD_A7;

    if (!initField(workArea, fieldFileList, arg))
    {
        return nullptr;
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_A7->getSaturnPtr(0x060861b0));

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex != 4 && pFieldTask->m2C_currentFieldIndex != 0x16)
    {
        if (pFieldTask->m32_previousSubField == (s16)0xFFFF)
        {
            loadSoundBanks(0xF, 0);
            playPCM(workArea, 100);
        }

        static void (*subfieldTable2[])(p_workArea) = {
            subfieldA7_2, subfieldA7_1,
        };
        subfieldTable2[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
    }
    else
    {
        loadSoundBanks(0xB, 0);

        static void (*subfieldTable1[])(p_workArea) = {
            subfieldA7_0, subfieldA7_1, subfieldA7_2,
        };
        subfieldTable1[getFieldTaskPtr()->m2E_currentSubFieldIndex](workArea);
    }

    return nullptr;
}
