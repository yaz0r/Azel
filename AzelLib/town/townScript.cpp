#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "town/ruin/twn_ruin.h"

#include <map>

sResData resData;

s32 resValue0 = 0;
sScriptTask* currentResTask = nullptr;

sResCameraProperties resCameraProperties;

void copyCameraPropertiesToRes()
{
    resCameraProperties.m8_LCSWidth = 2;
    resCameraProperties.mC_LCSHeight = 2;
    resCameraProperties.m10 = 0;
    resCameraProperties.m18 = graphicEngineStatus.m405C.m30;
    resCameraProperties.m24 = graphicEngineStatus.m405C.m10;
    resCameraProperties.m2C = graphicEngineStatus.m405C.m18_widthScale;
    resCameraProperties.m30 = graphicEngineStatus.m405C.m1C_heightScale;
}

struct sScriptUpdateSub0Sub0Var0
{
    sVec3_FP m0_position;
    s32 mC_distance;
    fixedPoint m10_y;
    //size 0x14
};

std::array< sScriptUpdateSub0Sub0Var0, 12> scriptUpdateSub0Sub0Var0;

struct sScriptUpdateSub0Sub0Var1
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    //size 0x10?
}scriptUpdateSub0Sub0Var1;


void scriptUpdateSub0Sub0(sMainLogic_74* r4)
{
    const sVec3_FP& r12 = *r4->m34_pRotation;

    switch (r4->m0)
    {
    case 0:
    case 1:
        for (int i = 0; i < 12; i++)
        {
            scriptUpdateSub0Sub0Var0[i].mC_distance = 0;
        }
        scriptUpdateSub0Sub0Var1.m0 = 0;
        scriptUpdateSub0Sub0Var1.m4 = 0;
        scriptUpdateSub0Sub0Var1.m8 = 0;
        scriptUpdateSub0Sub0Var1.mC = 0;
    case 2:
        r4->m48 = 0;
    case 3:
        r4->m44 = 0;
        break;
    default:
        assert(0);
        break;
    }

    pushCurrentMatrix();

    initMatrixToIdentity(pCurrentMatrix);
    pCurrentMatrix->matrix[3] = r4->m14[0];
    pCurrentMatrix->matrix[7] = r4->m14[1];
    pCurrentMatrix->matrix[11] = r4->m14[2];

    rotateCurrentMatrixShiftedZ(-r12[2]);
    rotateCurrentMatrixShiftedX(-r12[0]);
    rotateCurrentMatrixShiftedY(-r12[1]);

    sVec3_FP var0;
    var0[0] = -r4->m8_position[0] - resValue0;
    var0[1] = -r4->m8_position[1];
    var0[2] = -r4->m8_position[2] - resValue0;

    translateCurrentMatrix(var0);
}

void addBackgroundScript(sSaturnPtr r4, s32 r5, p_workArea r6, sVec3_S16* r7)
{
    if (r4.m_offset == 0)
        return;

    if (npcData0.m0_numBackgroundScripts >= 4)
        return;

    std::array<sRunningScriptContext, 4>::iterator r14 = npcData0.m4_backgroundScripts.begin();
    if (npcData0.m0_numBackgroundScripts)
    {
        for(s32 r13 = npcData0.m0_numBackgroundScripts; r13 >= 0; r14++, r13--)
        {
            if (r14->m0_scriptPtr == r4)
            {
                if (r14->m4 == r5)
                {
                    return;
                }
            }
        }
    }

    r14->m0_scriptPtr = r4;
    r14->m4 = r5;
    r14->m8_owner.setTask(r6);
    if (r7)
    {
        r14->mC = *r7;
    }

    npcData0.m0_numBackgroundScripts++;
}

s32 scriptUpdateSub0Sub1(sMainLogic_74* r13, sMainLogic_74* r14)
{
    if (r14->m40)
    {
        assert(0);
    }
    else
    {
        if (distanceSquareBetween2Points(r13->m8_position, r14->m8_position) >= FP_Pow2(r13->m4_collisionRadius + r13->m4_collisionRadius))
            return 0;

        //0600874E
        assert(0);
    }

    return 0;
}

void scriptUpdateSub0Sub2Sub0(sMainLogic_74* r4, fixedPoint r5, sVec3_FP* r6, sVec3_FP* r7)
{
    assert(0);
}

void scriptUpdateSub0Sub2Sub1(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[2]) + r13->m14[2];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[4];
        r13->m44 |= 0x2;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[10];
        r13->m44 |= 0x200;
    }

    if (r4->mC_distance < r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 + (*r6)[2];
    }
}

void scriptUpdateSub0Sub2Sub2(sMainLogic_74* r4, fixedPoint r5, sVec3_FP* r6, sVec3_FP* r7)
{
    assert(0);
}

void scriptUpdateSub0Sub2Sub3(sMainLogic_74* r4, fixedPoint r5, sVec3_FP* r6, sVec3_FP* r7)
{
    assert(0);
}

void scriptUpdateSub0Sub2Sub4(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) - r13->m14[1];

    sScriptUpdateSub0Sub0Var0* r4;
    if(r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[4];
        r13->m44 |= 0x8;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[9];
        r13->m44 |= 0x800;
    }

    if (r4->mC_distance > r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 - (*r6)[1];
    }
}

void scriptUpdateSub0Sub2Sub5(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) + r13->m14[1];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[2];
        r13->m44 |= 0x4;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[8];
        r13->m44 |= 0x400;
    }

    if (r4->mC_distance < r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 + (*r6)[1];
    }
}

