#include "PDS.h"
#include "twn_ruin.h"
#include "town/town.h"
#include "town/townScript.h"
#include "town/townEdge.h"
#include "town/townLCS.h"
#include "town/townMainLogic.h"
#include "kernel/animation.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"

#include "twn_ruin_lock.h"

sTownObject* TWN_RUIN_data::createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg)
{
    assert(definitionEA.m_file == this);
    assert(arg.m_file == this);

    switch (definitionEA.m_offset)
    {
    case 0x605EA20:
        return createSiblingTaskWithArgWithCopy<sLockTask>(parent, arg);
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

void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
}

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
        FunctionUnimplemented();
    }

    static void Update(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
    }

    static void Draw(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
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

    npcData0.m116 = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

//(0 cursor, 1 near, 2 far)
void drawLcsSprite(const sVec2_S16& r4, s32 r5_index)
{
    sSaturnPtr r5_spriteData = gTWN_RUIN->getSaturnPtr(0x605EEFC + r5_index * 0x1C);
    s32 r6 = dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory;
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
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, readSaturnU16(r5_spriteData + 2)); // command
    setVdp1VramU16(vdp1WriteEA + 0x04, readSaturnU16(r5_spriteData + 4)); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, var0); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, var4 ); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, readSaturnU16(r5_spriteData + 8)); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, var10[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -var10[1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x14, var14[0]);
    setVdp1VramU16(vdp1WriteEA + 0x16, -var14[1]);

    s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
    pExtendedCommand->depth = 0;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
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


const std::array<sVec2_FP, 2> cameraParams = {
    {
        {0x2CCC, 0xAAAAAA},
        {0x2CCC, -0x555555},
    }
};

fixedPoint transformByMatrixRow0(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[0] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[1] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[2] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[3];
}

fixedPoint transformByMatrixRow1(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[4] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[5] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[6] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[7];
}

fixedPoint transformByMatrixRow2(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->matrix[8] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->matrix[9] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->matrix[10] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->matrix[11];
}

fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5)
{
    s64 mac = 0;
    mac += (s64)r4.m_value[0] * (s64)r5.m_value[0];
    mac += (s64)r4.m_value[1] * (s64)r5.m_value[1];
    
    return fixedPoint(mac >> 16);
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
        FunctionUnimplemented();
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
    if ((MTH_Mul(0x151EB, cameraParam[0]) < r14_townTask->m24_distance) && !(r14_townTask->m4_flags &0x10000))
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

    if(((r6 < 0x71C71C7) && (r6 > -0x71C71C7)) || (r13_npcData->m54_oldPosition == r13_npcData->m0_position))
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

s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    loadSoundBanks(-1, 0);
    loadSoundBanks(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

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

s32 TwnFadeOut(s32 arg0)
{
    cameraTaskPtr->m1 = 0;
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    return 0;
}

u16 TwnFadeInComputeColor(sSaturnPtr r4, u32 r5)
{
    FunctionUnimplemented();
    return 0;
}

s32 TwnFadeIn(s32 arg0)
{
    u16 fadeColor;
    switch (cameraTaskPtr->m0)
    {
    case 0:
        fadeColor = TwnFadeInComputeColor(cameraTaskPtr->m8, cameraTaskPtr->m30);
        break;
    default:
        assert(0);
        break;
    }

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xC210, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);

    cameraTaskPtr->m1 = 1;

    return 0;
}

s32 isObjectCloseEnoughToActivate()
{
    return !canCurrentResActivate();
}

s32 hasLoadingCompleted()
{
    FunctionUnimplemented();
    return 1;
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

void scriptFunction_6057058_sub0()
{
    twnMainLogicTask->m_DrawMethod = sMainLogic::Draw;
    scriptFunction_6057058_sub0Sub0();
    twnMainLogicTask->m2_cameraFollowMode = 0;
    mainLogicUpdateSub3();
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
        FunctionUnimplemented();
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

        var14[0] = pCurrentMatrix->matrix[3] - setDividend(LCSCollisionData.m0_LCS_X.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m2C_projectionWidthScale);
        var14[1] = pCurrentMatrix->matrix[7] - setDividend(LCSCollisionData.m4_LCS_Y.getInteger(), LCSCollisionData.m28_LCSDepthMax, LCSCollisionData.m30_projectionHeightScale);
        var14[2] = pCurrentMatrix->matrix[11] - LCSCollisionData.m28_LCSDepthMax;

        sVec2_FP varC;
        updateEdgePositionSub3Sub1(var14, &varC);

        var14[0] = pCurrentMatrix->matrix[8];
        var14[1] = pCurrentMatrix->matrix[9];
        var14[2] = pCurrentMatrix->matrix[10];
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
    sMainLogic_74* r14 = &r12->m84;

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
                    buffer = dramAllocatorEnd[0].mC_buffer->m0_dramAllocation->getAnimation(offset);
                }
                else
                {
                    buffer = r12->m0_dramAllocation->getAnimation(offset);
                }

                // play falling animation
                playAnimationGeneric(&r12->m34_3dModel, buffer, 5);
            }
        }
    }
    //605B9AA
    static bool gGravity = true;
    ImGui::Begin("Town");
    ImGui::Checkbox("Gravity", &gGravity);
    ImGui::End();

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
                var10.matrix[2] = var10.matrix[1];
                var10.matrix[6] = var10.matrix[5];
                var10.matrix[10] = var10.matrix[9];
            }
            else if ((r14->m4C[1] == 0) && (r14->m4C[2] == 0))
            {
                var10.matrix[0] = var10.matrix[1];
                var10.matrix[4] = var10.matrix[5];
                var10.matrix[8] = var10.matrix[9];
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

void TWN_RUIN_data::create()
{
    if (gTWN_RUIN == NULL)
    {
        FILE* fHandle = fopen("TWN_RUIN.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_RUIN = new TWN_RUIN_data();
        gTWN_RUIN->m_name = "TWN_RUIN.PRG";
        gTWN_RUIN->m_data = fileData;
        gTWN_RUIN->m_dataSize = fileSize;
        gTWN_RUIN->m_base = 0x6054000;

        gTWN_RUIN->init();
    }
}

void TWN_RUIN_data::init()
{
    gCurrentTownOverlay = this;

    overlayScriptFunctions.m_zeroArg[0x6057570] = &hasLoadingCompleted;
    overlayScriptFunctions.m_zeroArg[0x6057058] = &scriptFunction_6057058;
    overlayScriptFunctions.m_zeroArg[0x605762A] = &scriptFunction_605762A;
    overlayScriptFunctions.m_zeroArg[0x605800E] = &isObjectCloseEnoughToActivate;

    overlayScriptFunctions.m_oneArg[0x605C83C] = &TwnFadeOut;
    overlayScriptFunctions.m_oneArg[0x605c7c4] = &TwnFadeIn;
    overlayScriptFunctions.m_oneArg[0x6054364] = &scriptFunction_6054364_waitForLockDisableCompletion;

    overlayScriptFunctions.m_twoArg[0x605B320] = &scriptFunction_605B320;
    overlayScriptFunctions.m_twoArg[0x6054334] = &scriptFunction_6054334_disableLock;

    mTownSetups.push_back(readTownSetup(getSaturnPtr(0x605E984), 12));
}
