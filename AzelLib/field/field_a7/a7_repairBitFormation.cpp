#include "PDS.h"
#include "a7_repairBit.h"
#include "o_fld_a7.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldDragonInput.h"
#include "field/fieldScriptWaitTask.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"


// Forward declare RepairBit child struct for position access
struct sRepairBit : public s_workAreaTemplateWithArg<sRepairBit, void*>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_targetPos;
    sVec3_FP m14_currentPos;
    // ... (remaining fields defined in a7_repairBitChild.cpp)
};

extern s32 playBattleSoundEffect(s32 effectIndex);
void skyTransporter_noop(s32* pScratch);

static inline s32 a7_performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

extern void dispatchTutorialMultiChoiceSub2();

// 060732fc, 06073266 — clearDragonScriptFlag, dragonTransitionToNormal

// 06059b8c
void repairBitFormation_Init(sRepairBitFormation* pThis, sSaturnPtr arg)
{
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    s16 entryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;
    if ((pSub->m370_fieldDebuggerWho & 2) != 0
        && pSub->m37C_debugMenuStatus1[1] == 0
        && pSub->m369 == 0
        && entryPoint != (s16)0xffff)
    {
        if (entryPoint != 0)
        {
            mainGameState.bitField[0x74] |= 0x20;
        }
        mainGameState.setPackedBits(0x599, 5, (u32)entryPoint);
    }

    playPCM((p_workArea)pThis, 0x66);

    pThis->m0_arg = arg;
    pThis->m4_count = (s32)mainGameState.readPackedBits(0x599, 5);
    getFieldSpecificData_A7()->m27C_pad[0] = (u8)pThis->m4_count;

    for (s32 i = 0; i < 5; i++)
    {
        pThis->m24_alive[i] = 0;
    }

    for (s32 i = 0; i < 5 && pThis->m4_count < 0xF; i++)
    {
        sSaturnPtr entry = arg + (u32)pThis->m4_count * 0x1C;
        p_workArea pChild = repairBit_spawn((p_workArea)pThis, entry, &pThis->m24_alive[i]);
        pThis->m10_children[i] = pChild;
        if (pChild == nullptr)
        {
            pThis->m24_alive[i] = 0;
        }
        else
        {
            pThis->m24_alive[i] = 1;
            pThis->m4_count++;
        }
    }
    pThis->m8_iter = 0;
}

// 06059cd4
void repairBitFormation_Update(sRepairBitFormation* pThis)
{
    s32 aliveCount = 0;
    s32 iter = pThis->m8_iter;
    s32 scratch[3] = { 0, 0, 0 }; // mirrors uStack_24/uStack_20/uStack_1c

    for (s32 i = 0; i < 5; i++)
    {
        if (pThis->m24_alive[i] == 0)
        {
            if (pThis->m4_count < 0xF)
            {
                sSaturnPtr entry = pThis->m0_arg + (u32)pThis->m4_count * 0x1C;
                p_workArea pChild = repairBit_spawn((p_workArea)pThis, entry, &pThis->m24_alive[i]);
                pThis->m10_children[i] = pChild;
                if (pChild != nullptr)
                {
                    pThis->m24_alive[i] = 1;
                    pThis->m4_count++;
                }
                startFieldScript(0x10, 0x5d8);
                dragonTransitionToNormal();
            }
        }
        else
        {
            aliveCount++;
            if (iter == 0)
            {
                // Saturn copies child[i]->m14..m1C (current X/Y/Z) into the
                // scratch buffer.
                sRepairBit* pChild = (sRepairBit*)pThis->m10_children[i];
                scratch[0] = pChild->m14_currentPos.m0_X.m_value;
                scratch[1] = pChild->m14_currentPos.m4_Y.m_value;
                scratch[2] = pChild->m14_currentPos.m8_Z.m_value;
            }
            iter--;
        }
    }

    if (aliveCount == 0)
    {
        playBattleSoundEffect(0x66);
        mainGameState.bitField[0x97] |= 0x80;
        pThis->getTask()->markFinished();
    }
    else
    {
        if (pThis->m8_iter < aliveCount)
        {
            skyTransporter_noop(scratch);
        }
        u32 next = (u32)(pThis->mC_phaseCounter + 1) & 3;
        pThis->mC_phaseCounter = (s32)next;
        if (next == 0)
        {
            pThis->m8_iter = (s32)performModulo(aliveCount, pThis->m8_iter + 1);
        }
    }

    // Field-debugger debug overlay status line.
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    if ((pSub->m370_fieldDebuggerWho & 2) != 0
        && pSub->m37C_debugMenuStatus1[1] == 0
        && pSub->m369 == 0)
    {
        vdp2PrintStatus.m10_palette = 0x8000;
        vdp2DebugPrintSetPosition(1, 0x17);
        u32 packed = mainGameState.readPackedBits(0x599, 5);
        vdp2PrintfSmallFont("%02d %02d %01d ", pThis->m4_count, packed);
    }
}
