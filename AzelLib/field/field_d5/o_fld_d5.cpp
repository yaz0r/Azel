#include "PDS.h"
#include "o_fld_d5.h"
#include "d5_nameEntry.h"
#include "d5_starfield.h"
#include "d5_gameOver.h"
#include "field.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

FLD_D5_data* gFLD_D5 = NULL;

static const s_MCB_CGB fieldFileList_D5[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },
    { NULL, NULL },
    { "FLD_D5_0.MCB", "FLD_D5_0.CGB" },
    { (const char*)-1, NULL },
};

// 06055088
static void nameEntryFieldCallback(p_workArea)
{
}

// 060550b4
static void gameOverFieldCallback(p_workArea)
{
}

// 0605508c
static void subfieldD5_initCallback(p_workArea parent)
{
    // 060541cc — create field-specific data subtask (4-byte work area stored in mC)
    Unimplemented(); // creates a minimal subtask and stores in getFieldTaskPtr()->mC
    // Also creates background atmosphere subtask (createFieldSubTask24)
}

// 06055170
static void dragonFieldUpdateD5(s_dragonTaskWorkArea* pDragon)
{
    Unimplemented(); // reads from field-specific data at mC, updates dragon position
}

// 0605414c: D5 subfield init
static void initSubfield_D5(p_workArea parent)
{
    setupField2(nullptr, subfieldD5_initCallback);
    createD5StarfieldTask(parent);
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF0 = (void(*)(s_dragonTaskWorkArea*))dragonFieldUpdateD5;
    fieldRadar_setEncounterDistance(fixedPoint(0x28000));
}

p_workArea overlayStart_FLD_D5(p_workArea workArea, u32 arg)
{
    if (gFLD_D5 == NULL)
    {
        gFLD_D5 = new FLD_D5_data();
    }

    // FLD_A3.PRG contains shared field infrastructure data (camera configs, etc.)
    if (gFLD_A3 == NULL)
    {
        gFLD_A3 = new FLD_A3_data();
    }
    gFieldCameraConfigEA = { 0x6092EF0, gFLD_A3 };
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    loadSoundBanks(0x35, 0);
    playPCM(workArea, 100);

    if (!initField(workArea, fieldFileList_D5, arg))
    {
        return NULL;
    }

    loadFileFromFileList(2);

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();

    if (pFieldTask->m2C_currentFieldIndex == 0)
    {
        // Name entry screen
        setupField2(nullptr, nameEntryFieldCallback);
        createNameEntryTask(workArea);
        createNameEntryParticleTask(workArea);
    }
    else if (pFieldTask->m2C_currentFieldIndex == 0x13)
    {
        // D5 field
        initSubfield_D5(workArea);
    }
    else
    {
        // Game over screen
        setupField2(nullptr, gameOverFieldCallback);
        createGameOverParticleTask(workArea);
        createGameOverSecondaryTask(workArea);
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    pFieldTask = getFieldTaskPtr();
    pFieldTask->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_D5->getSaturnPtr(0x06079b4c));

    return workArea;
}
