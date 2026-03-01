#include "PDS.h"
#include "twn_ruin.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townLCS.h"
#include "town/townMainLogic.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"
#include "town/townCamera.h"

#include "twn_ruin_lock.h"

sTownObject* TWN_RUIN_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    assert(definitionEA.m_file == this);
    assert(arg.m_file == this);

    switch (definitionEA.m_offset)
    {
    case 0x605EA20:
        return createSubTaskWithArgWithCopy<sLockTask>(parent, arg);
    default:
        assert(0);
        break;
    }
    return nullptr;
}

sTownObject* TWN_RUIN_data::createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    return nullptr;
}

TWN_RUIN_data* gTWN_RUIN = NULL;

const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "RUINMP.MCB",
    "RUINMP.CGB",
    nullptr
};

struct sRuinBackgroundTask : public s_workAreaTemplate<sRuinBackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sRuinBackgroundTask::Init, &sRuinBackgroundTask::Update, &sRuinBackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sRuinBackgroundTask* pThis)
    {
        reinitVdp2();
        Unimplemented();
    }

    static void Update(sRuinBackgroundTask* pThis)
    {
        Unimplemented();
    }

    static void Draw(sRuinBackgroundTask* pThis)
    {
        Unimplemented();
    }

    // size 0x9C
};

void startRuinBackgroundTask(p_workArea pThis)
{
    createSubTask<sRuinBackgroundTask>(pThis);
}

