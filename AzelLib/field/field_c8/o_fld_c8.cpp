#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldDragonMovement.h"

std::vector<sLCSTaskDrawSub5Sub1_Data1> readLCSTaskDrawSub5Sub1_Data1(sSaturnPtr source);
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

// 060557e8
static void createFieldSpecificDataTask_C8()
{
    Unimplemented(); // 0607a516 — creates dummy subtask (size 0) under overlay task

    s_fieldSpecificData_C8* pFieldData = createSubTaskFromFunction<s_fieldSpecificData_C8>(
        getFieldTaskPtr()->m8_pSubFieldData, nullptr);
    if (pFieldData)
    {
        getFieldTaskPtr()->mC = pFieldData;
    }
}

// 06060f9c — set dragon light colors from per-subfield table
static void initDragonLightColors_C8(s32 subfieldIndex)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (!pDragon) return;

    sSaturnPtr colorData = readSaturnEA(gFLD_C8->getSaturnPtr(0x060B3AF4 + subfieldIndex * 8));
    pDragon->mC8_normalLightColor = { (s8)readSaturnU8(colorData), (s8)readSaturnU8(colorData + 1), (s8)readSaturnU8(colorData + 2) };
    pDragon->mCB_falloffColor0 = { (s8)readSaturnU8(colorData + 3), (s8)readSaturnU8(colorData + 4), (s8)readSaturnU8(colorData + 5) };
    pDragon->mCE_falloffColor1 = { (s8)readSaturnU8(colorData + 6), (s8)readSaturnU8(colorData + 7), (s8)readSaturnU8(colorData + 8) };
    pDragon->mD1_falloffColor2 = { (s8)readSaturnU8(colorData + 9), (s8)readSaturnU8(colorData + 10), (s8)readSaturnU8(colorData + 11) };
}

// 06061014 — set dragon light rotation from per-subfield table
static void initDragonLightRotation_C8(s32 subfieldIndex)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (!pDragon) return;

    sSaturnPtr rotData = readSaturnEA(gFLD_C8->getSaturnPtr(0x060B3AF0 + subfieldIndex * 8));
    pDragon->mC0_lightRotationAroundDragon = readSaturnS32(rotData);
    pDragon->mC4 = readSaturnS32(rotData + 4);
}

// 06061048 — per-subfield dragon light init
static void initPerSubfieldDragonLight_C8(s32 subfieldIndex)
{
    initDragonLightColors_C8(subfieldIndex);
    initDragonLightRotation_C8(subfieldIndex);
}

// 06059824 — check cutscene table for current subfield and start script if found
static s32 startCutsceneFromTable_C8(p_workArea workArea)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    sSaturnPtr tablePtr;
    s32 count;

    if ((mainGameState.bitField[0xA6] & 0x10) == 0)
    {
        tablePtr = gFLD_C8->getSaturnPtr(0x0608E5B0);
        count = 5;
    }
    else
    {
        tablePtr = gFLD_C8->getSaturnPtr(0x0608E600);
        count = 7;
    }

    s16 subfield = pFieldTask->m2E_currentSubFieldIndex;
    for (s32 i = 0; i < count; i++)
    {
        sSaturnPtr entry = tablePtr + i * 16;
        if (readSaturnS32(entry) == subfield)
        {
            s32 scriptId = readSaturnS32(entry + 4);
            s32 scriptParam = readSaturnS32(entry + 8);
            s32 result = startFieldScript(scriptId, scriptParam);
            if (result != 0 && readSaturnS32(entry + 12) != 0)
            {
                // 06073ee8
                getFieldTaskPtr()->m8_pSubFieldData->fieldSubTaskStatus = 1;
            }
            return result;
        }
    }

    return 0;
}

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

    if (LCSTaskDrawSub5Sub1_Data1.empty())
    {
        LCSTaskDrawSub5Sub1_Data1 = readLCSTaskDrawSub5Sub1_Data1({ 0x06093B28, gFLD_A3 });
    }

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
        createFieldSpecificDataTask_C8();
    }

    loadFileFromFileList(1);

    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_C8->getSaturnPtr(0x060B5170));

    // 06061048 — per-subfield dragon light init
    initPerSubfieldDragonLight_C8(getFieldTaskPtr()->m2E_currentSubFieldIndex);

    // Subfield dispatch
    s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    switch (subfield)
    {
    case 0: subfieldC8_0(workArea); break;
    case 1: subfieldC8_1(workArea); break;
    default: Unimplemented(); break; // subfields 2-5
    }

    // m360 update function
    // m360 = FLD_C8::06057ab4 — debug "MAKING MODE" menu (only active in debug builds)
    // 0606ecac is a debug menu system, no-op in non-debug

    // 06059824 — check cutscene table and start if applicable
    s16 subfield2 = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    if (subfield2 != 1 || getFieldTaskPtr()->m30_fieldEntryPoint != 1)
    {
        if (!startCutsceneFromTable_C8(workArea))
        {
            // Start field script if no cutscene
            startFieldScript(subfield2 + 0x50, -1);
        }
    }

    if ((mainGameState.bitField[0xA6] & 0x10) != 0)
    {
        // 06071646
        getFieldTaskPtr()->m28_status |= 0x40000;
    }

    return nullptr;
}
