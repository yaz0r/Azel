#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"

void buildDragonRotationMatrix(s_dragonTaskWorkArea_48* r14, sVec3_FP* r13);
void updateCameraScriptSub0(p_workArea r4);

// Camera config functions — each calls setupFieldCameraConfigs with overlay-specific data
// 060558E8
void setupCameraConfig_A5_0() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089C34)), 1); }
// 060558F0
void setupCameraConfig_A5_2() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089C8C)), 2); }
// 060558F8
void setupCameraConfig_A5_4() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089D3C)), 2); }
// 06055900
void setupCameraConfig_A5_7() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089E44)), 1); }
// 06055908
void setupCameraConfig_A5_8() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089E9C)), 1); }
// 06055910
void setupCameraConfig_A5_9() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089EF4)), 1); }
// 06055920
void setupCameraConfig_A5_corridor() { setupFieldCameraConfigs(readCameraConfig(gFLD_A5->getSaturnPtr(0x06089DEC)), 1); }

// 0607c584 — set dragon position + rotation, build rotation matrix, update camera
void setupDragonPositionAndCamera_A5(sSaturnPtr posEA, sSaturnPtr rotEA)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    sFieldCameraStatus* pCam = getFieldCameraStatus();

    if (posEA.m_file != nullptr)
    {
        pDragon->m8_pos[0] = fixedPoint(readSaturnS32(posEA));
        pDragon->m8_pos[1] = fixedPoint(readSaturnS32(posEA + 4));
        pDragon->m8_pos[2] = fixedPoint(readSaturnS32(posEA + 8));
    }

    if (rotEA.m_file != nullptr)
    {
        pDragon->m20_angle[0] = fixedPoint(readSaturnS32(rotEA));
        pDragon->m20_angle[1] = fixedPoint(readSaturnS32(rotEA + 4));
        pDragon->m20_angle[2] = fixedPoint(readSaturnS32(rotEA + 8));

        // 0607fb0c — copy rotation to mBC sub-struct at +0x18
        if (pDragon->mBC != nullptr)
        {
            u8* pBC = (u8*)pDragon->mBC;
            *(s32*)(pBC + 0x18) = readSaturnS32(rotEA);
            *(s32*)(pBC + 0x1C) = readSaturnS32(rotEA + 4);
            *(s32*)(pBC + 0x20) = readSaturnS32(rotEA + 8);
        }
    }

    buildDragonRotationMatrix(&pDragon->m48, &pDragon->m20_angle);

    // Set camera Y rotation from dragon yaw
    pCam->mC_rotation[1] = pDragon->m20_angle[1];

    updateCameraScriptSub0(pDragon->mB8_lightWingEffect);
}

// 06055996 / 06055a1e / 06055a30 — post-battle sound for corridor subfields
void postBattleSound_A5_corridor()
{
    playPCM(getFieldTaskPtr()->m8_pSubFieldData, 0x71);
}

// 06054188 — open field dragon light/speed
void initDragonParams_A5_open()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC8_normalLightColor = { 0x16, 0x16, 0x13 };
    p->mCB_falloffColor0 = { 6, 6, 8 };
    p->mCE_falloffColor1 = { 0xF, 0xF, 0xB };
    p->mD1_falloffColor2 = { 0xF, 0xF, 0xB };
    p->mD4 = { 0x10, 0x10, 0x10 };
    p->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
    p->mC4 = 0;
}

// 060541F0 — corridor dragon light/speed (depends on bitfield)
void initDragonParams_A5_corridor()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if ((mainGameState.bitField[0xA3] & 0x20) == 0)
    {
        p->mC8_normalLightColor = { 0x10, 0x10, 0x10 };
        p->mCB_falloffColor0 = { 8, 8, 8 };
        p->mCE_falloffColor1 = { (s8)0xFD, (s8)0xFA, (s8)0xF7 };
        p->mD1_falloffColor2 = { 0x14, 0x14, 0x14 };
        p->mC0_lightRotationAroundDragon = fixedPoint(0xE5B05B0);
        p->mC4 = fixedPoint(0x9555555);
    }
    else
    {
        p->mC8_normalLightColor = { 9, 10, 12 };
        p->mCB_falloffColor0 = { 6, 7, 12 };
        p->mCE_falloffColor1 = { 4, 4, 5 };
        p->mD1_falloffColor2 = { 4, 4, 5 };
        p->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
        p->mC4 = 0;
    }
}

// 060542BC — subfield B dragon params
void initDragonParams_A5_B()
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC8_normalLightColor = { 5, 6, 9 };
    p->mCB_falloffColor0 = { 1, 2, 8 };
    p->mCE_falloffColor1 = { 2, 2, 2 };
    p->mD1_falloffColor2 = { 3, 3, 4 };
    p->mD4 = { 0xD, 0xD, 0xF };
    p->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
    p->mC4 = 0;
    g_fadeControls.m_4A = 0x8000 | ((s16)p->mD4.m2 << 10) | ((s16)p->mD4.m1 << 5) | (s16)p->mD4.m0;
}
