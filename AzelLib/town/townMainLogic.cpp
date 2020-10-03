#include "PDS.h"
#include "town.h"
#include "townMainLogic.h"
#include "townEdge.h"
#include "townLCS.h"
#include "ruin/twn_ruin.h"
#include "kernel/debug/trace.h"
#include "townScript.h"

sMainLogic* twnMainLogicTask;

void mainLogicDummy(struct sMainLogic*)
{
}

void mainLogicInitSub2()
{
    initVDP1Projection(DEG_80 / 2, 0);
}

// read inputs
void mainLogicUpdateSub1(sMainLogic* pThis)
{
    if (!(npcData0.mFC & 8))
    {
        if (npcData0.mFC & 0x10)
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1)
            {
                pThis->m4_flags |= 0x4000000;
            }
        }
        else
        {
            //605578C
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][2])
            {
                pThis->m4_flags |= 0x8000000;
            }
        }
    }

    //0x60557A0
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][13])
    {
        pThis->m4_flags |= 0x20000000;
    }

    s32 r6 = 0;
    s32 r7 = 0;
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
    {
        //60557BA
        r6 = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX * 512;
        if (graphicEngineStatus.m4514.m138[0]) // inverse axis
        {
            r7 = -graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY * 512;
        }
        else
        {
            r7 = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY * 512;
        }
    }
    else
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][4])
        {
            r6 = 0;
            r7 = 0x10000;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][5])
        {
            r6 = 0;
            r7 = -0x10000;
        }

        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][6])
        {
            r6 = 0x10000;
            r7 = 0;
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][7])
        {
            r6 = -0x10000;
            r7 = -0;
        }
    }

    //6055820
    pThis->m8_inputX = r6;
    pThis->mC_inputY = r7;
}

void mainLogicUpdateSub2(sMainLogic* pThis)
{
    if (pThis->m4_flags & 0x4000000)
    {
        // cancel LCS
        npcData0.mFC &= ~0x12;
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
    }
    else if ((pThis->m4_flags & 0x8000000) && !(npcData0.mFC & 0x10))
    {
        // enter LCS
        npcData0.mFC |= 0x12;
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        LCSCollisionData.m4_LCS_Y = 0;
        LCSCollisionData.m0_LCS_X = 0;
        pThis->m3 = 0;
        pThis->m30 = pThis->m68_cameraRotation[1];
    }

    //60558AE
    if (pThis->m4_flags & 0x10000000)
    {
        assert(0);
    }

    if (pThis->m4_flags & 0x40000000)
    {
        pThis->m0 ^= 1;
    }
}

void mainLogicUpdateSub4(sMainLogic* pThis)
{
    if (pThis->m14_EdgeTask)
    {
        sNPCE8* r5 = &pThis->m14_EdgeTask->mE8;
        if (pThis->m118_autoWalkDuration)
        {
            if (--pThis->m118_autoWalkDuration == 0)
            {
                pThis->m14_EdgeTask->m84.m0_collisionSetup.m0_collisionType = 0;
            }

            pThis->m11C_autoWalkStartPosition += pThis->m134_autoWalkPositionStep;
            pThis->m14_EdgeTask->mE8.m0_position = pThis->m11C_autoWalkStartPosition;
            pThis->m14_EdgeTask->mE8.mC_rotation = pThis->m128_autoWalkStartRotation;
        }
    }
}



void scriptFunction_6057058_sub0Sub0()
{
    sNPCE8* r13 = &twnMainLogicTask->m14_EdgeTask->mE8;
    twnMainLogicTask->m18_position = r13->m0_position + sVec3_FP(0, 0x1800, 0);

    sMatrix4x3 var0;
    initMatrixToIdentity(&var0);
    translateMatrix(twnMainLogicTask->m18_position, &var0);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var0);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var0);

    twnMainLogicTask->m5C_rawCameraPosition[0] = var0.matrix[3] + MTH_Mul(var0.matrix[2], 0x199);
    twnMainLogicTask->m5C_rawCameraPosition[1] = var0.matrix[7] + MTH_Mul(var0.matrix[6], 0x199);
    twnMainLogicTask->m5C_rawCameraPosition[2] = var0.matrix[11] + MTH_Mul(var0.matrix[10], 0x199);

    twnMainLogicTask->m38_interpolatedCameraPosition = twnMainLogicTask->m5C_rawCameraPosition;
    twnMainLogicTask->m50_upVector = twnMainLogicTask->m5C_rawCameraPosition;
    twnMainLogicTask->m50_upVector[1] += 0x10000;

    twnMainLogicTask->m44_cameraTarget[0] = var0.matrix[3] + MTH_Mul(var0.matrix[2], -0x1000);
    twnMainLogicTask->m44_cameraTarget[1] = var0.matrix[7] + MTH_Mul(var0.matrix[6], -0x1000);
    twnMainLogicTask->m44_cameraTarget[2] = var0.matrix[11] + MTH_Mul(var0.matrix[10], -0x1000);
}

