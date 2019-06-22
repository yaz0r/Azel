#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "town/ruin/twn_ruin.h"

#include <map>

struct sResData
{
    s32 m0;
    s32 m4;
    std::array<s32, 4>m8;
}resData;

s32 resValue0 = 0;
sScriptTask* currentResTask = nullptr;

struct sResCameraProperties
{
    s32 m8;
    s32 mC;
    s32 m10;
    fixedPoint m18;
    fixedPoint m24;
    fixedPoint m2C;
    fixedPoint m30;
}resCameraProperties;

void copyCameraPropertiesToRes()
{
    resCameraProperties.m8 = 2;
    resCameraProperties.mC = 2;
    resCameraProperties.m10 = 0;
    resCameraProperties.m18 = graphicEngineStatus.m405C.m30;
    resCameraProperties.m24 = graphicEngineStatus.m405C.m10;
    resCameraProperties.m2C = graphicEngineStatus.m405C.m18;
    resCameraProperties.m30 = graphicEngineStatus.m405C.m1C;
}

void scriptUpdateSub0()
{
    for (int i = 0; i < 4; i++)
    {
        if (resData.m8[i])
        {
            assert(0);
        }
    }
}

void initResTable()
{
    resData.m8.fill(0);
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
        return npcData0.m10C;
    }
    
    return npcData0.m70_npcPointerArray[r4];
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
typedef s32(*scriptFunction_two_arg)(s32 arg0, s32 arg1);
typedef s32(*scriptFunction_four_arg)(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

struct sKernelScriptFunctions
{
    std::map<u32, scriptFunction_zero_arg> m_zeroArg;
    std::map<u32, scriptFunction_one_arg> m_oneArg;
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
    sSaturnPtr* varC = &r13_pThis->m104_scriptPtr;
    sSaturnPtr r14 = r13_pThis->m104_scriptPtr;

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
    s32 r8_numExtraScriptsIterations = npcData0.m0_numExtraScriptsIterations;

    if (npcData0.m104_scriptPtr.m_offset)
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

        npcData0.m104_scriptPtr = runScript(&npcData0);
        if (npcData0.m104_scriptPtr.m_offset == 0)
        {
            npcData0.mFC &= ~0x1F;
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        }
        else
        {
            npcData0.m0_numExtraScriptsIterations = 0;
        }
        return;
    }
    else
    {
        if (r8_numExtraScriptsIterations == 0)
            return;

        npcData0.mF4 = 0;
        npcData0.mF0 = 0;
    }

    // 0x600CF48
    // beginning of loop
    while (r8_numExtraScriptsIterations)
    {
        assert(0);
    }
}

void sScriptTask::Init(sScriptTask* pThis)
{
    resData.m0 = 0;
    initResTable();
    copyCameraPropertiesToRes();
}

void sScriptTask::Update(sScriptTask* pThis)
{
    if (pThis->m18)
    {
        pThis->m18--;
    }

    pThis->m8 = 0;
    pThis->m4 = 0;
    if (npcData0.mFC & 0x10)
    {
        assert(0);
    }

    // 060306B4
    scriptUpdateSub0();
    initResTable();
    scriptUpdateRunScript();
}

void sScriptTask::Draw(sScriptTask* pThis)
{
    TaskUnimplemented();
}

void startScriptTask(p_workArea r4)
{
    currentResTask = createSubTask<sScriptTask>(r4);
}
