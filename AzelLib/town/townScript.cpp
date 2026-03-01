#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "townLCS.h"
#include "town/e006/twn_e006.h"
#include "town/ruin/twn_ruin.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/fileBundle.h"
#include "kernel/dialogTask.h"
#include "kernel/debug/trace.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "processModel.h" //TOOD: cleanup
#include "kernel/textDisplay.h"

#include <map>

s32 playBattleSoundEffect(s32 effectIndex); // TODO: cleanup

sScriptTask* currentResTask = nullptr;


void addBackgroundScript(sSaturnPtr r4, s32 r5, p_workArea r6, const sVec3_S16_12_4* r7)
{
    if (r4.m_offset == 0)
        return;

    if (npcData0.m0_numBackgroundScripts >= 4)
        return;

    std::array<sRunningScriptContext, 4>::iterator r14 = npcData0.m4_backgroundScripts.begin();
    if (npcData0.m0_numBackgroundScripts)
    {
        for(s32 r13 = npcData0.m0_numBackgroundScripts; r13 > 0; r13--)
        {
            if (r14->m0_scriptPtr == r4)
            {
                if (r14->m4 == r5)
                {
                    return;
                }
            }
            r14++;
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

p_workArea getNpcDataByIndexAsTask(s32 r4)
{
    if (r4 == -1)
    {
        return npcData0.m104_currentScript.m8_owner.getAsTask();
    }

    assert(0);
    return NULL;
    //return npcData0.m70_npcPointerArray[r4].pNPC;
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

s32 terminateTown()
{
    if (townDebugTask2)
    {
        townDebugTask2->getTask()->markFinished();
    }

    closeAllOpenFiles();

    return 0;
}

s32 getNpcData0_5d() {
    return npcData0.m5D;
}

sKernelScriptFunctions gKernelScriptFunctions =
{
    // zero arg
    {
        {0x602C1D0, &fadeOutAllSequences},
        {0x603011E, &terminateTown},
        {0x600CDD4, &getNpcData0_5d},
    },
    // one arg
    {
        {0x600CCB4, &initNPC},
        {0x602c2ca, &playSystemSoundEffect},
        {0x6027110, &setNextGameStatus},
        {0x602c32a, &playBattleSoundEffect},
    },
    // one arg ptr
    {
        {0x6014DF2, &initNPCFromStruct},
    },
    // two arg
    {
        {0x600CC78, &setSomethingInNpc0},
        {0x60144c0, &mainLogicUpdateSub0}
    },
    // three arg
    {

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

    sSaturnPtr functionEA = readSaturnEA(r14);

    switch (numArguments)
    {
    case 0:
        if (gKernelScriptFunctions.m_zeroArg.count(functionEA.m_offset))
        {
            scriptFunction_zero_arg& pFunction = gKernelScriptFunctions.m_zeroArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction();
        }
        else if(gCurrentTownOverlay->overlayScriptFunctions.m_zeroArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_zero_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_zeroArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction();
        }
        else
        {
            Unimplemented();
        }
        break;
    case 1:
        if (gKernelScriptFunctions.m_oneArg.count(functionEA.m_offset))
        {
            scriptFunction_one_arg& pFunction = gKernelScriptFunctions.m_oneArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4));
        }
        else if (gKernelScriptFunctions.m_oneArgPtr.count(functionEA.m_offset))
        {
            scriptFunction_one_arg_ptr pFunction = gKernelScriptFunctions.m_oneArgPtr.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnEA(r14 + 4));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_oneArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_one_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_oneArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_oneArgPtr.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_one_arg_ptr& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_oneArgPtr.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnEA(r14 + 4));
        }
        else
        {
            Unimplemented();
        }
        break;
    case 2:
        if (gKernelScriptFunctions.m_twoArg.count(functionEA.m_offset))
        {
            scriptFunction_two_arg pFunction = gKernelScriptFunctions.m_twoArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_twoArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_two_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_twoArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        else
        {
            Unimplemented();
        }
        break;
    case 3:
        if (gKernelScriptFunctions.m_threeArg.count(functionEA.m_offset))
        {
            scriptFunction_three_arg pFunction = gKernelScriptFunctions.m_threeArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8), readSaturnS32(r14 + 12));
        }
        else
        {
            Unimplemented();
        }
        break;
    case 4:
        if (gKernelScriptFunctions.m_fourArg.count(functionEA.m_offset))
        {
            scriptFunction_four_arg pFunction = gKernelScriptFunctions.m_fourArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8), readSaturnS32(r14 + 12), readSaturnS32(r14 + 16));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_fourArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_four_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_fourArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8), readSaturnS32(r14 + 12), readSaturnS32(r14 + 16));
        }
        else
        {
            Unimplemented();
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

        if (isTraceEnabled())
        {
            addTraceLog("runScript_interceptOpcode 0x%02X\n", r0_opcode);
        }

        switch (r0_opcode)
        {
        case 1: //end
            if (r13_pThis->m11C_currentStackPointer == r13_pThis->m120_stack.end())
            {
                return sSaturnPtr::getNull();
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

            if (r13_pThis->mF0 && r13_pThis->mF4 && !r13_pThis->mF8) // skip delay
            {
                r13_pThis->m100 = 0;
            }
            else
            {
                r13_pThis->m100 = delay - 1;
            }
            return r4;
        }
        case 3: //jump
            r14 = readSaturnEA(getAlignOn4(r14));
            break;
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

            // hack: this isn't in the original assembly, but allow returning null for unhandled functions and just stopping the script
            //if (r14.isNull())
            //    return r14;
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
        case 10://greater
            r14 = getAlignOn2(r14);
            r13_pThis->m118_currentResult = readSaturnS16(r14) < r13_pThis->m118_currentResult;
            r14 += 2;
            break;
        case 11:
            r14 = getAlignOn2(r14);
            r13_pThis->m118_currentResult = readSaturnS16(r14) <= r13_pThis->m118_currentResult;
            r14 += 2;
            break;
        case 12: // less
            r14 = getAlignOn2(r14);
            r13_pThis->m118_currentResult = readSaturnS16(r14) > r13_pThis->m118_currentResult;
            r14 += 2;
            break;
        case 13:
            r14 = getAlignOn2(r14);
            r13_pThis->m118_currentResult = readSaturnS16(r14) >= r13_pThis->m118_currentResult;
            r14 += 2;
            break;
        case 15:
        {
            r14 = getAlignOn2(r14);
            s16 r4 = readSaturnS16(r14);
            r14 += 2;
            if (r4 < 1000)
            {
                r4 += 3334;
            }

            mainGameState.setBit(r4);
            break;
        }
        case 16:
        {
            r14 = getAlignOn2(r14);
            s16 r4 = readSaturnS16(r14);
            r14 += 2;
            if (r4 < 1000)
            {
                r4 += 3334;
            }

            mainGameState.clearBit(r4);
            break;
        }
        case 17:
        {
            r14 = getAlignOn2(r14);
            s16 r4 = readSaturnS16(r14);
            r14 += 2;
            if (r4 < 1000)
            {
                r4 += 3334;
            }

            r13_pThis->m118_currentResult = (mainGameState.getBit(r4) != 0);
            break;
        }
        case 18: // read packed bits
        {
            s8 r6 = readSaturnS8(r14++);
            r14 = getAlignOn2(r14);
            s16 var0 = readSaturnS16(r14);
            r14 += 2;
            if (var0 < 1000)
            {
                var0 += 3334;
            }
            r13_pThis->m118_currentResult = mainGameState.readPackedBits(var0, r6);
            break;
        }
        case 20: // add to packed bits var
        {
            s8 var4 = readSaturnS8(r14++);
            r14 = getAlignOn2(r14);
            s16 var0 = readSaturnS16(r14);
            r14 += 2;
            s16 var8 = readSaturnS16(r14);
            r14 += 2;
            if (var0 < 1000)
            {
                var0 += 3334;
            }
            s32 value = mainGameState.readPackedBits(var0, var4);
            value += var8;
            mainGameState.setPackedBits(var0, var4, value);
            break;
        }
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
        case 25: // add cinematic bars
            if (r13_pThis->m164_cinematicBars)
            {
                if (r13_pThis->m164_cinematicBars->m0_status != 1)
                {
                    return --r14;
                }
            }
            else
            {
                r13_pThis->m164_cinematicBars = createCinematicBarTask(currentResTask);
                cinematicBars_startClosing(r13_pThis->m164_cinematicBars, 4);
                return --r14;
            }
            break;
        case 26: // remove cinematic bars
            if (r13_pThis->m164_cinematicBars)
            {
                if (r13_pThis->m164_cinematicBars->m0_status == 1)
                {
                    r13_pThis->m164_cinematicBars->cinematicBars_startOpening(4);
                    return --r14;
                }
                else if(r13_pThis->m164_cinematicBars->m0_status)
                {
                    return --r14;
                }
                else
                {
                    r13_pThis->m164_cinematicBars->getTask()->markFinished();
                    r13_pThis->m164_cinematicBars = nullptr;
                }
                return r14;
            }
        case 27: // draw string
            r14 = getAlignOn4(r14);
            setupVDP2StringRendering(3, 25, 38, 2);
            VDP2DrawString(readSaturnString(readSaturnEA(r14)).c_str());
            r14 += 4;
            r13_pThis->mF8 = 1;
            break;
        case 29: // clear string
            setupVDP2StringRendering(3, 25, 38, 2);
            clearVdp2TextArea();
            break;
        case 31: // wait for native function to return != 0
            {
                sSaturnPtr r14Back = r14 - 1;
                r14 = callNativeWithArguments(r13_pThis, r14);
                if (r13_pThis->m118_currentResult == 0)
                {
                    return r14Back;
                }
            }
            break;
        case 32: // wait fade
        {
            if (!g_fadeControls.m0_fade0.m20_stopped)
            {
                return r14 - 1;
            }
            break;
        }
        case 33:
            playSystemSoundEffect(readSaturnS8(r14++));
            break;
        case 34:
            playPCM(currentResTask, readSaturnS8(r14++));
            break;
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
        case 39: // multi-choice
        {
            if (r13_pThis->m170_multiChoiceTask)
            {
                if (r13_pThis->m170_multiChoiceTask->m0_Status != 4)
                {
                    return r14 - 1;
                }
                
                s8 r4 = readSaturnS8(r14++);
                if (r4 > 0)
                {
                    r14 = getAlignOn4(r14);
                    r14 += r4 * 4;
                }

                r13_pThis->mF0 = 0;
                return r14;
            }

            if (r13_pThis->m16C_displayStringTask)
            {
                assert(0);
            }

            sSaturnPtr var0 = r14 - 1;
            s8 r7 = readSaturnS8(r14++);
            if (r7)
            {
                startDialogTask(currentResTask, &r13_pThis->m170_multiChoiceTask, &r13_pThis->m118_currentResult, r7, getAlignOn4(r14));
                return var0;
            }
            break;
        }
        case 41: // get inventory count
            {
                r14 = getAlignOn2(r14);
                eItems itemIndex = (eItems)readSaturnS16(r14);
                r14 += 2;
                r13_pThis->m118_currentResult = mainGameState.getItemCount(itemIndex);
            }
            break;
        case 43: // add to inventory
            {
                s8 count = readSaturnS8(r14);
                r14++;
                r14 = getAlignOn2(r14);
                eItems itemIndex = (eItems)readSaturnS16(r14);
                r14 += 2;
                mainGameState.addItemCount(itemIndex, count);
            }
            break;
        case 46: // cutscene control
        {
            sSaturnPtr startOfOpcode = r14 - 1;
            r14 = getAlignOn2(r14);
            s16 frame = readSaturnS16(r14);
            r14 += 2;
            if (r13_pThis->mF0 == 0)
            {
                if (e006Task0)
                {
                    s32 currentFrame = getCutsceneFrameIndex(e006Task0->m0);
                    if (currentFrame <= frame)
                        return startOfOpcode;
                }
            }
            else
            {
                if ((r13_pThis->mF4 == 0) && (r13_pThis->mF8 != 0))
                {
                    r13_pThis->m100 = 0x3C;
                    r13_pThis->mF8 = 0;
                    return r14;
                }
            }
            break;
        }
        case 48:
            if (r13_pThis->m16C_receivedItemTask)
            {
                if (r13_pThis->m16C_receivedItemTask->m0 != 4)
                {
                    return r14 - 1;
                }

                r13_pThis->mF0 = 0;
                r14 = getAlignOn2(r14);
                r14 += 6;
                return r14;
            }
            else
            {
                sSaturnPtr r11 = r14 - 1;
                r14 = getAlignOn2(r14);
                s16 r6 = readSaturnS16(r14);
                r14 += 2;
                eItems itemIndex = (eItems)readSaturnS16(r14);
                r14 += 2;
                s16 itemCount = readSaturnS16(r14);
                r14 += 2;
                createReceiveItemTask(currentResTask, &r13_pThis->m16C_receivedItemTask, r6, itemIndex, itemCount);
                return r11;
            }
            break;
        default:
            assert(0);
        }
    } while (1);
}

