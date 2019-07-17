#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "townLCS.h"
#include "town/ruin/twn_ruin.h"

#include <map>

sResData resData;

s32 resValue0 = 0;
sScriptTask* currentResTask = nullptr;

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

sTownCellTask* scriptUpdateSub0Sub3Sub0(fixedPoint r4_x, fixedPoint r5_z)
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

void scriptUpdateSub0Sub3Sub2(sMainLogic_74* r12, sTownCellTask* r13)
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
        case 8: //equal
            r14 = getAlignOn2(r14);
            if (readSaturnS16(r14) == r13_pThis->m118_currentResult)
            {
                r13_pThis->m118_currentResult = 1;
            }
            else
            {
                r13_pThis->m118_currentResult = 0;
            }
            r14 += 2;
            break;
        case 9: //not equal
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

sVec3_FP getEnvLCSTargetPosition(s32 index)
{
    sVec3_FP position;
    if (index >= npcData0.m68_numEnvLCSTargets)
    {
        position.zero();
    }
    else
    {
        position = readSaturnVec3(npcData0.m6C_LCSTargets + index * 12);
    }

    return position;
}

void sScriptTask::Update(sScriptTask* pThis)
{
    if (pThis->m18_LCSFocusLineScale)
    {
        pThis->m18_LCSFocusLineScale--;
    }

    pThis->m8_currentLCSType = 0;
    pThis->m4 = sSaturnPtr::getNull();
    if (!(npcData0.mFC & 0x10))
    {
        pThis->m8_currentLCSType = 0;
        pThis->m4 = sSaturnPtr::getNull();
    }
    else if(!(npcData0.mFC & 1))
    {
        if (pThis->m8_currentLCSType == 2)
        {
            assert(0);
        }

        //60305A8
        setupDataForLCSCollision();
        findLCSCollision();

        switch (LCSCollisionData.m10_activeLCSType)
        {
        case 1:
            pThis->m8_currentLCSType = LCSCollisionData.m10_activeLCSType;
            pThis->mC_AsIndex = LCSCollisionData.m14_activeLCSEnvironmentIndex;
            break;
        case 2:
            pThis->m8_currentLCSType = LCSCollisionData.m10_activeLCSType;
            pThis->mC = LCSCollisionData.m14_activeLCS;
            break;
        default:
            break;
        }

        sScriptTask* var0;

        //06030618
        sSaturnPtr r13 = sSaturnPtr::getNull();
        p_workArea r11 = nullptr;
        switch (pThis->m8_currentLCSType)
        {
        case 0: // nothing
            r13 = sSaturnPtr::getNull();
            break;
        case 1: // environment object
            var0 = pThis;
            pThis->m10_distanceToLCS = vecDistance(getEnvLCSTargetPosition(pThis->mC_AsIndex), *npcData0.m160_pEdgePosition);
            r13 = readSaturnEA(npcData0.m64_scriptList + pThis->mC_AsIndex * 4);
            r11 = nullptr;
            break;
        case 2: // NPC object
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
                pThis->m18_LCSFocusLineScale = 8;
                pThis->m1C_LCS_X = LCSCollisionData.m0_LCS_X.getInteger();
                pThis->m1E_LCS_Y = LCSCollisionData.m4_LCS_Y.getInteger();
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

void updateTownLCSTargetPosition(sScriptTask* pThis)
{
    if (pThis->m8_currentLCSType == 0)
        return;

    pushCurrentMatrix();
    switch (pThis->m8_currentLCSType)
    {
    case 1:
        translateCurrentMatrix(getEnvLCSTargetPosition(pThis->mC_AsIndex));
        break;
    case 2:
        translateCurrentMatrix(pThis->mC->m8_position);
        if (pThis->mC->m2C == 3)
        {
            adjustMatrixTranslation(pThis->mC->m14[1] / 2);
        }
        break;
    default:
        assert(0);
        break;
    }

    if ((pCurrentMatrix->matrix[11] >= graphicEngineStatus.m405C.m14_farClipDistance) || (pCurrentMatrix->matrix[11] < graphicEngineStatus.m405C.m10_nearClipDistance))
    {
        pThis->m8_currentLCSType = 0;
    }
    else
    {
        sVec2_S16 LCSScreenCoordinates;
        LCSScreenCoordinates[0] = setDividend(graphicEngineStatus.m405C.m18_widthScale, pCurrentMatrix->matrix[3], pCurrentMatrix->matrix[11]);
        LCSScreenCoordinates[1] = setDividend(graphicEngineStatus.m405C.m1C_heightScale, pCurrentMatrix->matrix[7], pCurrentMatrix->matrix[11]);

        if (
            (LCSScreenCoordinates[0] < graphicEngineStatus.m405C.mC - 0x10) || (LCSScreenCoordinates[0] > graphicEngineStatus.m405C.mE + 0x10) ||
            (LCSScreenCoordinates[1] < graphicEngineStatus.m405C.mA - 0x10) || (LCSScreenCoordinates[1] > graphicEngineStatus.m405C.m8 + 0x10))
        {
            pThis->m8_currentLCSType = 0;
        }
        else
        {
            pThis->m14_LCS = LCSScreenCoordinates;
        }
    }

    popMatrix();
}

void drawLineRectangle(u8 r4, s16* position, u32 color)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1005); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x400 | r4); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, color); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x0C, position[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -position[1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x010, position[2]); // CMDXC
    setVdp1VramU16(vdp1WriteEA + 0x012, -position[3]); // CMDYX
    setVdp1VramU16(vdp1WriteEA + 0x014, position[4]);
    setVdp1VramU16(vdp1WriteEA + 0x016, -position[5]);
    setVdp1VramU16(vdp1WriteEA + 0x018, position[6]);
    setVdp1VramU16(vdp1WriteEA + 0x01A, -position[7]);

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void sScriptTask::Draw(sScriptTask* pThis)
{
    if (pThis->m18_LCSFocusLineScale)
    {
        s16 var8[8];
        var8[0] = var8[6] = pThis->m1C_LCS_X + performDivision(8, (-pThis->m1C_LCS_X - 176) * pThis->m18_LCSFocusLineScale); // X1
        var8[1] = var8[3] = pThis->m1E_LCS_Y + performDivision(8, (-pThis->m1E_LCS_Y - 112) * pThis->m18_LCSFocusLineScale); // Y1
        var8[2] = var8[4] = pThis->m1C_LCS_X + performDivision(8, (-pThis->m1C_LCS_X + 176) * pThis->m18_LCSFocusLineScale); // X2
        var8[5] = var8[7] = pThis->m1E_LCS_Y + performDivision(8, (-pThis->m1E_LCS_Y + 112) * pThis->m18_LCSFocusLineScale); // Y2

        drawLineRectangle(0xC0, var8, (((pThis->m18_LCSFocusLineScale * 3) + 7) << 10) | ((pThis->m18_LCSFocusLineScale + 2) << 5) | (pThis->m18_LCSFocusLineScale + 2) | 0x8000);
    }
    
    updateTownLCSTargetPosition(pThis);
}

void startScriptTask(p_workArea r4)
{
    currentResTask = createSubTask<sScriptTask>(r4);
}
