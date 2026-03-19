#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

FLD_C8_data* gFLD_C8 = nullptr;

static const s_MCB_CGB fieldFileList_C8[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },       // 0
    { "FLD_T0.MCB", "FLD_T0.CGB" },        // 1
    { "FLD_T0_0.MCB", "FLD_T0_0.CGB" },    // 2
    { "FLD_T0_1.MCB", "FLD_T0_1.CGB" },    // 3
    { "FLD_T0_3.MCB", "FLD_T0_3.CGB" },    // 4
    { "FLD_T0_4.MCB", "FLD_T0_4.CGB" },    // 5
    { "FLD_T0_5.MCB", "FLD_T0_5.CGB" },    // 6
    { "FLD_T1_0.MCB", "FLD_T1_0.CGB" },    // 7
    { "FLD_T1_1.MCB", "FLD_T1_1.CGB" },    // 8
    { "FLD_T2_0.MCB", "FLD_T2_0.CGB" },    // 9
    { "FLD_T2_1.MCB", "FLD_T2_1.CGB" },    // 10
    { "FLD_T2_4.MCB", "FLD_T2_4.CGB" },    // 11
    { "FLD_T4_0.MCB", "FLD_T4_0.CGB" },    // 12
    { "FLD_T4_1.MCB", "FLD_T4_1.CGB" },    // 13
    { "FLD_T4_2.MCB", "FLD_T4_2.CGB" },    // 14
    { nullptr, nullptr },                    // 15 (empty)
    { nullptr, nullptr },                    // 16 (empty)
    { nullptr, nullptr },                    // 17 (empty)
    { nullptr, nullptr },                    // 18 (empty)
    { (const char*)-1, nullptr }
};

// 06054000
p_workArea overlayStart_FLD_C8(p_workArea workArea, u32 arg)
{
    if (gFLD_C8 == nullptr)
    {
        gFLD_C8 = new FLD_C8_data();
    }

    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    gFieldCameraConfigEA = gFLD_C8->getSaturnPtr(0x0608A868);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;

    if (!initField(workArea, fieldFileList_C8, arg))
    {
        return nullptr;
    }

    // Sound banks depend on field index
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m32_previousSubField == -1)
    {
        if (pFieldTask->m2C_currentFieldIndex == 0xF)
        {
            loadSoundBanks(0x2B, 0);
            mainGameState.bitField[0xA6] &= 0xEF;
        }
        else if (pFieldTask->m2C_currentFieldIndex == 0x12)
        {
            loadSoundBanks(0x32, 0);
            mainGameState.bitField[0xA6] |= 0x10;
        }
        playPCM(pFieldTask->m8_pSubFieldData, 100);
        Unimplemented(); // FUN_FLD_C8__060557e8
    }

    loadFileFromFileList(1);

    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_C8->getSaturnPtr(0x060B5170));

    Unimplemented(); // FUN_FLD_C8__06061048 — per-subfield file loading

    // Subfield dispatch
    s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    switch (subfield)
    {
    case 0: subfieldC8_0(workArea); break;
    default: Unimplemented(); break; // subfields 1-5
    }

    Unimplemented(); // post-subfield: m360 update function, script start

    return nullptr;
}
