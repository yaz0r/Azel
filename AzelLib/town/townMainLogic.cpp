#include "PDS.h"
#include "town.h"
#include "townMainLogic.h"
#include "townEdge.h"
#include "townLCS.h"
#include "ruin/twn_ruin.h"


sMainLogic* twnMainLogicTask;

void mainLogicDummy(struct sMainLogic*)
{
}

void mainLogicInitSub2()
{
    initVDP1Projection(DEG_80 / 2, 0);
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