void registerNpcs(const std::vector<const sTownSetup*>& townSetups, sSaturnPtr r5_script, s32 r6)
{
    npcData0.m0_numBackgroundScripts = 0;
    npcData0.m5E = -1;
    npcData0.m60_townSetup = &townSetups;
    npcData0.mFC = 0;
    npcData0.m100 = 0;
    npcData0.m11C_currentStackPointer = npcData0.m120_stack.end();
    npcData0.m164_cinematicBars = 0;
    npcData0.m168 = 0;
    npcData0.m16C_displayStringTask = 0;
    npcData0.m170_multiChoiceTask = 0;

    for (int i = 0; i < npcData0.m70_npcPointerArray.size(); i++)
    {
        npcData0.m70_npcPointerArray[i].workArea = nullptr;
        npcData0.m70_npcPointerArray[i].pNPC = nullptr;
    }

    townVar0 = nullptr;

    npcData0.mFC |= 0xF;

    npcData0.m104_currentScript.m0_scriptPtr = r5_script;
    npcData0.m104_currentScript.m4 = 0;
    npcData0.m104_currentScript.m8_owner.reset();

    npcData0.m118_currentResult = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

//(0 cursor, 1 near, 2 far)
void drawLcsSprite(const sVec2_S16& r4, s32 r5_index)
{
    sSaturnPtr r5_spriteData = gTWN_RUIN->getSaturnPtr(0x605EEFC + r5_index * 0x1C);
    s32 r6 = dramAllocatorEnd[0].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
    s32 var4 = readSaturnU16(r5_spriteData + 6) + r6;
    s32 var0 = readSaturnU16(r5_spriteData + 0xA) + r6;
    s32 var8 = readSaturnS32(r5_spriteData + 0xC) >> 12;
    s32 varC = readSaturnS32(r5_spriteData + 0x10) >> 12;

    sVec2_S16 var10;
    var10[0] = (readSaturnS32(r5_spriteData + 0x14) >> 12) + r4[0];
    var10[1] = (readSaturnS32(r5_spriteData + 0x18) >> 12) + r4[1];

    sVec2_S16 var14;
    var14[0] = var10[0] + var8;
    var14[1] = var10[1] - varC;

    //////////
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = readSaturnU16(r5_spriteData + 2); // command
    vdp1WriteEA.m4_CMDPMOD = readSaturnU16(r5_spriteData + 4); // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = var0; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = var4; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = readSaturnU16(r5_spriteData + 8); // CMDSIZE
    vdp1WriteEA.mC_CMDXA = var10[0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -var10[1]; // CMDYA
    vdp1WriteEA.m14_CMDXC = var14[0];
    vdp1WriteEA.m16_CMDYC = -var14[1];

    s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
    pExtendedCommand->depth = 0;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

bool canCurrentResActivate()
{
    switch (currentResTask->m8_currentLCSType)
    {
    case 1:
        if (currentResTask->m10_distanceToLCS > npcData0.m54_activationNear)
            return false;
        else
            return true;
    case 2:
        if (currentResTask->m10_distanceToLCS > npcData0.m58_activationFar)
            return false;
        else
            return true;
    default:
        return false;
        break;
    }
}

void drawLcs()
{
    if ((npcData0.mFC & 0x10) && !(npcData0.mFC & 0x8))
    {
        sVec2_S16 var0;
        var0[0] = LCSCollisionData.m0_LCS_X.getInteger();
        var0[1] = LCSCollisionData.m4_LCS_Y.getInteger();

        drawLcsSprite(var0, 0);

        if (currentResTask->m8_currentLCSType)
        {
            //6056C6C
            if (canCurrentResActivate())
            {
                drawLcsSprite(currentResTask->m14_LCS, 1);
            }
            else
            {
                drawLcsSprite(currentResTask->m14_LCS, 2);
            }
        }
    }
    else
    {
        if (enableDebugTask)
        {
            assert(0);
        }
    }
}

// TODO kernel
const sVec3_FP* cameraFollowMode0_LCSSub1Sub0(s32 r4)
{
    if (r4 >= npcData0.m68_numEnvLCSTargets)
    {
        assert(0);
        return nullptr;
    }
    else
    {
        return &(*npcData0.m6C_LCSTargets)[r4];
    }
}


s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    gTWN_RUIN->makeCurrent();

    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    initVdp1Ram(pThis, 0x25C18800, 0x63800);

    registerNpcs(gTWN_RUIN->mTownSetups, gTWN_RUIN->getSaturnPtr(0x06054398), arg);

    startScriptTask(pThis);

    startRuinBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_RUIN->getSaturnPtr(0x605E990));

    npcData0.m160_pEdgePosition = &pEdgeTask->m84.m8_position;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}

s32 isObjectCloseEnoughToActivate()
{
    return !canCurrentResActivate();
}

void setupNPCWalkInZDirection(s32 r4_npcIndex, s32 r5_zDirection, s32 r6_distance)
{
    sNPC* r13 = getNpcDataByIndex(r4_npcIndex);
    sNPCE8* r14 = &r13->mE8;
    r14->m30_stepTranslation[0] = 0;
    r14->m30_stepTranslation[1] = 0;
    r14->m30_stepTranslation[2] = -r5_zDirection;

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r14->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r14->mC_rotation[0], &var10);
    sVec3_FP var4;
    transformVec(r14->m30_stepTranslation, var4, var10);

    r14->m3C_targetPosition[0] = r14->m0_position[0] + var4[0] * r6_distance;
    r14->m3C_targetPosition[1] = r14->m0_position[1] + var4[1] * r6_distance;
    r14->m3C_targetPosition[2] = r14->m0_position[2] + var4[2] * r6_distance;

    r14->m48_targetRotation = r14->mC_rotation;

    r13->mF &= ~0x6;
    r13->mF |= 1;
    r13->mC |= 4;
}

s32 scriptFunction_6057058()
{
    scriptFunction_6057058_sub0();
    setupNPCWalkInZDirection(0, 227, 36);

    return 0;
}