void moveTownLCSCursor(sMainLogic* r4)
{
    LCSCollisionData.m0_LCS_X += MTH_Mul(0x50000, -r4->m8_inputX);
    if (LCSCollisionData.m0_LCS_X > 0x7428F5)
    {
        LCSCollisionData.m0_LCS_X = 0x7428F5;
    }
    if (LCSCollisionData.m0_LCS_X < -0x7428F5)
    {
        LCSCollisionData.m0_LCS_X = -0x7428F5;
    }

    LCSCollisionData.m4_LCS_Y += MTH_Mul(0x50000, r4->mC_inputY);
    if (LCSCollisionData.m4_LCS_Y > 0x49EB85)
    {
        LCSCollisionData.m4_LCS_Y = 0x49EB85;
    }
    if (LCSCollisionData.m4_LCS_Y < -0x49EB85)
    {
        LCSCollisionData.m4_LCS_Y = -0x49EB85;
    }
}

const std::array<sVec2_FP, 2> cameraParams = {
    {
        {0x2CCC, 0xAAAAAA},
        {0x2CCC, -0x555555},
    }
};

void cameraFollowMode0_LCSSub1(sMainLogic* r4)
{
    sVec3_FP* r13 = &r4->m18_position;
    if ((npcData0.mFC & 1) && (currentResTask->m8_currentLCSType))
    {
        //assert(0);
        /*
        if (currentResTask->m8 == 1)
        {
            r13 = cameraFollowMode0_LCSSub1Sub0(currentResTask->mC);
        }
        else
        {
            r13 = currentResTask->mC_AsOffset + 8;
        }
        */
        //6055C48
        //assert(0);
        Unimplemented();
    }

    //6055CCA
    r4->m44_cameraTarget[0] = MTH_Mul(r4->m44_cameraTarget[0] - (*r13)[0], 0xF333) + (*r13)[0];
    r4->m44_cameraTarget[1] = MTH_Mul(r4->m44_cameraTarget[1] - (*r13)[1], 0xF333) + (*r13)[1];
    r4->m44_cameraTarget[2] = MTH_Mul(r4->m44_cameraTarget[2] - (*r13)[2], 0xF333) + (*r13)[2];
}

void cameraFollowMode0_LCS(sMainLogic* r14_pose)
{
    sNPCE8* r12_npcData = &r14_pose->m14_EdgeTask->mE8;

    r14_pose->m68_cameraRotation[0] += MTH_Mul(0xE38E3, r14_pose->mC_inputY);
    if (r14_pose->m68_cameraRotation[0] > 0x13E93E9)
    {
        r14_pose->m68_cameraRotation[0] = 0x13E93E9;
    }
    if (r14_pose->m68_cameraRotation[0] < -0x13E93E9)
    {
        r14_pose->m68_cameraRotation[0] = -0x13E93E9;
    }

    r14_pose->m30 += MTH_Mul(0xE38E3, r14_pose->m8_inputX);

    if (!(npcData0.mFC & 1))
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][11])
        {
            r14_pose->m30 = ((r14_pose->m68_cameraRotation[1] - r12_npcData->mC_rotation[1] + 0x5000000) & 0xC000000) + r12_npcData->mC_rotation[1];
        }
        else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][15])
        {
            //060561D4
            r14_pose->m30 = ((r14_pose->m68_cameraRotation[1] - r12_npcData->mC_rotation[1] - 0x1000000) & 0xC000000) + r12_npcData->mC_rotation[1];
        }

    }
    //60561E6
    r14_pose->m68_cameraRotation[1] += MTH_Mul(fixedPoint(r14_pose->m30 - r14_pose->m68_cameraRotation[1]).normalized(), 0x1999);
    moveTownLCSCursor(r14_pose);
    r14_pose->m24_distance = vecDistance(r14_pose->m18_position, r14_pose->m5C_rawCameraPosition);
    fixedPoint r4 = MTH_Mul(cameraParams[r14_pose->m1_cameraParamsIndex][0] - r14_pose->m24_distance, 0x1999);
    if (r4 > 0x599)
    {
        r4 = 0x599;
    }
    if (r4 < -0x599)
    {
        r4 = -0x599;
    }
    r14_pose->m24_distance += r4;

    sMatrix4x3 var4;
    initMatrixToIdentity(&var4);
    rotateMatrixShiftedY(r14_pose->m68_cameraRotation[1], &var4);
    rotateMatrixShiftedX(r14_pose->m68_cameraRotation[0], &var4);
    scaleMatrixRow2(r14_pose->m24_distance, &var4);

    r14_pose->m38_interpolatedCameraPosition[0] = var4.matrix[2] + r14_pose->m18_position[0];
    r14_pose->m38_interpolatedCameraPosition[1] = var4.matrix[6] + r14_pose->m18_position[1];
    r14_pose->m38_interpolatedCameraPosition[2] = var4.matrix[10] + r14_pose->m18_position[2];

    r14_pose->m5C_rawCameraPosition = r14_pose->m38_interpolatedCameraPosition;

    cameraFollowMode0_LCSSub1(r14_pose);
}

