#include "PDS.h"
#include "BTL_A3_data.h"
#include "BTL_A3_map3.h"
#include "BTL_A3_map4.h"
#include "BTL_A3_map6.h"
#include "BTL_A3_BaldorFormation.h"
#include "BTL_A3_UrchinFormation.h"
#include "kernel/grid.h"

struct BTL_A3_data* g_BTL_A3 = nullptr;

sSaturnPtr BTL_A3_data::getBattleEngineInitData()
{
    return getSaturnPtr(0x60AAFA0);
}

void BTL_A3_data::invoke(sSaturnPtr Func, p_workArea pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x060565da:
        Create_BTL_A3_BaldorFormation(pParent, arg0);
        break;
    case 0x6054DF4:
        Create_BTL_A3_UrchinFormationConfig(pParent, arg0);
        break;
    default:
        FunctionUnimplemented();
        break;
    }
}

void BTL_A3_data::invoke(sSaturnPtr Func, p_workArea pParent)
{
    switch (Func.m_offset)
    {
    case 0x06054b58: // map6
        Create_BTL_A3_map6(pParent);
        break;
    case 0x06054b44: // map3
        Create_BTL_A3_map3(pParent);
        break;
    case 0x06054b4e: // map4
        Create_BTL_A3_map4(pParent);
        break;
    default:
        FunctionUnimplemented();
        break;
    }
}

sHotpointBundle* readRiderDefinitionSub(sSaturnPtr ptrEA)
{
    if (ptrEA.isNull())
    {
        return nullptr;
    }

    return new sHotpointBundle(ptrEA);
}

sGenericFormationData* readUrchinFormation(sSaturnPtr ptrEA)
{
    sGenericFormationData* pNewData = new sGenericFormationData;

    pNewData->m0_formationSize = readSaturnS8(ptrEA + 0);
    for (int i = 0; i < 3; i++)
    {
        pNewData->m1_perTypeCount[i] = readSaturnS8(ptrEA + 1 + i);
    }

    for (int i = 0; i < 3; i++)
    {
        sSaturnPtr subEntry = readSaturnEA(ptrEA + 4 + i * 4);
        if(subEntry.m_offset)
        {
            sGenericFormationPerTypeData* pSubEntry = new sGenericFormationPerTypeData;
            pSubEntry->m0 = readSaturnS8(subEntry + 0);
            pSubEntry->m1_fileBundleIndex = readSaturnS8(subEntry + 1);
            pSubEntry->m2 = readSaturnS8(subEntry + 2);
            pSubEntry->m4 = readSaturnS16(subEntry + 4);
            pSubEntry->m8_modelOffset = readSaturnU16(subEntry + 8);
            pSubEntry->mA_poseOffset = readSaturnU16(subEntry + 0xA);
            pSubEntry->mC_hotspotDefinitions = readRiderDefinitionSub(readSaturnEA(subEntry + 0xC));
            pSubEntry->m18 = readSaturnS8(subEntry + 0x18);

            sSaturnPtr ptrTo1C = readSaturnEA(subEntry + 0x1C);
            for (int j=0; j<1; j++)
            {
                sGenericFormationPerTypeDataSub1C subData1C;

                subData1C.m0 = readSaturnEA(ptrTo1C + 0);
                subData1C.m4[0] = readSaturnEA(ptrTo1C + 0x4 + 0);
                subData1C.m4[1] = readSaturnEA(ptrTo1C + 0x4 + 4);
                subData1C.m4[2] = readSaturnEA(ptrTo1C + 0x4 + 8);
                subData1C.m4[3] = readSaturnEA(ptrTo1C + 0x4 + 0xC);
                subData1C.m14[0] = readSaturnS8(ptrTo1C + 0x14 + 0);
                subData1C.m14[1] = readSaturnS8(ptrTo1C + 0x14 + 1);
                subData1C.m14[2] = readSaturnS8(ptrTo1C + 0x14 + 2);
                subData1C.m14[3] = readSaturnS8(ptrTo1C + 0x14 + 3);
                subData1C.m1C_animationOffset = readSaturnU16(ptrTo1C + 0x1C);
                subData1C.m1E = readSaturnS8(ptrTo1C + 0x1E);
                pSubEntry->m1C.push_back(subData1C);
            }

            pSubEntry->m24 = readSaturnU32(subEntry + 0x24);
            pSubEntry->m28[0] = readSaturnS8(subEntry + 0x28 + 0);
            pSubEntry->m28[1] = readSaturnS8(subEntry + 0x28 + 1);
            pSubEntry->m28[2] = readSaturnS8(subEntry + 0x28 + 2);
            pSubEntry->m28[3] = readSaturnS8(subEntry + 0x28 + 3);
            pSubEntry->m38 = readSaturnS8(subEntry + 0x38);
            pNewData->m4_perTypeParams[i] = pSubEntry;
        }
        else
        {
            pNewData->m4_perTypeParams[i] = nullptr;
        }
    }

    pNewData->m10_formationSubData = new sGenericFormationSubData;
    for (int i = 0; i < pNewData->m0_formationSize; i++)
    {
        pNewData->m10_formationSubData->m0_perEnemyPosition.push_back(readSaturnVec3(readSaturnEA(readSaturnEA(ptrEA + 0x10)) + i * 0xC));
    }

    pNewData->m14 = readSaturnS8(ptrEA + 0x14);
    pNewData->m15 = readSaturnS8(ptrEA + 0x15);
    pNewData->m16 = readSaturnS8(ptrEA + 0x16);
    pNewData->m17 = readSaturnS8(ptrEA + 0x17);

    for (int i = 0; i < 3; i++)
    {
        pNewData->m18[i] = readSaturnS8(ptrEA + 0x18 + i);
    }

    pNewData->m1C = readSaturnS32(ptrEA + 0x1C);

    for (int i = 0; i < 3; i++)
    {
        pNewData->m20[i] = readSaturnS8(ptrEA + 0x20 + i);
    }

    return pNewData;
}