s32 scriptFunction_605762A()
{
    if (twnMainLogicTask->m14_EdgeTask == nullptr)
    {
        return 0;
    }

    if (twnMainLogicTask->m118_autoWalkDuration)
    {
        return 0;
    }

    sNPCE8& r5 =twnMainLogicTask->m14_EdgeTask->mE8;

    twnMainLogicTask->m11C_autoWalkStartPosition = r5.m0_position;
    twnMainLogicTask->m128_autoWalkStartRotation = r5.mC_rotation;

    sVec3_FP var0 = npcData0.m104_currentScript.mC.toSVec3_FP();

    twnMainLogicTask->m134_autoWalkPositionStep[0] = MTH_Mul(0x199, var0[0]);
    twnMainLogicTask->m134_autoWalkPositionStep[1] = MTH_Mul(0x199, var0[1]);
    twnMainLogicTask->m134_autoWalkPositionStep[2] = MTH_Mul(0x199, var0[2]);

    twnMainLogicTask->m118_autoWalkDuration = 5;
    twnMainLogicTask->m14_EdgeTask->m84.m0_collisionSetup.m0_collisionType = 1;

    return 1;
}

void updateEdgeControls(sEdgeTask* r4)
{
    s32 r5 = 0;
    s32 r6 = 0;
    if (npcData0.mFC & 2)
    {
        r4->m14C_inputFlags |= 0xC0;
    }
    else
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //0605BC5E
            r5 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
            r6 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x10)
            {
                r6 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x20)
            {
                r6 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x40)
            {
                r5 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x80)
            {
                r5 = -0x10000;
            }
        }
    }

    r4->m150_inputX = r5;
    r4->m154_inputY = r6;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][0])
    {
        r4->m14C_inputFlags |= 2;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][1])
    {
        r4->m14C_inputFlags |= 1;
    }
}

void updateEdgePositionSub1(sEdgeTask* r4)
{
    if (!(r4->m14C_inputFlags & 0x40))
    {
        switch (twnMainLogicTask->m0)
        {
        case 0:
            if (r4->m14C_inputFlags & 2)
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x555555, r4->m150_inputX);
            }
            else
            {
                r4->mE8.m24_stepRotation[1] = MTH_Mul(0x38E38E, r4->m150_inputX);
            }
            break;
        default:
            assert(0);
            break;
        }
    }
    else
    {
        r4->mE8.m24_stepRotation[1] = 0;
    }

    //605BDAE
    if (r4->m14C_inputFlags & 0x80)
    {
        r4->mE8.m30_stepTranslation[0] = 0;
        r4->mE8.m30_stepTranslation[2] = 0;
    }
    else
    {
        s32 r10;
        if (r4->m14C_inputFlags & 0x2)
        {
            r10 = -0x212;
        }
        else
        {
            r10 = -0x109;
        }

        //0605BDCA
        switch (twnMainLogicTask->m0)
        {
        case 0:
            r4->mE8.m30_stepTranslation[2] = MTH_Mul(r4->m154_inputY, r10);
            r4->mE8.m30_stepTranslation[0] = 0;
            break;
        default:
            assert(0);
            break;
        }
    }

    //605BE7E
    r4->m14C_inputFlags &= 1;
    if (r4->m14C_inputFlags)
    {
        assert(0);
    }
}

void updateEdgePositionSub2(sNPCE8* r4)
{
    r4->mC_rotation[1] += r4->m24_stepRotation[1];
}

// TODO: kernel
void updateEdgePositionSub3Sub1(const sVec3_FP& r4, sVec2_FP* r5)
{
    if ((r4[0] == 0) && (r4[2] == 0))
    {
        if (r4[1] >= 0)
        {
            (*r5)[0] = 0x4000000;
        }
        else
        {
            (*r5)[0] = -0x4000000;
        }
    }

    //6036342
    fixedPoint r0 = sqrt_F(FP_Pow2(r4[0]) + FP_Pow2(r4[2]));

    if (r4[1] >= 0)
    {
        (*r5)[0] = atan2_FP(r4[1], r0);
    }
    else
    {
        (*r5)[0] = -atan2_FP(-r4[1], r0);
    }

    (*r5)[1] = atan2_FP(r4[0], r4[2]);
}

