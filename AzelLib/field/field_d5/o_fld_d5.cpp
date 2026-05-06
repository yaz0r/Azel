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

struct sFieldSpecificData_D5 : public s_workAreaTemplate<sFieldSpecificData_D5>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, nullptr };
        return &taskDefinition;
    }
    // size 0x04
};

// 060541cc
static void createFieldSpecificDataTask_D5(p_workArea parent)
{
    sFieldSpecificData_D5* p = createSubTask<sFieldSpecificData_D5>(parent);
    if (p)
    {
        getFieldTaskPtr()->mC = (p_workArea)p;
    }
}

// 0605508c
static void subfieldD5_initCallback(p_workArea parent)
{
    createFieldSpecificDataTask_D5(parent);
    Unimplemented(); // createFieldSubTask24 — creates 0x24-byte game-over sequence task
}

// 06055170
static void dragonFieldUpdateD5(s_dragonTaskWorkArea* pDragon)
{
    s32* pFieldData = (s32*)getFieldTaskPtr()->mC;
    pDragon->m8_pos.m4_Y = fixedPoint(0);
    pDragon->m8_pos.m0_X = fixedPoint(0);
    if (pFieldData[1] == 0)
    {
        pDragon->m160_deltaTranslation.m8_Z = fixedPoint(0);
    }
    else
    {
        pDragon->m160_deltaTranslation.m8_Z = fixedPoint((s32)0xFFFFA778);
    }
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
    gCurrentFieldOverlay = gFLD_A3;

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