void scriptUpdateRunScript()
{
    s32 r8_numBackgroundScripts = npcData0.m0_numBackgroundScripts;

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
        if (r8_numBackgroundScripts)
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
                    return;
                }

                r9++;
            } while (--r8_numBackgroundScripts);

            npcData0.m0_numBackgroundScripts = 0;
        }
    }
}

void sScriptTask::Init(sScriptTask* pThis)
{
    gCollisionRegistry.m0 = 0;
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
        position.zeroize();
    }
    else
    {
        position = (*npcData0.m6C_LCSTargets)[index];
    }

    return position;
}

void sScriptTask::Update(sScriptTask* pThis)
{
    if (pThis->m18_LCSFocusLineScale)
    {
        pThis->m18_LCSFocusLineScale--;
    }

    if (!(npcData0.mFC & 0x10))
    {
        pThis->m8_currentLCSType = 0;
        pThis->m4 = sSaturnPtr::getNull();
    }
    else if(!(npcData0.mFC & 1))
    {
        if (pThis->m8_currentLCSType == 2)
        {
            if (pThis->mC->m38_pOwner->getTask()->isFinished())
            {
                pThis->m8_currentLCSType = 0;
            }
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
            r13 = (*npcData0.m64_scriptList)[pThis->mC_AsIndex];
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
                playSystemSoundEffect(0x24);
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6)
            {
                // Activate LCS
                addBackgroundScript(r13, 3, r11, nullptr);
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
        if (pThis->mC->m2C_collisionSetupIndex == 3)
        {
            adjustMatrixTranslation(pThis->mC->m14_halfAABB[1] / 2);
        }
        break;
    default:
        assert(0);
        break;
    }

    if ((pCurrentMatrix->m[2][3] >= graphicEngineStatus.m405C.m14_farClipDistance) || (pCurrentMatrix->m[2][3] < graphicEngineStatus.m405C.m10_nearClipDistance))
    {
        pThis->m8_currentLCSType = 0;
    }
    else
    {
        sVec2_S16 LCSScreenCoordinates;
        LCSScreenCoordinates[0] = setDividend(graphicEngineStatus.m405C.m18_widthScale, pCurrentMatrix->m[0][3], pCurrentMatrix->m[2][3]);
        LCSScreenCoordinates[1] = setDividend(graphicEngineStatus.m405C.m1C_heightScale, pCurrentMatrix->m[1][3], pCurrentMatrix->m[2][3]);

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
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = 0x1005; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x400 | r4; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = color; // CMDCOLR
    vdp1WriteEA.mC_CMDXA = position[0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -position[1]; // CMDYA
    vdp1WriteEA.m10_CMDXB = position[2]; // CMDXC
    vdp1WriteEA.m12_CMDYB = -position[3]; // CMDYX
    vdp1WriteEA.m14_CMDXC = position[4];
    vdp1WriteEA.m16_CMDYC = -position[5];
    vdp1WriteEA.m18_CMDXD = position[6];
    vdp1WriteEA.m1A_CMDYD = -position[7];

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
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