void scriptUpdateSub0Sub2(sMainLogic_74* r4, fixedPoint r5)
{
    sVec3_FP var4;
    var4[0] = pCurrentMatrix->matrix[1];
    var4[1] = pCurrentMatrix->matrix[5];
    var4[2] = pCurrentMatrix->matrix[9];

    sVec3_FP var10;
    var10[0] = MTH_Mul(var4[0], r4->m14[0]);
    var10[1] = MTH_Mul(var4[1], r4->m14[1]);
    var10[2] = MTH_Mul(var4[2], r4->m14[2]);

    fixedPoint r14 = r5;
    r14 += MTH_Mul(var4[0], pCurrentMatrix->matrix[3]);
    r14 += MTH_Mul(var4[1], pCurrentMatrix->matrix[7]);
    r14 += MTH_Mul(var4[2], pCurrentMatrix->matrix[11]);
    r14 -= var10[0];
    r14 -= var10[1];
    r14 -= var10[2];

    switch (r4->m0)
    {
    case 0:
        // 6009474
        if (FP_Pow2(r14) < FP_Pow2(var10[2]))
        {
            if (r14 > var10[2])
            {
                scriptUpdateSub0Sub2Sub0(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub1(r4, r14, &var10, &var4);
            }
        }
        //60094AA
        if (FP_Pow2(r14) < FP_Pow2(var10[0]))
        {
            if (r14 > var10[0])
            {
                scriptUpdateSub0Sub2Sub2(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub3(r4, r14, &var10, &var4);
            }
        }
        //60094E0
        if (FP_Pow2(r14) < FP_Pow2(var10[1]))
        {
            if (r14 > var10[1])
            {
                scriptUpdateSub0Sub2Sub4(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub5(r4, r14, &var10, &var4);
            }
        }
        break;
    default:
        assert(0);
    }
}

s32 scriptUpdateSub0Sub3Sub1(fixedPoint r4_x, fixedPoint r5_z)
{
    s32 r8 = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex * 2) & 1;
    s32 r0 = MTH_Mul32(r5_z, gTownGrid.m30_worldToCellIndex * 4) & 2;

    return r8 + r0;
}

sEnvironmentTask* scriptUpdateSub0Sub3Sub0(fixedPoint r4_x, fixedPoint r5_z)
{
    s32 r13_cellX = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex);
    s32 r5_cellY = MTH_Mul32(r5_z, gTownGrid.m30_worldToCellIndex);

    if (r13_cellX < 0)
        return nullptr;

    if (r13_cellX >= gTownGrid.m0_sizeX)
        return nullptr;

    if (r5_cellY < 0)
        return nullptr;

    if (r5_cellY >= gTownGrid.m4_sizeY)
        return nullptr;

    r13_cellX -= gTownGrid.m10_currentX;
    r5_cellY -= gTownGrid.m14_currentY;

    if (r13_cellX < -3)
        return nullptr;

    if (r13_cellX > 3)
        return nullptr;

    if (r5_cellY < -3)
        return nullptr;

    if (r5_cellY > 3)
        return nullptr;

    return gTownGrid.m40_cellTasks[(gTownGrid.mC + r5_cellY) & 7][(gTownGrid.m8 + r13_cellX) & 7];
}

void processTownMeshCollision(sMainLogic_74* r4, u8* r5)
{
    FunctionUnimplemented();
}

void scriptUpdateSub0Sub3Sub2(sMainLogic_74* r12, sEnvironmentTask* r13)
{
    if (r13 == nullptr)
        return;

    if (r13->m8.m_offset == 0)
        return;

    if (readSaturnU32(r13->m8 + 0x14) == 0)
        return;

    pushCurrentMatrix();
    {
        sVec3_FP var0_positionInCell;
        var0_positionInCell = r12->m8_position - r13->mC_position;

        sSaturnPtr r14 = readSaturnEA(r13->m8 + 0x14);
        while (readSaturnU32(r14))
        {
            u8* r11 = r13->m0_dramAllocation + READ_BE_U32(r13->m0_dramAllocation + readSaturnU32(r14));
            sVec3_FP meshPositionInCell = readSaturnVec3(r14 + 4);
            if (distanceSquareBetween2Points(var0_positionInCell, meshPositionInCell) < FP_Pow2(READ_BE_S32(r11) + r12->m4_collisionRadius))
            {
                pushCurrentMatrix();
                translateCurrentMatrix(meshPositionInCell);
                processTownMeshCollision(r12, r11);
                popMatrix();
            }

            r14 += 0x10;
        }
    }
    popMatrix();
}

void handleCollisionWithTownEnv(sMainLogic_74* r4)
{
    s32 type = scriptUpdateSub0Sub3Sub1(r4->m8_position[0], r4->m8_position[2]);
    switch (type)
    {
    case 0:
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] - gTownGrid.m28_cellSize, r4->m8_position[2]));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2] - gTownGrid.m28_cellSize));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] - gTownGrid.m28_cellSize, r4->m8_position[2] - gTownGrid.m28_cellSize));
        break;
    case 1:
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] + gTownGrid.m28_cellSize, r4->m8_position[2]));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2] - gTownGrid.m28_cellSize));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] + gTownGrid.m28_cellSize, r4->m8_position[2] - gTownGrid.m28_cellSize));
        break;
    default:
        assert(0);
        break;
    }

    scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2]));
}

void scriptUpdateSub0Sub4Sub0(sMainLogic_74* r12)
{
    std::array< sScriptUpdateSub0Sub0Var0, 12>& r13 = scriptUpdateSub0Sub0Var0;

    r12->m44 &= ~(r12->m44 << 8);

    if ((r12->m44 & 0x400) && (r12->m44 & 0x8))
    {
        //06007C86
        assert(0);
    }
    else if ((r12->m44 & 0x800) && (r12->m44 & 0x4))
    {
        //06007CBA
        assert(0);
    }
    else if ((r12->m44 & 0x200) && !(r12->m44 & 0x1))
    {
        //06007D24
        r12->m44 |= 2;
        r13[4] = r13[10];
    }
    else if ((r12->m44 & 0x100) && !(r12->m44 & 0x2))
    {
        //06007D5E
        assert(0);
    }
    else if ((r12->m44 & 0x2000) && !(r12->m44 & 0x10))
    {
        //06007D98
        assert(0);
    }
    else if ((r12->m44 & 0x1000) && !(r12->m44 & 0x20))
    {
        //06007DD6
        assert(0);
    }

    //6007DF8
    sVec3_FP var14;
    var14.zero();

    const sScriptUpdateSub0Sub0Var0& var0 = scriptUpdateSub0Sub0Var0[4];
    const sScriptUpdateSub0Sub0Var0& var4 = scriptUpdateSub0Sub0Var0[5];
    const sScriptUpdateSub0Sub0Var0& r8 = scriptUpdateSub0Sub0Var0[1];

    switch (r12->m44 & 0x33)
    {
    case 0:
        break;
    case 2:
    {
        fixedPoint r10 = FP_Div(var0.m10_y, sqrt_F(FP_Pow2(-var0.m0_position[1]) + 0x10000));
        var14[0] = MTH_Mul(var0.m0_position[0], r10);
        var14[2] = MTH_Mul(var0.m0_position[2], r10);
        break;
    }
    default:
        assert(0);
        break;
    }

    //6008446
    const sScriptUpdateSub0Sub0Var0& r10 = r13[3];
    switch (r12->m44 & 0xC)
    {
    case 0:
        break;
    case 4:
        var14[1] = r13[2].mC_distance;
        break;
    case 8:
        var14[1] = r13[3].mC_distance;
        break;
    default:
        assert(0);
        break;
    }

    //0600854A
    if ((scriptUpdateSub0Sub0Var1.m0 != 0) || (scriptUpdateSub0Sub0Var1.m4 != 0))
    {
        assert(0);
    }

    //060085A4
    if ((scriptUpdateSub0Sub0Var1.m8 != 0) || (scriptUpdateSub0Sub0Var1.mC != 0))
    {
        assert(0);
    }

    //06008604
    if (r12->m44 & 4)
    {
        r12->m4C = r13[2].m0_position;
    }

    //6008622
    if (var14[0] > r12->m14[0] / 2)
    {
        var14[0] = r12->m14[0] / 2;
    }

    if (var14[0] < -r12->m14[0] / 2)
    {
        var14[0] = -r12->m14[0] / 2;
    }

    if (var14[1] > r12->m14[1] / 2)
    {
        var14[1] = r12->m14[1] / 2;
    }

    if (var14[1] < -r12->m14[1] / 2)
    {
        var14[1] = -r12->m14[1] / 2;
    }

    if (var14[2] > r12->m14[2] / 2)
    {
        var14[2] = r12->m14[2] / 2;
    }

    if (var14[2] < -r12->m14[2] / 2)
    {
        var14[2] = -r12->m14[2] / 2;
    }

    //6008668
    sMatrix4x3 var20;
    initMatrixToIdentity(&var20);
    rotateMatrixYXZ(r12->m34_pRotation, &var20);
    transformVec(var14, r12->m58_collisionSolveTranslation, var20);
}