void updateCameraTarget(sMainLogic* r4, const sVec3_FP& r14_pose)
{
    // project the point to screen

    sVec3_FP var8;
    var8[0] = transformByMatrixRow0(r14_pose);
    var8[1] = transformByMatrixRow1(r14_pose);
    var8[2] = transformByMatrixRow2(r14_pose);

    sVec2_FP var0;
    var0[0] = FP_Div(var8[0], var8[2]);
    var0[1] = FP_Div(var8[1], var8[2]);

    if (var0[0] < 0)
    {
        var0[0] = -var0[0];
    }

    if (var0[1] < 0)
    {
        var0[1] = -var0[1];
    }

    // If the projected target point is already at the center of the screen
    if ((MTH_Mul(graphicEngineStatus.m405C.m18_widthScale, var0[0]) == 0) && (MTH_Mul(graphicEngineStatus.m405C.m1C_heightScale, var0[1]) == 0))
    {
        r4->m44_cameraTarget = r14_pose;
        return;
    }

    // If not, interpolate
    if (var0[0] > 0x800000)
        var0[0] = 0x800000;

    if (var0[1] > 0x800000)
        var0[1] = 0x800000;

    fixedPoint r13 = MulVec2(var0, var0) / 2;

    if (r13 > 0xB333)
        r13 = 0xB333;

    if (r13 < 0xCCC)
        r13 = 0xCCC;

    r13 = 0x10000 - r13;

    r4->m44_cameraTarget[0] = r14_pose[0] + MTH_Mul(r4->m44_cameraTarget[0] - r14_pose[0], r13);
    r4->m44_cameraTarget[1] = r14_pose[1] + MTH_Mul(r4->m44_cameraTarget[1] - r14_pose[1], r13);
    r4->m44_cameraTarget[2] = r14_pose[2] + MTH_Mul(r4->m44_cameraTarget[2] - r14_pose[2], r13);
}

void mainLogicUpdateSub5(sMainLogic* r4)
{
    sMatrix4x3 varC;

    const sNPCE8& r13 = r4->m14_EdgeTask->mE8;

    initMatrixToIdentity(&varC);
    rotateMatrixShiftedY(r13.m24_stepRotation[1] + r13.mC_rotation[1], &varC);
    rotateMatrixShiftedX(r13.m24_stepRotation[0] + r13.mC_rotation[0], &varC);
    scaleMatrixRow2(-r4->m24_distance / 2, &varC);

    sVec3_FP var0;
    var0[0] = varC.matrix[2] + r4->m18_position[0];
    var0[1] = varC.matrix[6] + r4->m18_position[1];
    var0[2] = varC.matrix[10] + r4->m18_position[2];

    updateCameraTarget(r4, var0);
}