void updateEdgeLookAt(sEdgeTask* r4)
{
    sNPCE8* r13_npcE8 = &r4->mE8;

    if ((currentResTask->m8_currentLCSType) && (npcData0.mFC & 1))
    {
        //605BEEA
        //assert(0);
        Unimplemented();
    }
    //605C018
    else if ((npcData0.mFC & 0x10) && !(npcData0.mFC & 0x8))
    {
        //0605C030
        pushCurrentMatrix();
        translateCurrentMatrix(r13_npcE8->m0_position);
        rotateCurrentMatrixShiftedY(r13_npcE8->mC_rotation[1]);
        rotateCurrentMatrixShiftedX(r13_npcE8->mC_rotation[0]);
        adjustMatrixTranslation(0x1800);
        sVec3_FP var14;

        var14[0] = pCurrentMatrix->m[0][3] - setDividend(LCSCollisionData.m0_LCS_X.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m2C_projectionWidthScale);
        var14[1] = pCurrentMatrix->m[1][3] - setDividend(LCSCollisionData.m4_LCS_Y.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m30_projectionHeightScale);
        var14[2] = pCurrentMatrix->m[2][3] - LCSCollisionData.m28_LCSDepthMax;

        sVec2_FP varC;
        updateEdgePositionSub3Sub1(var14, &varC);

        var14[0] = pCurrentMatrix->m[2][0];
        var14[1] = pCurrentMatrix->m[2][1];
        var14[2] = pCurrentMatrix->m[2][2];
        sVec2_FP var4;
        updateEdgePositionSub3Sub1(var14, &var4);

        popMatrix();

        varC -= var4;

        varC[0] = varC[0].normalized();
        varC[1] = (-varC[1]).normalized();

        //0605C12E
        if (varC[0] > 0x18E38E3)
        {
            varC[0] = 0x18E38E3;
        }
        if (varC[0] < -0x18E38E3)
        {
            varC[0] = -0x18E38E3;
        }

        if (varC[1] > 0x38E38E3)
        {
            varC[1] = 0x38E38E3;
        }
        if (varC[1] < -0x38E38E3)
        {
            varC[1] = -0x38E38E3;
        }

        r4->m20_lookAtAngle[0] += MTH_Mul(varC[0] - r4->m20_lookAtAngle[0], 0xB333);
        r4->m20_lookAtAngle[1] += MTH_Mul(varC[1] - r4->m20_lookAtAngle[1], 0xB333);
    }
    else
    {
        //605C174
        fixedPoint r13 = r4->mE8.m24_stepRotation[1];
        if (r13 > 0x1C71C71)
        {
            r13 = 0x1C71C71;
        }
        if (r13 < -0x1C71C71)
        {
            r13 = -0x1C71C71;
        }

        if (r13)
        {
            r4->m20_lookAtAngle[1] += MTH_Mul(r13 - r4->m20_lookAtAngle[1], 0xB333);
        }
        else
        {
            r4->m20_lookAtAngle[1] += MTH_Mul(r13 - r4->m20_lookAtAngle[1], 0x8000);
        }

        r4->m20_lookAtAngle[0] = MTH_Mul(r4->m20_lookAtAngle[0], 0xB333);
    }
}

