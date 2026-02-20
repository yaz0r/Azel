#include "PDS.h"
#include "rayDisplay.h"


s32 isGunShotVisible(std::array<sVec3_FP, 2>& param_1, s_graphicEngineStatus_405C& param_2)
{
    Unimplemented();
    return 1;
}


s32 rayComputeDisplayMatrix_fixedWidth(std::array<sVec3_FP, 2>& param_1, s32 param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4)
{
    if (isGunShotVisible(param_1, param_3))
    {
        sVec2_FP local_38;
        fixedPoint ratio0 = FP_Div(0x10000, param_1[0][2]);
        local_38[0] = MTH_Mul_5_6(param_3.m18_widthScale, param_1[0][0], ratio0);
        local_38[1] = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[0][1], ratio0);

        sVec2_FP local_2c;
        fixedPoint ratio1 = FP_Div(0x10000, param_1[1][2]);
        local_2c[0] = MTH_Mul_5_6(param_3.m18_widthScale, param_1[1][0], ratio1);
        local_2c[1] = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[1][1], ratio1);

        s32 angle = atan2(local_38[1] - local_2c[1], local_38[0] - local_2c[0]);

        sVec2_FP iVar9;
        iVar9[0] = MTH_Mul_5_6(param_3.m18_widthScale, MTH_Mul(param_2, getSin(angle)), ratio0);
        iVar9[1] = MTH_Mul_5_6(param_3.m1C_heightScale, MTH_Mul(param_2, getCos(angle)), ratio0);

        param_4[0][0] = local_38[0] - iVar9[0];
        param_4[0][1] = local_38[1] + iVar9[1];
        param_4[3][0] = local_38[0] + iVar9[0];
        param_4[3][1] = local_38[1] - iVar9[1];

        sVec2_FP iVar3;
        iVar3[0] = MTH_Mul_5_6(param_3.m18_widthScale, MTH_Mul(param_2, getSin(angle)), ratio1);
        iVar3[1] = MTH_Mul_5_6(param_3.m1C_heightScale, MTH_Mul(param_2, getCos(angle)), ratio1);

        param_4[1][0] = local_2c[0] - iVar3[0];
        param_4[1][1] = local_2c[1] + iVar3[1];
        param_4[2][0] = local_2c[0] + iVar3[0];
        param_4[2][1] = local_2c[1] - iVar3[1];

        return 1;
    }

    return 0;
}

s32 rayComputeDisplayMatrix_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4)
{
    if (isGunShotVisible(param_1, param_3))
    {
        fixedPoint ratio0 = FP_Div(0x10000, param_1[0][2]);
        fixedPoint iVar2 = MTH_Mul_5_6(param_3.m18_widthScale, param_1[0][0], ratio0);
        fixedPoint iVar3 = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[0][1], ratio0);

        fixedPoint ratio1 = FP_Div(0x10000, param_1[1][2]);
        fixedPoint iVar4 = MTH_Mul_5_6(param_3.m18_widthScale, param_1[1][0], ratio1);
        fixedPoint iVar5 = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[1][1], ratio1);

        fixedPoint angle = atan2(iVar3 - iVar5, iVar2 - iVar4);

        {
            fixedPoint iVar6 = MTH_Mul(param_2[0], getSin(angle));
            fixedPoint iVar7 = MTH_Mul(param_2[0], getCos(angle));

            iVar6 = MTH_Mul_5_6(param_3.m18_widthScale, iVar6, ratio0);
            iVar7 = MTH_Mul_5_6(param_3.m1C_heightScale, iVar7, ratio0);

            param_4[0][0] = iVar2 - iVar6;
            param_4[0][1] = iVar3 + iVar7;
            param_4[3][0] = iVar2 + iVar6;
            param_4[3][1] = iVar3 - iVar7;
        }

        {
            fixedPoint iVar2 = MTH_Mul(param_2[1], getSin(angle));
            fixedPoint iVar3 = MTH_Mul(param_2[1], getCos(angle));

            iVar2 = MTH_Mul_5_6(param_3.m18_widthScale, iVar2, ratio1);
            iVar3 = MTH_Mul_5_6(param_3.m1C_heightScale, iVar3, ratio1);

            param_4[1][0] = iVar4 - iVar2;
            param_4[1][1] = iVar5 + iVar3;
            param_4[2][0] = iVar4 + iVar2;
            param_4[2][1] = iVar5 - iVar3;
        }

        return 1;
    }

    return 0;
}



void sendRaySegmentToVdp1(sScreenQuad3& projectedCoordinates, fixedPoint& depth, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1002; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x484 | colorMode; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = characterColor; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = characterAddress; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = characterSize; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = projectedCoordinates[0][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -projectedCoordinates[0][1]; // CMDYA
    vdp1WriteEA.m10_CMDXB = projectedCoordinates[1][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -projectedCoordinates[1][1]; // CMDYB
    vdp1WriteEA.m14_CMDXC = projectedCoordinates[2][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -projectedCoordinates[2][1]; // CMDYC
    vdp1WriteEA.m18_CMDXD = projectedCoordinates[3][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -projectedCoordinates[3][1]; // CMDYD

    int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
    quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);
    outputColor = *pQuadColor;
    vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

}


void displayRaySegment(std::array<sVec3_FP, 2>& param_1, s32 param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    std::array<sVec3_FP, 2> transformedPoints;
    transformAndAddVecByCurrentMatrix(&param_1[0], &transformedPoints[0]);
    transformAndAddVecByCurrentMatrix(&param_1[1], &transformedPoints[1]);

    sScreenQuad3 screenQuad;

    if (rayComputeDisplayMatrix_fixedWidth(transformedPoints, param_2, graphicEngineStatus.m405C, screenQuad))
    {
        //if (sGunShotTask_DrawSub1Sub1(local_50, graphicEngineStatus.m405C) && sGunShotTask_DrawSub1Sub2(local_50))
        Unimplemented();
        {
            fixedPoint depth = transformedPoints[1][2];
            sendRaySegmentToVdp1(screenQuad, depth, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
        }
    }
}
