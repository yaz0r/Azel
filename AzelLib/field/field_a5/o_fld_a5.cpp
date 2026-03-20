#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

FLD_A5_data* gFLD_A5 = nullptr;

static const s_MCB_CGB fieldFileList[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },
    { "FLD_A5.MCB", "FLD_A5.CGB" },
    { "FLD_A5_0.MCB", "FLD_A5_0.CGB" },
    { "FLD_A5_2.MCB", "FLD_A5_2.CGB" },
    { "FLD_A5_4.MCB", "FLD_A5_4.CGB" },
    { "FLD_A5_7.MCB", "FLD_A5_7.CGB" },
    { "FLD_A5_8.MCB", "FLD_A5_8.CGB" },
    { "FLD_A5_9.MCB", "FLD_A5_9.CGB" },
    { "FLD_A5_B.MCB", "FLD_A5_B.CGB" },
    { (const char*)-1, nullptr }
};

// 06054000
p_workArea overlayStart_FLD_A5(p_workArea workArea, u32 arg)
{
    if (gFLD_A5 == nullptr)
    {
        gFLD_A5 = new FLD_A5_data();
    }

    // FLD_A3.PRG contains shared field infrastructure data (dragon anim tables, etc.)
    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    // 06072e36 — override subfield for specific game state
    if (mainGameState.bitField[0x1B] & 8)
    {
        getFieldTaskPtr()->m2E_currentSubFieldIndex = 0xB;
        getFieldTaskPtr()->m30_fieldEntryPoint = 0;
    }

    gFieldCameraConfigEA = gFLD_A5->getSaturnPtr(0x0609E938);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    if (!initField(workArea, fieldFileList, arg))
    {
        return nullptr;
    }

    s16 subFieldIndex = getFieldTaskPtr()->m2E_currentSubFieldIndex;

    if (subFieldIndex == 0xB)
    {
        loadSoundBanks(1, 0);
        playPCM(workArea, 100);
    }
    else
    {
        loadSoundBanks(7, 0);
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_A5->getSaturnPtr(0x0609D4A8));

    // Per-subfield dispatch
    static void (*subfieldTable[])(p_workArea) = {
        subfieldA5_0, subfieldA5_1, subfieldA5_2, subfieldA5_3,
        subfieldA5_4, subfieldA5_5, subfieldA5_6, subfieldA5_7,
        subfieldA5_8, subfieldA5_9, subfieldA5_A, subfieldA5_B,
    };

    s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    if (subfield < (s16)(sizeof(subfieldTable) / sizeof(subfieldTable[0])))
    {
        subfieldTable[subfield](workArea);
    }
    else
    {
        assert(0); // unimplemented subfield
    }

    Unimplemented(); // post-subfield init: collision data table, update functions

    if (subfield != 0xB)
    {
        startFieldScript(subfield + 0x19, -1);
    }

    return nullptr;
}