void scriptUpdateSub0Sub4(sMainLogic_74* r4)
{
    popMatrix();

    switch (r4->m0)
    {
    case 0:
    case 1:
        scriptUpdateSub0Sub4Sub0(r4);
        break;
    case 2:
    case 3:
        break;
    default:
        assert(0);
        break;
    }
}

void scriptUpdateSub0()
{
    for (int r8 = 4-1; r8 > 0; r8--)
    {
        sResData1C* r11 = resData.m8_headOfLinkedList[r8];
        while (r11)
        {
            sMainLogic_74* r14 = r11->m4;
            r11 = r11->m0_pNext;

            if (r14->m2_collisionLayersBitField)
            {
                scriptUpdateSub0Sub0(r14);
                s32 r9 = r8 + 1;
                do 
                {
                    if (r14->m2_collisionLayersBitField & (1 << r9))
                    {
                        sResData1C* r12 = resData.m8_headOfLinkedList[r9];
                        while (r12)
                        {
                            sMainLogic_74* r13 = r12->m4;
                            r12 = r12->m0_pNext;
                            if (scriptUpdateSub0Sub1(r14, r13))
                            {
                                if ((r14->m0 == 0) && r13->m3C_scriptEA.m_offset)
                                {
                                    //06007B16
                                    addBackgroundScript(r13->m3C_scriptEA, 1, r13->m38_pOwner, 0);
                                }

                                r14->m48 = r13;
                                r13->m48 = r14;
                                if (r14->m0 >= 2)
                                {
                                    goto endOfLoop;
                                }
                            }
                        }
                    }
                } while (++r9 < 5);
                //06007B3C
                
                if (r14->m2_collisionLayersBitField & 0x10)
                {
                    scriptUpdateSub0Sub2(r14, resData.m0);
                }

                handleCollisionWithTownEnv(r14);

            endOfLoop:
                //06007B4E
                scriptUpdateSub0Sub4(r14);
            }
        }
    }
}

void initResTable()
{
    resData.m8_headOfLinkedList.fill(0);
    resData.m4 = 0;

    if (resValue0 < 0)
    {
        resValue0 = 2;
    }
    else
    {
        resValue0 = -2;
    }
}

s32 setSomethingInNpc0(s32 arg0, s32 arg1)
{
    fixedPoint value;
    switch (arg1)
    {
    case 0:
        value = 0x7D0000;
        break;
    case 1:
        value = 0x5000;
        break;
    case 2:
        value = 0x2000;
        break;
    case 3:
        value = -1;
        break;
    default:
        assert(0);
    }

    if (arg0 != 1)
    {
        npcData0.m54_activationNear = value;
    }
    else
    {
        npcData0.m58_activationFar = value;
    }

    return 0;
}

sNPC* getNpcDataByIndex(s32 r4)
{
    if (r4 == -1)
    {
        return npcData0.m104_currentScript.m8_owner.getNPC();
    }
    
    return npcData0.m70_npcPointerArray[r4].pNPC;
}

s32 setNpcLocation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z)
{
    sNPC* pNPC = getNpcDataByIndex(r4_npcIndex);
    if (pNPC)
    {
        pNPC->mE8.m0_position[0] = r5_X;
        pNPC->mE8.m0_position[1] = r6_Y;
        pNPC->mE8.m0_position[2] = r7_Z;
    }

    return 0;
}

s32 setNpcOrientation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z)
{
    sNPC* pNPC = getNpcDataByIndex(r4_npcIndex);
    if (pNPC)
    {
        pNPC->mE8.mC_rotation[0] = r5_X;
        pNPC->mE8.mC_rotation[1] = r6_Y;
        pNPC->mE8.mC_rotation[2] = r7_Z;
    }

    return 0;
}


typedef s32(*scriptFunction_zero_arg)();
typedef s32(*scriptFunction_one_arg)(s32 arg0);
typedef s32(*scriptFunction_one_arg_ptr)(sSaturnPtr arg0);
typedef s32(*scriptFunction_two_arg)(s32 arg0, s32 arg1);
typedef s32(*scriptFunction_four_arg)(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

struct sKernelScriptFunctions
{
    std::map<u32, scriptFunction_zero_arg> m_zeroArg;
    std::map<u32, scriptFunction_one_arg> m_oneArg;
    std::map<u32, scriptFunction_one_arg_ptr> m_oneArgPtr;
    std::map<u32, scriptFunction_two_arg> m_twoArg;
    std::map<u32, scriptFunction_four_arg> m_fourArg;
};

sKernelScriptFunctions gKernelScriptFunctions =
{
    // zero arg
    {},
    // one arg
    {
        {0x600CCB4, &initNPC},
    },
    // one arg ptr
    {
        {0x6014DF2, &initNPCFromStruct}
    },
    // two arg
    {
        {0x600CC78, &setSomethingInNpc0},
    },
    // four arg
    {
        {0x605AEE0, &setNpcLocation},
        {0x605AF0E, &setNpcOrientation},
    },
};

sSaturnPtr callNativeWithArguments(sNpcData* r4_pThis, sSaturnPtr r5)
{
    u8 numArguments = readSaturnU8(r5);
    r5 = r5 + 1;
    sSaturnPtr r14 = r5 + 3;
    r14.m_offset &= ~3;

    switch (numArguments)
    {
    case 0:
        if (gKernelScriptFunctions.m_zeroArg.count(readSaturnEA(r14).m_offset))
        {
            scriptFunction_zero_arg pFunction = gKernelScriptFunctions.m_zeroArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction();
        }
        else
        {
            r4_pThis->m118_currentResult = TWN_RUIN_ExecuteNative(readSaturnEA(r14));
        }
        break;
    case 1:
        if (gKernelScriptFunctions.m_oneArg.count(readSaturnEA(r14).m_offset))
        {
            scriptFunction_one_arg pFunction = gKernelScriptFunctions.m_oneArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4));
        }
        else if (gKernelScriptFunctions.m_oneArgPtr.count(readSaturnEA(r14).m_offset))
        {
            scriptFunction_one_arg_ptr pFunction = gKernelScriptFunctions.m_oneArgPtr.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnEA(r14 + 4));
        }
        else
        {
            r4_pThis->m118_currentResult = TWN_RUIN_ExecuteNative(readSaturnEA(r14), readSaturnS32(r14 + 4));
        }
        break;
    case 2:
        if (gKernelScriptFunctions.m_twoArg.count(readSaturnEA(r14).m_offset))
        {
            scriptFunction_two_arg pFunction = gKernelScriptFunctions.m_twoArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        else
        {
            r4_pThis->m118_currentResult = TWN_RUIN_ExecuteNative(readSaturnEA(r14), readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        break;
    case 4:
        if (gKernelScriptFunctions.m_fourArg.count(readSaturnEA(r14).m_offset))
        {
            scriptFunction_four_arg pFunction = gKernelScriptFunctions.m_fourArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8), readSaturnS32(r14 + 12), readSaturnS32(r14 + 16));
        }
        else
        {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }

    return r14 + (numArguments + 1) * 4;
}