void updateEdgePosition(sNPC* r4)
{
    sEdgeTask* r12 = static_cast<sEdgeTask*>(r4);
    sNPCE8* r13 = &r12->mE8;
    sCollisionBody* r14 = &r12->m84;

    updateEdgeControls(r12);
    updateEdgePositionSub1(r12);

    *r4->m84.m30_pPosition += r4->m84.m58_collisionSolveTranslation;
    if (r4->m84.m44 & 4)
    {
        if (r14->m4C[1] < 0xB504)
        {
            r12->mF |= 0x80;
        }
        else
        {
            r12->mF &= ~0x80;
            if (r13->m30_stepTranslation[1] < 0)
            {
                r13->m30_stepTranslation[1] = 0;
            }
        }

        //605B948
        if (r12->mE_controlState != 4)
        {
            r12->mE_controlState = 4;
        }
    }
    else
    {
        //605B954
        if ((r13->m30_stepTranslation[1] < -0x199) || (r13->m30_stepTranslation[1] > 0))
        {
            r12->mF |= 0x80;
            if (r12->m2C_currentAnimation != 4)
            {
                r12->m2C_currentAnimation = 4; //falling
                sSaturnPtr var0 = r12->m30_animationTable + 4 * r12->m2C_currentAnimation; // walk animation

                u32 offset = readSaturnU16(var0 + 2);
                sAnimationData* buffer;
                if (readSaturnU16(var0))
                {
                    buffer = dramAllocatorEnd[0].mC_fileBundle->m0_fileBundle->getAnimation(offset);
                }
                else
                {
                    buffer = r12->m0_fileBundle->getAnimation(offset);
                }

                // play falling animation
                playAnimationGeneric(&r12->m34_3dModel, buffer, 5);
            }
        }
    }
    //605B9AA
    static bool gGravity = true;
    if(!isShipping())
    {
        ImGui::Begin("Town");
        ImGui::Checkbox("Gravity", &gGravity);
        ImGui::End();
    }

    if (gGravity)
    {
        r13->m30_stepTranslation[1] += -0x56;
    }

    if (r13->m30_stepTranslation[1] < -0x800)
    {
        r13->m30_stepTranslation[1] = -0x800;
    }

    updateEdgePositionSub2(r13);

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var10);

    if (r14->m44 & 4)
    {
        //0605B9F0
        if ((r14->m4C[0] != 0) || (r14->m4C[2] != 0))
        {
            if ((r14->m4C[0] == 0) && (r14->m4C[1] == 0))
            {
                var10.m[0][2] = var10.m[0][1];
                var10.m[1][2] = var10.m[1][1];
                var10.m[2][2] = var10.m[2][1];
            }
            else if ((r14->m4C[1] == 0) && (r14->m4C[2] == 0))
            {
                var10.m[0][0] = var10.m[0][1];
                var10.m[1][0] = var10.m[1][1];
                var10.m[2][0] = var10.m[2][1];
            }
            else
            {
                //605BA64
                assert(0);
            }
        }
    }

    //0605BB48
    if ((r14->m44 & 4) && (r14->m4C[1] < 0xB504))
    {
        //605BB50
        assert(0);
    }

    //605BBD6
    transformVec(r13->m30_stepTranslation, r13->m18_stepTranslationInWorld, var10);

    r13->m0_position += r13->m18_stepTranslationInWorld;

    updateEdgeLookAt(r12);

    r12->m14C_inputFlags = 0;
}

s32 scriptFunction_605B320(s32 arg0, s32 arg1)
{
    getNpcDataByIndex(arg0)->mE_controlState = arg1;
    return 0;
}

TWN_RUIN_data::TWN_RUIN_data() : sTownOverlay("TWN_RUIN.PRG")
{
    overlayScriptFunctions.m_zeroArg[0x6057570] = &hasLoadingCompleted;
    overlayScriptFunctions.m_zeroArg[0x6057058] = &scriptFunction_6057058;
    overlayScriptFunctions.m_zeroArg[0x605762A] = &scriptFunction_605762A;
    overlayScriptFunctions.m_zeroArg[0x605800E] = &isObjectCloseEnoughToActivate;

    overlayScriptFunctions.m_oneArg[0x605C83C] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605c7c4] = &TwnFadeIn;
    overlayScriptFunctions.m_oneArg[0x6054364] = &scriptFunction_6054364_waitForLockDisableCompletion;

    overlayScriptFunctions.m_twoArg[0x605C55C] = &townCamera_setup;
    overlayScriptFunctions.m_twoArg[0x605B320] = &scriptFunction_605B320;
    overlayScriptFunctions.m_twoArg[0x6054334] = &scriptFunction_6054334_disableLock;

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605E984), 12));
}
