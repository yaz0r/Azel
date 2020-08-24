#pragma once

struct ItemBoxModelData
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    sVec3_FP m18_scale;
    // size 0x24
};

void modeDrawFunction6Sub2(struct sModelHierarchy* pModelData, std::vector<sPoseData>::iterator& pPoseData, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7);
void modeDrawFunction1Sub2(struct sModelHierarchy* pModelData, std::vector<sMatrix4x3>::iterator& r5, std::vector<s_hotpointDefinition>::iterator& r6, std::vector<std::vector<sVec3_FP>>::iterator& r7);
void LCSItemBox_DrawType0Sub0(struct s_fileBundle* r4, s16 r5, s16 r6);

bool model_initHotpointBundle(s_3dModel* pDragonStateData1, sHotpointBundle* unkArg);
sHotpointBundle* readRiderDefinitionSub(sSaturnPtr ptrEA);