sSaturnPtr getAlignOn2(const sSaturnPtr& inPtr)
{
    sSaturnPtr output = inPtr;
    output.m_offset += 1;
    output.m_offset &= ~1;

    return output;
}

sSaturnPtr getAlignOn4(const sSaturnPtr& inPtr)
{
    sSaturnPtr output = inPtr;
    output.m_offset += 3;
    output.m_offset &= ~3;

    return output;
}

sSaturnPtr runScript(sNpcData* r13_pThis)
{
    sRunningScriptContext* varC = &r13_pThis->m104_currentScript;
    sSaturnPtr r14 = r13_pThis->m104_currentScript.m0_scriptPtr;

    do
    {
        u8 r0_opcode = readSaturnU8(r14++);

        switch (r0_opcode)
        {
        case 1: //end
            if (r13_pThis->m11C_currentStackPointer == r13_pThis->m120_stack.end())
            {
                r14.m_offset = 0;
                return r14;
            }
            else
            {
                r14 = *r13_pThis->m11C_currentStackPointer;
                r13_pThis->m11C_currentStackPointer++;
            }
            break;
        case 2: //wait
        {
            sSaturnPtr r4 = r14 + 1;
            r4.m_offset &= ~1;
            u16 delay = readSaturnU16(r4);
            r4 += 2;

            if (r13_pThis->mF0)
            {
                assert(0);
            }

            r13_pThis->m100 = delay;
            return r4;
        }
        case 5: //if
            if (r13_pThis->m118_currentResult)
            {
                r14 += 4;
                r14 = getAlignOn4(r14);
            }
            else
            {
                r14 = readSaturnEA(getAlignOn4(r14));
            }
            break;
        case 6: //callScript
            *(--r13_pThis->m11C_currentStackPointer) = getAlignOn4(r14) + 4;
            r14 = readSaturnEA(getAlignOn4(r14));
            break;
        case 7: //callNative
            r14 = callNativeWithArguments(r13_pThis, r14);
            break;
        case 9:
            r14 = getAlignOn2(r14);
            if (readSaturnS16(r14) != r13_pThis->m118_currentResult)
            {
                r13_pThis->m118_currentResult = 1;
            }
            else
            {
                r13_pThis->m118_currentResult = 0;
            }
            r14 += 2;
            break;
        case 21:
        {
            s8 arg = readSaturnS8(r14++);
            r14 = getAlignOn4(r14);
            if ((r13_pThis->m118_currentResult < readSaturnU8(r14)) && (r13_pThis->m118_currentResult >= 0))
            {
                r14 = readSaturnEA(r14 + r13_pThis->m118_currentResult * 4);
            }
            else
            {
                r14 += arg * 4;
            }
            break;
        }
        case 24: //setResult
            r13_pThis->m118_currentResult = varC->m4;
            break;
        case 32: // wait fade
        {
            if (!g_fadeControls.m0_fade0.m20_stopped)
            {
                return r14 - 1;
            }
            break;
        }
        case 36: // display string
            if (r13_pThis->m16C_displayStringTask)
            {
                r13_pThis->m16C_displayStringTask->getTask()->markFinished();
                return r14 - 1;
            }
            else
            {
                r14 = getAlignOn2(r14);
                s16 duration = readSaturnS16(r14);
                r14 += 2;
                r14 = getAlignOn4(r14);

                sSaturnPtr stringPtr = readSaturnEA(r14);
                r14 += 4;

                createDisplayStringBorromScreenTask(currentResTask, &r13_pThis->m16C_displayStringTask, duration, stringPtr);
            }
            break;
        default:
            assert(0);
        }
    } while (1);
}

void scriptUpdateRunScript()
{
    s32 r8_numExtraScriptsIterations = npcData0.m0_numBackgroundScripts;

    if (!npcData0.m104_currentScript.m0_scriptPtr.isNull())
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & (graphicEngineStatus.m4514.mD8_buttonConfig[0][1] | graphicEngineStatus.m4514.mD8_buttonConfig[0][2]))
            && (npcData0.mF4 == 0))
        {
            npcData0.m100 = 0;
            npcData0.mF8 = 0;
            npcData0.mF0 = 0;
        }

        //600CF06
        if (npcData0.m100)
        {
            --npcData0.m100;
            return;
        }

        npcData0.m104_currentScript.m0_scriptPtr = runScript(&npcData0);
        if (npcData0.m104_currentScript.m0_scriptPtr.isNull())
        {
            npcData0.mFC &= ~0x1F;
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        }
        else
        {
            npcData0.m0_numBackgroundScripts = 0;
        }
        return;
    }
    else
    {
        // 0x600CF48
        if (r8_numExtraScriptsIterations)
        {
            npcData0.mF4 = 0;
            npcData0.mF0 = 0;

            std::array<sRunningScriptContext, 4>::iterator r9 = npcData0.m4_backgroundScripts.begin();

            do
            {
                npcData0.m104_currentScript = *r9;

                npcData0.m104_currentScript.m0_scriptPtr = runScript(&npcData0);
                if (!npcData0.m104_currentScript.m0_scriptPtr.isNull())
                {
                    npcData0.mFC |= 0xF;
                    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
                    npcData0.m0_numBackgroundScripts = 0;
                }

                r9++;
            } while (--r8_numExtraScriptsIterations);
        }
    }
}

void sScriptTask::Init(sScriptTask* pThis)
{
    resData.m0 = 0;
    initResTable();
    copyCameraPropertiesToRes();
}

