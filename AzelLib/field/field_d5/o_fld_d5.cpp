#include "PDS.h"
#include "o_fld_d5.h"
#include "d5_nameEntry.h"
#include "d5_starfield.h"
#include "d5_gameOver.h"
#include "field.h"
#include "field/field_a3/o_fld_a3.h"
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

// 0605414c: D5 subfield init
static void initSubfield_D5(p_workArea parent)
{
    Unimplemented();
}

p_workArea overlayStart_FLD_D5(p_workArea workArea, u32 arg)
{
    if (gFLD_D5 == NULL)
    {
        gFLD_D5 = new FLD_D5_data();
    }

    // TODO: cleanup
    // FLD_A3.PRG contains shared field infrastructure data (camera configs, etc.)
    // that initField/fieldOverlaySubTaskInit depends on
    if (gFLD_A3 == NULL)
    {
        gFLD_A3 = new FLD_A3_data();
    }

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