void cameraFollowMode0Bis(sMainLogic* r14_townTask)
{
    sEdgeTask* r4_edge = r14_townTask->m14_EdgeTask;
    sNPCE8* r13_npcData = &r4_edge->mE8;

    if ((r4_edge->mC & 4) == 0)
    {
        r14_townTask->m2C = 0;
        r14_townTask->m30 = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][11])
        {
            r14_townTask->m30 = 0x4000000;
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][15])
            {
                r14_townTask->m30 = 0x8000000;
            }
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][15])
            {
                r14_townTask->m30 = 0xC000000;
            }
        }
    }

    //6055E90
    const sVec2_FP& cameraParam = cameraParams[r14_townTask->m1_cameraParamsIndex];
    if ((MTH_Mul(0x151EB, cameraParam[0]) < r14_townTask->m24_distance) && !(r14_townTask->m4_flags & 0x10000))
    {
        if (!(r4_edge->mC & 4))
        {
            //06055EBE
            (*r14_townTask->m74_townCamera.m30_pPosition)[0] += r14_townTask->m74_townCamera.m58_collisionSolveTranslation[0];
        }

        (*r14_townTask->m74_townCamera.m30_pPosition)[1] += r14_townTask->m74_townCamera.m58_collisionSolveTranslation[1];
        (*r14_townTask->m74_townCamera.m30_pPosition)[2] += r14_townTask->m74_townCamera.m58_collisionSolveTranslation[2];
    }

    //6055EE4
    sVec3_FP var4 = r14_townTask->m18_position - r14_townTask->m5C_rawCameraPosition;
    r14_townTask->m24_distance = sqrt_F(MTH_Product3d_FP(var4, var4));

    computeVectorAngles(var4, r14_townTask->m68_cameraRotation);

    s32 r4 = atan2_FP(0x147, r14_townTask->m24_distance);
    if (r4 > 0x1555555)
    {
        r4 = 0x1555555;
    }

    fixedPoint r5 = cameraParam[1] - r14_townTask->m68_cameraRotation[0];
    fixedPoint r6 = r5.normalized();

    r5 = -r4;
    if (r6 > r4)
    {
        r6 = r4;
    }

    if (r6 < r5)
    {
        r6 = r5;
    }

    r14_townTask->m68_cameraRotation[0] += r6;

    fixedPoint r3 = r14_townTask->m68_cameraRotation[0];
    if (r3 > 0x13E93E9)
    {
        r14_townTask->m68_cameraRotation[0] = 0x13E93E9;
    }
    if (r3 < -0x13E93E9)
    {
        r14_townTask->m68_cameraRotation[0] = -0x13E93E9;
    }

    r6 = r13_npcData->mC_rotation[1] + r14_townTask->m30 - r14_townTask->m68_cameraRotation[1];
    r6 = r6.normalized();

    if (((r6 < 0x71C71C7) && (r6 > -0x71C71C7)) || (r13_npcData->m54_oldPosition == r13_npcData->m0_position))
    {
        // 6055FFA
        if (r6 > r4)
            r6 = r4;
        if (r6 < r5)
            r6 = r5;

        r14_townTask->m68_cameraRotation[1] += r6;
    }

    // 605600E
    r4 = MTH_Mul(cameraParam[0] - r14_townTask->m24_distance, 0x3333);
    if (r4 > 0x599)
        r4 = 0x599;
    if (r4 < -0x599)
        r4 = -0x599;

    r14_townTask->m24_distance += r4;

    fixedPoint r13;
    if (r14_townTask->m24_distance >= cameraParam[0])
    {
        r13 = 0xCCCC;
    }
    else
    {
        r13 = setDividend(r14_townTask->m24_distance - 0x1000, 0xCCCC, cameraParam[0] - 0x1000);
        if (r13 < 0)
            r13 = 0;
    }

    //6056078
    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r14_townTask->m68_cameraRotation[1], &var10);
    rotateMatrixShiftedX(r14_townTask->m68_cameraRotation[0], &var10);
    scaleMatrixRow2(r14_townTask->m24_distance, &var10);

    r14_townTask->m5C_rawCameraPosition[0] = var10.matrix[2] + r14_townTask->m18_position[0];
    r14_townTask->m5C_rawCameraPosition[1] = var10.matrix[6] + r14_townTask->m18_position[1];
    r14_townTask->m5C_rawCameraPosition[2] = var10.matrix[10] + r14_townTask->m18_position[2];

    r14_townTask->m38_interpolatedCameraPosition[0] = r14_townTask->m5C_rawCameraPosition[0] + MTH_Mul(r14_townTask->m38_interpolatedCameraPosition[0] - r14_townTask->m5C_rawCameraPosition[0], r13);
    r14_townTask->m38_interpolatedCameraPosition[1] = r14_townTask->m5C_rawCameraPosition[1] + MTH_Mul(r14_townTask->m38_interpolatedCameraPosition[1] - r14_townTask->m5C_rawCameraPosition[1], r13);
    r14_townTask->m38_interpolatedCameraPosition[2] = r14_townTask->m5C_rawCameraPosition[2] + MTH_Mul(r14_townTask->m38_interpolatedCameraPosition[2] - r14_townTask->m5C_rawCameraPosition[2], r13);

    EdgeUpdateSub0(&r14_townTask->m74_townCamera);
    mainLogicUpdateSub5(r14_townTask);

    addTraceLog("cameraFollowMode0Bis set camera position: 0x%04X 0x%04X 0x%04X\n", r14_townTask->m38_interpolatedCameraPosition[0].asS32(), r14_townTask->m38_interpolatedCameraPosition[1].asS32(), r14_townTask->m38_interpolatedCameraPosition[2].asS32());
}