void scriptUpdateSub1()
{
    resCameraProperties.m10 = 0;
    resCameraProperties.m28_LCSDepth = graphicEngineStatus.m405C.m14_farClipDistance;
    resCameraProperties.m1C_LCSHeightMin = -(resCameraProperties.m4_LCS_Y.getInteger() - resCameraProperties.mC_LCSHeight); // todo: there might be a bug here, used to be clipped to 16 bit
    resCameraProperties.m1E_LCSHeightMax = -(resCameraProperties.m4_LCS_Y.getInteger() + resCameraProperties.mC_LCSHeight);
    resCameraProperties.m20_LCSWidthMin = resCameraProperties.m0_LCS_X.getInteger() - resCameraProperties.m8_LCSWidth;
    resCameraProperties.m22_LCSWidthMax = resCameraProperties.m0_LCS_X.getInteger() + resCameraProperties.m8_LCSWidth;

    s32 var0[2];
    var0[0] = resCameraProperties.m20_LCSWidthMin;
    var0[1] = resCameraProperties.m2C;
    fixedPoint r12 = FP_Div(0x10000, resCameraProperties.m2C);
    resCameraProperties.m34_boundMinX = MTH_Mul(resCameraProperties.m20_LCSWidthMin, r12);
    resCameraProperties.m38_radiusScaleMinX = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    var0[0] = resCameraProperties.m22_LCSWidthMax;
    resCameraProperties.m3C_boundMaxX = MTH_Mul(resCameraProperties.m22_LCSWidthMax, r12);
    resCameraProperties.m40_radiusScaleMaxX = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    // same for height
    var0[0] = -resCameraProperties.m1C_LCSHeightMin;
    var0[1] = resCameraProperties.m30;
    r12 = FP_Div(0x10000, resCameraProperties.m30);
    resCameraProperties.m44_boundMinY = MTH_Mul(resCameraProperties.m1C_LCSHeightMin, r12);
    resCameraProperties.m48_radiusScaleMinY = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    var0[0] = -resCameraProperties.m1E_LCSHeightMax;
    resCameraProperties.m4C_boundMaxY = MTH_Mul(resCameraProperties.m1E_LCSHeightMax, r12);
    resCameraProperties.m50_radiusScaleMaxY = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    resCameraProperties.m54 = MTH_Mul(resCameraProperties.m38_radiusScaleMinX, gTownGrid.m2C);
    resCameraProperties.m58 = MTH_Mul(resCameraProperties.m40_radiusScaleMaxX, gTownGrid.m2C);
}

const std::array<s8[2],25> townGridSearchPattern = {
    {{ 0,    0},
    { 1,    0},
    { 0,    1},
    {-1,    0},
    { 0,   -1},
    { 1,    1},
    {-1,    1},
    {-1,   -1},
    { 1,   -1},
    { 2,    0},
    { 0,    2},
    {-2,    0},
    { 0,   -2},
    { 2,    1},
    {-1,    2},
    {-2,   -1},
    { 1,   -2},
    { 2,   -1},
    { 1,    2},
    {-2,    1},
    {-1,   -2},
    { 2,    2},
    {-2,    2},
    {-2,   -2},
    { 2,  -2}}
};

void scriptUpdateSub2Sub0(sResCameraProperties* r4, s32 r5)
{
    sVec3_FP var14;
    var14[0] = pCurrentMatrix->matrix[1];
    var14[1] = pCurrentMatrix->matrix[5];
    var14[2] = pCurrentMatrix->matrix[9];

    sVec2_FP var0;
    var0[0] = performDivision(r4->m2C, r4->m0_LCS_X);
    var0[1] = performDivision(r4->m30, r4->m4_LCS_Y);

    fixedPoint r12 = var14[2] + MulVec2(sVec2_FP({ var14[0], var14[1] }), var0);
    if (r12 >= 0)
    {
        return;
    }

    sVec3_FP var8;
    var8[0] = pCurrentMatrix->matrix[3] + MTH_Mul(r5, pCurrentMatrix->matrix[1]);
    var8[1] = pCurrentMatrix->matrix[7] + MTH_Mul(r5, pCurrentMatrix->matrix[5]);
    var8[2] = pCurrentMatrix->matrix[11] + MTH_Mul(r5, pCurrentMatrix->matrix[9]);

    fixedPoint r4_fp = FP_Div(MTH_Product3d_FP(var14, var8), r12);
    if (r4->m28_LCSDepth > r4_fp)
    {
        r4->m10 = 0;
        r4->m14 = 0;
        r4->m28_LCSDepth = r4_fp;
    }
}

struct sBundleEntry
{
    u8* m_bundleBase;
    u8* m_entry;
};

sBundleEntry getBundleEntry(u8* pBundleBase, u32 entryOffset)
{
    u32 offsetInBundle = READ_BE_U32(pBundleBase + entryOffset);
    u8* pEntry = pBundleBase + offsetInBundle;

    sBundleEntry newEntry;
    newEntry.m_bundleBase = pBundleBase;
    newEntry.m_entry = pEntry;

    return newEntry;
}

struct sTransformedVertice
{
    s16 m0_X;
    s16 m2_Y;
    s32 m4_fullPrecisionZ;
    s32 m8_fullPrecisionX;
    s32 mC_fullPrecisionY;
    s32 m10_clippedX;
    s32 m14_clippedY;
    u32 m18_clipFlags;
    u32 m1C_clipFlags2;
    // size 0x20
};

void computeFinalProjectionMatrix(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6, std::array<s16, 9>& outputMatrix, s32& r8_outputTranslationX, s32& r9_outputTranslationY, s32& r10_outputTranslationZ, s16*& r11, std::vector<sVec3_S16_12_4>::const_iterator& r12_pVertices, u32& r13_numVertices)
{
    //r6 is off by 0x14 compared to asm
    outputMatrix[0] = (r5.matrix[0] * (r6->m2C * 16)) >> 16;
    outputMatrix[1] = (r5.matrix[1] * (r6->m2C * 16)) >> 16;
    outputMatrix[2] = (r5.matrix[2] * (r6->m2C * 16)) >> 16;

    r8_outputTranslationX = (r5.matrix[3] * r6->m2C);

    outputMatrix[3] = (r5.matrix[4] * (r6->m30 * 16)) >> 16;
    outputMatrix[4] = (r5.matrix[5] * (r6->m30 * 16)) >> 16;
    outputMatrix[5] = (r5.matrix[6] * (r6->m30 * 16)) >> 16;

    r9_outputTranslationY = (r5.matrix[7] * -r6->m30);

    outputMatrix[6] = r5.matrix[8] >> 4;
    outputMatrix[7] = r5.matrix[9] >> 4;
    outputMatrix[8] = r5.matrix[10] >> 4;

    r10_outputTranslationZ = r5.matrix[11] << 8;

    r11 = &outputMatrix[6];
    r13_numVertices = r4.m4_numVertices;
    r12_pVertices = r4.m8_vertices.begin();

    // note: this also init DIV register
}

