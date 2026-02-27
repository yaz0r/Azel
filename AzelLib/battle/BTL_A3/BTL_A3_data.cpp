#include "PDS.h"
#include "BTL_A3_data.h"
#include "BTL_A3_map3.h"
#include "BTL_A3_map4.h"
#include "BTL_A3_map6.h"
#include "BTL_A3_gnd.h"
#include "BTL_A3_wtr.h"
#include "BTL_A3_BaldorFormation.h"
#include "BTL_A3_UrchinFormation.h"
#include "BTL_A3_BaldorQueenFormation.h"
#include "kernel/grid.h"

struct BTL_A3_data* g_BTL_A3 = nullptr;

sSaturnPtr BTL_A3_data::getEncounterDataTable()
{
    return getSaturnPtr(0x60AAFA0);
}

void BTL_A3_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1)
{
    switch (Func.m_offset)
    {
    case 0x060565da:
        Create_BTL_A3_BaldorFormation(pParent, arg0);
        break;
    case 0x6054DF4:
        Create_BTL_A3_UrchinFormationConfig(pParent, arg0);
        break;
    case 0x06058da4:
        Create_BTL_A3_BaldorQueenFormation(pParent, arg0);
        break;
    default:
        Unimplemented();
        break;
    }
}

void BTL_A3_data::invoke(sSaturnPtr Func, s_workAreaCopy* pParent)
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
    case 0x06054b62: // gnd
        Create_BTL_A3_gnd(pParent);
        break;
    case 0x06054bcc: // wtr
        Create_BTL_A3_wtr(pParent);
        break;
    default:
        Unimplemented();
        break;
    }
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
            pSubEntry->m0_enemyTypeId = readSaturnS8(subEntry + 0);
            pSubEntry->m1_fileBundleIndex = readSaturnS8(subEntry + 1);
            pSubEntry->m2 = readSaturnS8(subEntry + 2);
            pSubEntry->m4 = readSaturnS16(subEntry + 4);
            pSubEntry->m8_modelOffset = readSaturnU16(subEntry + 8);
            pSubEntry->mA_poseOffset = readSaturnU16(subEntry + 0xA);
            pSubEntry->mC_hotspotDefinitions = readRiderDefinitionSub(readSaturnEA(subEntry + 0xC));
            pSubEntry->m18_knockbackStrength = readSaturnS8(subEntry + 0x18);

            sSaturnPtr ptrTo1C = readSaturnEA(subEntry + 0x1C);
            for (int j=0; j<1; j++)
            {
                sGenericFormationPerTypeDataSub1C subData1C;

                subData1C.m0 = readSaturnEA(ptrTo1C + 0);
                for (int k = 0; k < 4; k++)
                {
                    sSaturnPtr attackData = readSaturnEA(ptrTo1C + 0x4 + k * 4);
                    if (attackData.isNull())
                    {
                        subData1C.m4[k] = nullptr;
                    }
                    else
                    {
                        subData1C.m4[k] = new sAttackCommand;

                        subData1C.m4[k]->m4_cameraList = readSaturnEA(attackData + 4);
                        subData1C.m4[k]->m8_type = readSaturnS8(attackData + 8);
                        subData1C.m4[k]->m9_flags = readSaturnU8(attackData + 9);
                        subData1C.m4[k]->mA_attackDisplayName = readSaturnS8(attackData + 0xA);
                    }
                }
                subData1C.m14_quadrantFlags[0] = readSaturnS8(ptrTo1C + 0x14 + 0);
                subData1C.m14_quadrantFlags[1] = readSaturnS8(ptrTo1C + 0x14 + 1);
                subData1C.m14_quadrantFlags[2] = readSaturnS8(ptrTo1C + 0x14 + 2);
                subData1C.m14_quadrantFlags[3] = readSaturnS8(ptrTo1C + 0x14 + 3);
                subData1C.m1C_animationOffset = readSaturnU16(ptrTo1C + 0x1C);
                subData1C.m1E_quadrantAttackDirections = readSaturnS8(ptrTo1C + 0x1E);
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

    sSaturnPtr positionTableTable = readSaturnEA(ptrEA + 0x10);
    do 
    {
        sSaturnPtr positionTable = readSaturnEA(positionTableTable);
        if (!positionTable.isNull())
        {
            sGenericFormationSubData* pNewFormation = new sGenericFormationSubData;
            for (int i = 0; i < pNewData->m0_formationSize; i++)
            {
                pNewFormation->m0_perEnemyPosition.push_back(readSaturnVec3(positionTable + i * 0xC));
            }
            pNewData->m10_formationSubData.push_back(pNewFormation);
        }
        else
        {
            pNewData->m10_formationSubData.push_back(nullptr);
            break;
        }
        positionTableTable += 4;
    } while (1);

    pNewData->m14 = readSaturnS8(ptrEA + 0x14);
    pNewData->m15_formationNameIndex = readSaturnS8(ptrEA + 0x15);
    pNewData->m16 = readSaturnS8(ptrEA + 0x16);
    pNewData->m17 = readSaturnS8(ptrEA + 0x17);

    for (int i = 0; i < 3; i++)
    {
        pNewData->m18_initialDirections[i] = readSaturnS8(ptrEA + 0x18 + i);
    }

    pNewData->m1C = readSaturnS32(ptrEA + 0x1C);

    for (int i = 0; i < 3; i++)
    {
        pNewData->m20_deathSoundEffect[i] = readSaturnS8(ptrEA + 0x20 + i);
    }

    return pNewData;
}

BTL_A3_data::BTL_A3_data() : battleOverlay("BTL_A3.PRG")
{
    m_map3 = readGrid(getSaturnPtr(0x60a5c78), 2, 2);
    m_map4 = readGrid(getSaturnPtr(0x60a605c), 2, 2);
    m_map6 = readGrid(getSaturnPtr(0x60a6698), 2, 2);

    m_60A8AE8_urchinFormation = readUrchinFormation(getSaturnPtr(0x60A8AE8));
    m_60a8ac4_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a8ac4));
    m_60a7d34_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a7d34));
    m_60a7d58_urchinFormation = readUrchinFormation(getSaturnPtr(0x60a7d58));
}