void mainLogicUpdateSub3()
{
    if (twnMainLogicTask->m14_EdgeTask == nullptr)
        return;

    s32 r5_inLcsMode = npcData0.mFC & 0x10;

    switch (twnMainLogicTask->m2_cameraFollowMode)
    {
    case 0:
        if (r5_inLcsMode)
        {
            twnMainLogicTask->m10 = &cameraFollowMode0_LCS;
        }
        else
        {
            twnMainLogicTask->m10 = &cameraFollowMode0Bis;
        }
        break;
    default:
        assert(0);
        break;
    }
}

int scriptFunction_6057058_sub0()
{
    twnMainLogicTask->m_DrawMethod = sMainLogic::Draw;
    scriptFunction_6057058_sub0Sub0();
    twnMainLogicTask->m2_cameraFollowMode = 0;
    mainLogicUpdateSub3();

    return 0; // dummy
}

s32 hasLoadingCompleted()
{
    Unimplemented();
    return 1;
}

void sMainLogic::Init(sMainLogic* pThis)
{
    twnMainLogicTask = pThis;

    if (mainGameState.getBit(0x274 * 8 + 7))
    {
        pThis->m1_cameraParamsIndex = 1;
    }
    else
    {
        pThis->m1_cameraParamsIndex = 0;
    }
    pThis->m0 = 0;
    pThis->m10 = &mainLogicDummy;

    resetMatrixStack();

    pThis->m74_townCamera.m30_pPosition = &pThis->m5C_rawCameraPosition;
    pThis->m74_townCamera.m34_pRotation = &pThis->m68_cameraRotation;
    pThis->m74_townCamera.m38_pOwner = pThis;
    pThis->m74_townCamera.m3C_scriptEA = sSaturnPtr::getNull();
    pThis->m74_townCamera.m40 = 0;

    mainLogicInitSub0(&pThis->m74_townCamera, 0);
    mainLogicInitSub1(&pThis->m74_townCamera, readSaturnVec3(gTWN_RUIN->getSaturnPtr(0x605EEE4)), readSaturnVec3(gTWN_RUIN->getSaturnPtr(0x605EEF0)));

    npcData0.mFC &= ~0x10;

    mainLogicInitSub2();
}

void sMainLogic::Update(sMainLogic* pThis)
{
    if (pThis->m14_EdgeTask)
    {
        sEdgeTask* r13 = pThis->m14_EdgeTask;
        mainLogicUpdateSub0(r13->mE8.m0_position[0], r13->mE8.m0_position[2]);
        pThis->m18_position = r13->mE8.m0_position;
        pThis->m18_position[1] += 0x1800;
    }

    mainLogicUpdateSub1(pThis);

    mainLogicUpdateSub2(pThis);

    if (!(npcData0.mFC & 1))
    {
        mainLogicUpdateSub3();
    }

    pThis->m10(pThis);

    // Hack
#ifndef SHIPPING_BUILD
    {
        static bool forceCameraPosition = false;
        ImGui::Begin("Town");
        {
            ImGui::Checkbox("Force camera position", &forceCameraPosition);
            Imgui_Vec3FP("Camera position", &pThis->m38_interpolatedCameraPosition);
            Imgui_Vec3FP("Camera target", &pThis->m44_cameraTarget);
        }
        ImGui::End();

        if (forceCameraPosition && pThis->m14_EdgeTask)
        {
            pThis->m38_interpolatedCameraPosition = pThis->m14_EdgeTask->mE8.m0_position - sVec3_FP(0, 0, 0x5000);
            pThis->m44_cameraTarget = pThis->m14_EdgeTask->mE8.m0_position;
        }
    }
#endif

    pThis->m50_upVector = pThis->m38_interpolatedCameraPosition;
    pThis->m50_upVector[1] += 0x10000;
    pThis->m4_flags = 0;

    mainLogicUpdateSub4(pThis);
}

void sMainLogic::Draw(sMainLogic* pThis)
{
    sVec3_FP var18 = pThis->m38_interpolatedCameraPosition + ((pThis->m44_cameraTarget - pThis->m38_interpolatedCameraPosition) * 16);

    generateCameraMatrix(&cameraProperties2, pThis->m38_interpolatedCameraPosition, var18, pThis->m50_upVector);

    drawLcs();

    if (enableDebugTask)
    {
        assert(0);
    }
}

p_workArea startMainLogic(p_workArea pParent)
{
    return createSubTask<sMainLogic>(pParent);
}