void transformVerticesCliped(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6, std::array<sTransformedVertice, 256> & r7_transformedVertices)
{
    std::array<s16,9> projectionMatrix;
    s32 r8_translationX;
    s32 r9_translationY;
    s32 r10_translationZ;
    s16* dummy_r11;
    std::vector<sVec3_S16_12_4>::const_iterator dummy_r12;
    u32 r13_numVertices;
    computeFinalProjectionMatrix(r4, r5, r6, projectionMatrix, r8_translationX, r9_translationY, r10_translationZ, dummy_r11, dummy_r12, r13_numVertices);
    std::vector<sVec3_S16_12_4>::const_iterator r12_pVertices = r4.m8_vertices.begin(); // this is moved out of the previous function;
    // r14 is now 0xFFFFFF00 hardware

    std::array<sTransformedVertice, 256>::iterator r7 = r7_transformedVertices.begin();

    s64 mac = 0;
    s32 DVDNTUL = 0;
    s32 r1 = 0;
    s32 r0 = 0;
    do
    {
        //Z
        mac = r10_translationZ;
        mac += ((s64)(projectionMatrix[6]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[7]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[8]) * (s64)(r12_pVertices->m_value[2]));
        DVDNTUL = 0x10000000000 / (mac & 0xFFFFFFFF);
        r7->m4_fullPrecisionZ = mac;

        //X
        mac = r8_translationX;
        mac += ((s64)(projectionMatrix[0]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[1]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[2]) * (s64)(r12_pVertices->m_value[2]));
        r7->m8_fullPrecisionX = mac;

        //Y
        mac = r9_translationY;
        mac += ((s64)(projectionMatrix[3]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[4]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[5]) * (s64)(r12_pVertices->m_value[2]));
        r12_pVertices++;
        r7->mC_fullPrecisionY = mac;

        s32 r3 = r7->m4_fullPrecisionZ < (r6->m28_LCSDepth << 8);
        r3 <<= 1;

        s32 depthClipped = (r6->m24 << 8) > r7->m4_fullPrecisionZ;
        r3 |= depthClipped;

        if (!depthClipped)
        {
            //0601F36C
            s32 r2 = ((s64)DVDNTUL * (s64)r7->mC_fullPrecisionY) >> 32;
            s32 r1 = ((s64)DVDNTUL * (s64)r7->m8_fullPrecisionX) >> 32;

            r7->m2_Y = r2;
            r7->m0_X = r1;

            r3 <<= 1;
            r3 |= r2 > r6->m1C_LCSHeightMin;
            r3 <<= 1;
            r3 |= r2 < r6->m1E_LCSHeightMax;
            r3 <<= 1;
            r3 |= r1 > r6->m20_LCSWidthMin;
            r3 <<= 1;
            r3 |= r1 < r6->m22_LCSWidthMax;
            r7->m18_clipFlags = r3;
        }
        else
        {
            r7->m18_clipFlags = r3 << 4;
            
        }
        r7++;
    } while (--r13_numVertices);
}
bool testQuadsForCollisionSub2(sResCameraProperties* r6, sTransformedVertice& r8, sTransformedVertice& r9)
{
    s32 r0 = (r6->m24.asS32() << 8) - r8.m4_fullPrecisionZ;
    s32 r2 = r9.m4_fullPrecisionZ - r8.m4_fullPrecisionZ;

    s32 r1 = r0 / r2;
    r1 <<= 8;
    s64 mac = r1 * (r9.m8_fullPrecisionX - r8.m8_fullPrecisionX);
    r2 = mac >> 16;
    r2 += r8.m8_fullPrecisionX;

    mac = r1 * (r9.mC_fullPrecisionY - r8.mC_fullPrecisionY);
    s32 r3 = mac >> 16;
    r3 += r8.mC_fullPrecisionY;

    r8.m14_clippedY = r6->m18 * r3;
    r8.m10_clippedX = r6->m18 * r2;
    return false;
}

bool testQuadsForCollisionSub1(s32 r1, sResCameraProperties* r6, bool(*r2Callback)(sResCameraProperties* r6, sTransformedVertice&, sTransformedVertice&), sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    if (r13.m1C_clipFlags2 & r1)
    {
        //601F7E0
        if (r12.m1C_clipFlags2 & r1)
        {
            //601F828
            if (r11.m1C_clipFlags2 & r1)
            {
                //0601F852
                if (r10.m1C_clipFlags2 & r1)
                {
                    // all clipped!
                    return true;
                }
                //601F852
                //r13 r12 r11 clipped
                r2Callback(r6, r11, r10);
                r2Callback(r6, r12, r10);
                return r2Callback(r6, r13, r10);
            }
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F840
                //r13 r12 r10 clipped
                r2Callback(r6, r10, r11);
                r2Callback(r6, r13, r11);
                return r2Callback(r6, r12, r11);
            }
            //0601F830
            // r13 r12 clipped
            r2Callback(r6, r13, r10);
            return r2Callback(r6, r12, r11);
        }
        if (r11.m1C_clipFlags2 & r1)
        {
            //0601F810
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F816
                //r13 r11 r10 clipped
                r2Callback(r6, r10, r12);
                r2Callback(r6, r13, r12);
                return r2Callback(r6, r11, r12);
            }
            //0601F812
            //r13 r11 clipped
            return true;
        }
        if (r10.m1C_clipFlags2 & r1)
        {
            //601F800
            // r13 r10 clipped
            r2Callback(r6, r10, r11);
            return r2Callback(r6, r13, r12);
        }

        // 601F7EE
        // only r13 clipped
        if (r12.m4_fullPrecisionZ > r10.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r13, r12);
        }
        else
        {
            return r2Callback(r6, r13, r10);
        }
    }
    if (r12.m1C_clipFlags2 & r1)
    {
        //601F7A2
        if (r11.m1C_clipFlags2 & r1)
        {
            //601F7BC
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F7CE
                // r12 r11 r10 clipped
                r2Callback(r6, r11, r13);
                r2Callback(r6, r12, r13);
                return r2Callback(r6, r10, r13);
            }
            //601F7BC
            // r12 r11 clipped
            r2Callback(r6, r11, r10);
            return r2Callback(r6, r12, r13);
        }

        if (r10.m1C_clipFlags2 & r1)
        {
            //601F812
            //r12 r10 clipped
            return true;
        }

        //601F7AA
        // only r12 clipped
        if (r13.m4_fullPrecisionZ > r11.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r12, r13);
        }
        else
        {
            return r2Callback(r6, r12, r11);
        }
    }
    if (r11.m1C_clipFlags2 & r1)
    {
        //601F77E
        if (r10.m1C_clipFlags2 & r1)
        {
            //601F792
            // r11 r10 clipped
            r2Callback(r6, r10, r13);
            return r2Callback(r6, r11, r12);
        }
        //601F780
        // only 11 clipped
        if (r10.m4_fullPrecisionZ > r12.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r11, r10);
        }
        else
        {
            return r2Callback(r6, r11, r12);
        }
    }
    if (r10.m1C_clipFlags2 & r1)
    {
        //601F76C
        // only r10 clipped
        if (r11.m4_fullPrecisionZ > r13.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r10, r11);
        }
        else
        {
            return r2Callback(r6, r10, r13);
        }
    }

    return false;
}

bool setTBitIfCollisionWithQuadSub0(sResCameraProperties* r6, sTransformedVertice& r8_input, sTransformedVertice& r9_input)
{
    s32 r2 = r9_input.m10_clippedX - r8_input.m10_clippedX;
    s32 r14 = r9_input.m14_clippedY - r8_input.m14_clippedY;

    s32 r8 = r6->m1C_LCSHeightMin - r8_input.m14_clippedY;
    s32 r9 = r6->m1E_LCSHeightMax - r8_input.m14_clippedY;

    r8 *= r2;
    s32 r3 = (r6->m20_LCSWidthMin - r8_input.m10_clippedX) * r14;

    if (r8 > r3)
    {
        return false;
    }

    r9 *= r2;
    if (r9 > r3)
    {
        return false;
    }

    r3 = (r6->m22_LCSWidthMax - r8_input.m10_clippedX) * r14;

    if (r8 > r3)
    {
        return false;
    }
    if (r9 > r3)
    {
        return false;
    }

    return true;
}