void BTL_A3_data::init()
{
    m_map3 = readGrid(getSaturnPtr(0x60a5c78), 2, 2);
    m_map4 = readGrid(getSaturnPtr(0x60a605c), 2, 2);
    m_map6 = readGrid(getSaturnPtr(0x60a6698), 2, 2);

    sSaturnPtr laserDataEA = getSaturnPtr(0x60adce4);

    mLaserData.m4 = readSaturnS16(laserDataEA + 0x4);
    mLaserData.m6 = readSaturnS16(laserDataEA + 0x6);
    mLaserData.m8 = readSaturnS16(laserDataEA + 0x8);
    mLaserData.mA = readSaturnS16(laserDataEA + 0xA);
    mLaserData.mC = readSaturnS16(laserDataEA + 0xC);
    mLaserData.mE = readSaturnS16(laserDataEA + 0xE);
    mLaserData.m10 = readSaturnS16(laserDataEA + 0x10);
    mLaserData.m12 = readSaturnS16(laserDataEA + 0x12);
    mLaserData.m14 = readSaturnS16(laserDataEA + 0x14);
    sSaturnPtr laserVerticesEA = readSaturnEA(laserDataEA + 0x18);
    sSaturnPtr laserColorsEA = readSaturnEA(laserDataEA + 0x1C);
    mLaserData.m20_numLaserNodes = readSaturnU32(laserDataEA + 0x20);

    mLaserData.m18_vertices.resize(mLaserData.m20_numLaserNodes);
    mLaserData.m1C_colors.resize(mLaserData.m20_numLaserNodes);

    for (int i=0; i< mLaserData.m20_numLaserNodes; i++)
    {
        mLaserData.m18_vertices[i] = readSaturnFP(laserVerticesEA + 4 * i);
        mLaserData.m1C_colors[i][0] = readSaturnU16(laserColorsEA + 4 * 2 * i + 0);
        mLaserData.m1C_colors[i][1] = readSaturnU16(laserColorsEA + 4 * 2 * i + 2);
        mLaserData.m1C_colors[i][2] = readSaturnU16(laserColorsEA + 4 * 2 * i + 4);
        mLaserData.m1C_colors[i][3] = readSaturnU16(laserColorsEA + 4 * 2 * i + 6);
    }

    m_60A8AE8_urchinFormation = readUrchinFormation(getSaturnPtr(0x60A8AE8));
    m_60a8ac4_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a8ac4));
    m_60a7d34_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a7d34));
    m_60a7d58_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a7d58));
}

