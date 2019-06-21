#pragma once

struct ItemBoxModelData
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    sVec3_FP m18_scale;
    // size 0x24
};

void modeDrawFunction6Sub2(u8* pModelDataRoot, u8* pModelData, std::vector<sPoseData>::iterator& pPoseData, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7);
void modeDrawFunction1Sub2(u8* pModelDataRoot, u8* pModelData, std::vector<sMatrix4x3>::iterator& r5, const s_RiderDefinitionSub*& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7);
void LCSItemBox_DrawType0Sub0(u8* r4, s16 r5, s16 r6);
u32 stepAnimation(s_3dModel* p3DModel);
void interpolateAnimation(s_3dModel* p3dModel);