bool setTBitIfCollisionWithQuad(sResCameraProperties* r6, sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    // if all points have at least one common clip flag, the quad is clipped
    if (r10.m18_clipFlags & r11.m18_clipFlags & r12.m18_clipFlags & r13.m18_clipFlags)
        return true;

    // if at least a single vertices is clipped by 0x20 (far clip?), the quad is clipped
    if ((r10.m18_clipFlags | r11.m18_clipFlags | r12.m18_clipFlags | r13.m18_clipFlags) & 0x20)
        return true;

    r10.m1C_clipFlags2 = r10.m18_clipFlags;
    r11.m1C_clipFlags2 = r11.m18_clipFlags;
    r12.m1C_clipFlags2 = r12.m18_clipFlags;
    r13.m1C_clipFlags2 = r13.m18_clipFlags;

    r10.m10_clippedX = r10.m0_X;
    r10.m14_clippedY = r10.m2_Y;

    r11.m10_clippedX = r11.m0_X;
    r11.m14_clippedY = r11.m2_Y;

    r12.m10_clippedX = r12.m0_X;
    r12.m14_clippedY = r12.m2_Y;

    r13.m10_clippedX = r13.m0_X;
    r13.m14_clippedY = r13.m2_Y;

    if (!testQuadsForCollisionSub1(0x10, r6, testQuadsForCollisionSub2, r10, r11, r12, r13))
    {
        s32 r0 = (r13.m14_clippedY - r11.m14_clippedY) * (r12.m10_clippedX - r10.m10_clippedX);
        s32 r2 = (r12.m14_clippedY - r10.m14_clippedY) * (r13.m10_clippedX - r11.m10_clippedX);

        if (r2 > r0)
        {
            return true;
        }

        if (setTBitIfCollisionWithQuadSub0(r6, r10, r11))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r12, r13))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r11, r12))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r13, r10))
            return true;

        return false;
    }
    return true;
}

void testQuadsForCollisionSub0(const sProcessed3dModel::sQuad& r4, s32& r5_result, sResCameraProperties* r6, sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    s16 r1 = r10.m4_fullPrecisionZ;
    s16 r2 = r11.m4_fullPrecisionZ;
    s16 r3 = r12.m4_fullPrecisionZ;
    s16 r8 = r13.m4_fullPrecisionZ;

    u8 r0 = r4.m8_lightingControl & 0xF0;
    switch (r0)
    {
    case 0:
        //601E634
        // basically max of r1/r2/r3/r8
        if (r1 < r2)
        {
            r1 = r2;
        }
        if (r3 < r8)
        {
            r3 = r8;
        }
        if (r1 > r3)
        {
            r1 = r3;
        }
        break;
    case 0x10:
        //601E648
        if (r1 >= r2)
        {
            r1 = r2;
        }
        if (r3 >= r8)
        {
            r3 = r8;
        }
        if (r1 >= r3)
        {
            r1 = r3;
        }
        break;
    default:
        r1 += r2;
        r1 += r3;
        r1 += r8;
        r1 >>= 2;
        break;
    }

    //601E65A
    r1 <<= 8;
    if (r1 < r6->m28_LCSDepth)
    {
        r6->m28_LCSDepth = r1;
        r5_result = r4.m12_CMDSIZE;
    }
}

void testQuadsForCollision(const std::vector<sProcessed3dModel::sQuad>& r4_vector, s32& r5_result, sResCameraProperties* r6, std::array<sTransformedVertice, 256>& r7_transformeVertices)
{
    std::vector<sProcessed3dModel::sQuad>::const_iterator r4 = r4_vector.begin();
    do 
    {
        if ((r4->m8_lightingControl & 0xF0) == 0)
        {
            sTransformedVertice& r10 = r7_transformeVertices[r4->m0_indices[0]];
            sTransformedVertice& r11 = r7_transformeVertices[r4->m0_indices[1]];
            sTransformedVertice& r12 = r7_transformeVertices[r4->m0_indices[2]];
            sTransformedVertice& r13 = r7_transformeVertices[r4->m0_indices[3]];

            if (!setTBitIfCollisionWithQuad(r6, r10, r11, r12, r13))
            {
                testQuadsForCollisionSub0(*r4, r5_result, r6, r10, r11, r12, r13);
            }
        }

        r4++;
    } while (r4 != r4_vector.end());

}

s32 testMeshForCollision(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6)
{
    std::array<sTransformedVertice, 256> transformedVertices;
    transformVerticesCliped(r4, r5, r6, transformedVertices);
    // note: normally, r4 would have been increased by 0xC by the previous function (skip over to quad info)

    s32 r5_result = -1;

    testQuadsForCollision(r4.mC_Quads, r5_result, r6, transformedVertices);

    return r5_result;
}

s32 scriptUpdateSub2Sub1Sub0(sResCameraProperties* r14, const sProcessed3dModel& r11)
{
    // check depth
    if (pCurrentMatrix->matrix[11] < r14->m24 - r11.m0_radius)
        return -1;

    if (pCurrentMatrix->matrix[11] > r14->m28_LCSDepth + r11.m0_radius)
        return -1;

    // check X
    if (pCurrentMatrix->matrix[3] < MTH_Mul(pCurrentMatrix->matrix[11], r14->m34_boundMinX) - MTH_Mul(r11.m0_radius, r14->m38_radiusScaleMinX))
        return -1;

    if (pCurrentMatrix->matrix[3] > MTH_Mul(pCurrentMatrix->matrix[11], r14->m3C_boundMaxX) + MTH_Mul(r11.m0_radius, r14->m40_radiusScaleMaxX))
        return -1;

    //0601412C
    //check Y
    if (pCurrentMatrix->matrix[7] < MTH_Mul(pCurrentMatrix->matrix[11], r14->m44_boundMinY) - MTH_Mul(r11.m0_radius, r14->m48_radiusScaleMinY))
        return -1;

    if (pCurrentMatrix->matrix[7] > MTH_Mul(pCurrentMatrix->matrix[11], r14->m4C_boundMaxY) + MTH_Mul(r11.m0_radius, r14->m50_radiusScaleMaxY))
        return -1;

    return testMeshForCollision(r11, *pCurrentMatrix, r14); //we should be passing r14->m14, but that seems to be a handrolled hack to alias r14->m14 to s_graphicEngineStatus_405C
}

