#include "PDS.h"

void modeDrawFunction6Sub2(u8* pModelDataRoot, u8* pModelData, std::vector<sPoseData>::iterator& pPoseData, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    PDS_unimplemented("modeDrawFunction6Sub2");
}

void modeDrawFunction1Sub2(u8* pModelDataRoot, u8* pModelData, std::vector<sMatrix4x3>::iterator& r5, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7)
{
    assert(0);
}

void LCSItemBox_DrawType0Sub0Sub0(u8* pModelDataRoot, u8* r4, u8** r5)
{
    do
    {
        pushCurrentMatrix();

        ItemBoxModelData temp;
        temp.m0_position[0] = READ_BE_S32(*r5 + 0);
        temp.m0_position[1] = READ_BE_S32(*r5 + 4);
        temp.m0_position[2] = READ_BE_S32(*r5 + 8);

        temp.mC_rotation[0] = READ_BE_S32(*r5 + 0xC);
        temp.mC_rotation[1] = READ_BE_S32(*r5 + 0x10);
        temp.mC_rotation[2] = READ_BE_S32(*r5 + 0x14);

        temp.m18_scale[0] = READ_BE_S32(*r5 + 0x18);
        temp.m18_scale[1] = READ_BE_S32(*r5 + 0x1C);
        temp.m18_scale[2] = READ_BE_S32(*r5 + 0x20);

        translateCurrentMatrix(&temp.m0_position);
        rotateCurrentMatrixZYX(&temp.mC_rotation);

        if (u32 meshOffset = READ_BE_U32(r4))
        {
            addObjectToDrawList(pModelDataRoot, meshOffset);
        }
        if (u32 subMeshOffset = READ_BE_U32(r4 + 4))
        {
            *r5 += 0x24;
            LCSItemBox_DrawType0Sub0Sub0(pModelDataRoot, pModelDataRoot + subMeshOffset, r5);
        }

        popMatrix();

        u32 nextMeshOffset = READ_BE_U32(r4 + 8);

        if (nextMeshOffset == 0)
            break;

        *r5 += 0x24;
        r4 = pModelDataRoot + nextMeshOffset;

    } while (1);
}

void LCSItemBox_DrawType0Sub0(u8* r4, s16 r5, s16 r6)
{
    u8* varC = r4;
    s16 var8 = r5;
    u8* var15 = varC + READ_BE_U32(varC + r6);

    LCSItemBox_DrawType0Sub0Sub0(r4, varC + READ_BE_U32(varC + var8), &var15);
}