void scriptUpdateSub2Sub1(sResCameraProperties* r14, sEnvironmentTask* r12)
{
    if (r12 == nullptr)
        return;

    if (r12->m8.isNull())
        return;

    if (readSaturnS32(r12->m8 + 0x14) == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(r12->mC_position);
    if (
        (pCurrentMatrix->matrix[11] >= r14->m24 - gTownGrid.m2C) && (pCurrentMatrix->matrix[11] <= r14->m2C + gTownGrid.m2C) &&
        (pCurrentMatrix->matrix[3] >= MTH_Mul(pCurrentMatrix->matrix[11], r14->m34_boundMinX) - r14->m54) && (pCurrentMatrix->matrix[3] <= MTH_Mul(pCurrentMatrix->matrix[11], r14->m3C_boundMaxX) + r14->m58)
        )
    {
        sSaturnPtr r13 = readSaturnEA(r12->m8 + 0x14);
        while (readSaturnS32(r13))
        {
            pushCurrentMatrix();
            translateCurrentMatrix(readSaturnVec3(r13 + 4));
            sProcessed3dModel model(r12->m0_dramAllocation, READ_BE_U32(r12->m0_dramAllocation + readSaturnU32(r13)));
            s32 r0 = scriptUpdateSub2Sub1Sub0(r14, model);
            if (r0 != -1)
            {
                if (r0)
                {
                    r14->m10 = pCurrentMatrix->matrix[3];
                    r14->m14 = (sMainLogic_74*)r0;
                }
                else
                {
                    r14->m10 = 0;
                }
            }
            popMatrix();
            r13 += 0x10;
        }
    }
    //60140B8
    popMatrix();
}

const std::vector<sVec3_S16_12_4> townLCSVisibilityTestMeshIniVertices = {
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0}
};

std::vector<sProcessed3dModel::sQuad> townLCSVisibilityTestMeshIniQuad = {
    {
        {0,1,2,3},
        0x2900,
        0x5,
        0xE8,
        0,
        0,
        0,
        {
            {{0,0x1000,0}, {0,0,0}}
        }
    },
};

sProcessed3dModel townLCSVisibilityTestMesh(fixedPoint(0), townLCSVisibilityTestMeshIniVertices, townLCSVisibilityTestMeshIniQuad);

void scriptUpdateSub2()
{
    scriptUpdateSub2Sub0(&resCameraProperties, resData.m0);

    for (int i = 0; i < townGridSearchPattern.size(); i++)
    {
        scriptUpdateSub2Sub1(&resCameraProperties, gTownGrid.m40_cellTasks[(gTownGrid.mC + townGridSearchPattern[i][1]) & 7][(gTownGrid.m8 + townGridSearchPattern[i][0]) & 7]);
    }

    sResData* var10 = &resData;
    sProcessed3dModel& var4 = townLCSVisibilityTestMesh;

    sVec3_S16_12_4& r10 = townLCSVisibilityTestMesh.m8_vertices[0];
    sVec3_S16_12_4& r8 = townLCSVisibilityTestMesh.m8_vertices[1];
    sVec3_S16_12_4& var0 = townLCSVisibilityTestMesh.m8_vertices[2];
    sVec3_S16_12_4& r9 = townLCSVisibilityTestMesh.m8_vertices[3];

    for(int i=0; i<5; i++)
    {
        sResData1C* r12 = var10->m8_headOfLinkedList[i];
        while (r12)
        {
            sMainLogic_74* r14 = r12->m4;
            r12 = r12->m0_pNext;
            if (r14->m1)
            {
                var4.m0_radius = r14->m4_collisionRadius;
                
                s16 r4 = r14->m14[0] >> 4;
                r10[0] = r4;
                r9[0] = r4;

                var0[0] = -r4;
                r8[0] = -r4;

                r4 = r14->m14[1] >> 4;
                r8[1] = r4;
                r10[1] = r4;

                r9[1] = r4;
                var0[1] = r4;

                var0[2] = 0;
                r9[2] = 0;
                r8[2] = 0;
                r10[2] = 0;

                pushCurrentMatrix();
                translateCurrentMatrix(r14->m8_position);
                sMatrix4x3& r5 = cameraProperties2.m88_billboardViewMatrix;

                pCurrentMatrix->matrix[0] = r5.matrix[0];
                pCurrentMatrix->matrix[1] = r5.matrix[1];
                pCurrentMatrix->matrix[2] = r5.matrix[2];
                pCurrentMatrix->matrix[4] = r5.matrix[4];
                pCurrentMatrix->matrix[5] = r5.matrix[5];
                pCurrentMatrix->matrix[6] = r5.matrix[6];
                pCurrentMatrix->matrix[8] = r5.matrix[8];
                pCurrentMatrix->matrix[9] = r5.matrix[9];
                pCurrentMatrix->matrix[10] = r5.matrix[10];

                if(scriptUpdateSub2Sub1Sub0(&resCameraProperties, var4))
                {
                    if (!r14->m3C_scriptEA.isNull())
                    {
                        resCameraProperties.m10 = 2;
                        resCameraProperties.m14 = r14;
                    }
                    else
                    {
                        resCameraProperties.m10 = 0;
                    }
                }
                popMatrix();
            }
        }
    }
}

void sScriptTask::Update(sScriptTask* pThis)
{
    if (pThis->m18)
    {
        pThis->m18--;
    }

    pThis->m8_activationType = 0;
    pThis->m4 = sSaturnPtr::getNull();
    if (!(npcData0.mFC & 0x10))
    {
        pThis->m8_activationType = 0;
        pThis->m4 = sSaturnPtr::getNull();
    }
    else if(!(npcData0.mFC & 1))
    {
        if (pThis->m8_activationType == 2)
        {
            assert(0);
        }

        //60305A8
        scriptUpdateSub1();
        scriptUpdateSub2();

        switch (resCameraProperties.m10)
        {
        case 1:
        case 2:
            pThis->m8_activationType = resCameraProperties.m10;
            pThis->mC = resCameraProperties.m14;
            break;
        default:
            break;
        }

        sScriptTask* var0;

        //06030618
        sSaturnPtr r13 = sSaturnPtr::getNull();
        p_workArea r11 = nullptr;
        switch (pThis->m8_activationType)
        {
        case 0:
            r13 = sSaturnPtr::getNull();
            break;
        case 2:
            var0 = pThis;
            pThis->m10_distanceToLCS = vecDistance(pThis->mC->m8_position, *npcData0.m160_pEdgePosition);
            r13 = pThis->mC->m3C_scriptEA;
            r11 = pThis->mC->m38_pOwner;
            break;
        default:
            assert(0);
            break;
        }

        //06030670
        if (!r13.isNull())
        {
            addBackgroundScript(r13, 2, r11, nullptr);
            if (pThis->m4 != r13)
            {
                pThis->m18 = 8;
                pThis->m1C_LCS_X = resCameraProperties.m0_LCS_X.getInteger();
                pThis->m1E_LCS_Y = resCameraProperties.m4_LCS_Y.getInteger();
                playSoundEffect(0x24);
            }
        }

        pThis->m4 = r13;
    }

    // 060306B4
    scriptUpdateSub0();
    initResTable();
    scriptUpdateRunScript();
}

void sScriptTask::Draw(sScriptTask* pThis)
{
    FunctionUnimplemented();
}

void startScriptTask(p_workArea r4)
{
    currentResTask = createSubTask<sScriptTask>(r4);
}
