#include "PDS.h"
#include "a7_dialogChoiceTask.h"
#include "o_fld_a7.h"
#include "menu_dragon.h"
#include "audio/systemSounds.h"

// 0x10-byte sibling task for FLD_A7 dialog-for-choice interactions.
// Task definition at FLD_A7::06086570 = {init=NULL, update=0605e97c,
// draw=0605e9d6, delete=0605ea46}.
struct sA7DialogChoiceTask : public s_workAreaTemplate<sA7DialogChoiceTask>
{
    p_workArea m0_parent;             // 0x00 — paused parent workArea
    u32        m4_dialogScriptEA;     // 0x04 — Saturn EA: current script cursor
    u32        m8_drawnNameEA;        // 0x08 — Saturn EA of last parsed name string
    u8         mC_drawSubState;       // 0x0C — draw-side state (0 = parse, 1 = draw)
    u8         mD_state;              // 0x0D — input/wait countdown
    u8         mE_pad[2];              // 0x0E..0x0F
    // size 0x10
};

// 0605eab0 — pull the next opcode from the choice script and act on it.
//   0x01      → terminate (mD_state = 0, returns 0)
//   0x15 ptr  → 4-byte aligned name pointer (mD_state = 0, returns name EA)
//   0x18      → pause for 10 frames, switch to dialog HUD slot 3 (returns 0)
//   default   → mD_state = 0, returns 0
static u32 a7DialogChoiceScriptStep_0605eab0(sA7DialogChoiceTask* pThis)
{
    sSaturnPtr cursor = gFLD_A7->getSaturnPtr(pThis->m4_dialogScriptEA);
    s8 opcode = (s8)readSaturnU8(cursor);
    sSaturnPtr next = cursor + 1;
    u32 result = 0;

    if (opcode != 0x01)
    {
        if (opcode == 0x15)
        {
            // The 4-byte payload sits at the next 4-byte aligned offset
            // *after* (cursor + 4). Mirrors `(uint)(p+4) & ~3` from Saturn.
            sSaturnPtr aligned = sSaturnPtr{ (s32)((cursor.m_offset + 4) & ~3u), cursor.m_file };
            result = readSaturnU32(aligned);
            next = aligned + 4;
            pThis->mD_state = 0;
            goto done;
        }
        if (opcode == 0x18)
        {
            pThis->mD_state = 10;
            graphicEngineStatus.m40AC.m0_menuId = 3;
            goto done;
        }
    }
    pThis->mD_state = 0;

done:
    pThis->m4_dialogScriptEA = next.m_offset;
    return result;
}

// 0605eb2e
static void a7DialogChoiceSetupTextArea_0605eb2e()
{
    setupVDP2StringRendering(6, 5, 0x20, 0xE);
    clearVdp2TextArea();
}

// 0605eaf6 — render the parsed name into the dialog text area.
static bool a7DialogChoiceRenderName_0605eaf6(sA7DialogChoiceTask* pThis)
{
    vdp2StringContext.m1C = vdp2StringContext.m0;
    vdp2StringContext.m0 = 0;
    a7DialogChoiceSetupTextArea_0605eb2e();
    bool hasName = pThis->m8_drawnNameEA != 0;
    if (hasName)
    {
        std::string name = readSaturnString(gFLD_A7->getSaturnPtr(pThis->m8_drawnNameEA));
        drawObjectName(name.c_str());
    }
    vdp2StringContext.m0 = vdp2StringContext.m1C;
    return hasName;
}

// 0605e97c
static void a7DialogChoice_Update_0605e97c(sA7DialogChoiceTask* pThis)
{
    if (pThis->mD_state == 0)
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) != 0)
        {
            pThis->m8_drawnNameEA = a7DialogChoiceScriptStep_0605eab0(pThis);
            if (pThis->mD_state == 0)
            {
                pThis->mC_drawSubState = 1;
                playSystemSoundEffect(6);
                return;
            }
            pThis->mC_drawSubState = 2;
        }
    }
    else
    {
        pThis->mD_state--;
        // Reload the script pointer from the per-archetype dialog table.
        sSaturnPtr tableEntry = gFLD_A7->getSaturnPtr(0x060861b0)
                                + (gDragonState->m1C_dragonArchetype + 0x26) * 4;
        pThis->m4_dialogScriptEA = readSaturnU32(tableEntry);
        pThis->m8_drawnNameEA = a7DialogChoiceScriptStep_0605eab0(pThis);
        pThis->mC_drawSubState = 1;
    }
}

// 0605e9d6
static void a7DialogChoice_Draw_0605e9d6(sA7DialogChoiceTask* pThis)
{
    u32 sub = pThis->mC_drawSubState;
    if (sub == 0)
    {
        pThis->m8_drawnNameEA = a7DialogChoiceScriptStep_0605eab0(pThis);
        pThis->mC_drawSubState++;
    }
    else if (sub != 1)
    {
        return;
    }

    drawBlueBox(3, 4, 0x28, 0x10, 0x1000);
    bool drewName = a7DialogChoiceRenderName_0605eaf6(pThis);
    if (!drewName)
    {
        if (pThis->mD_state == 0)
        {
            pThis->m0_parent->getTask()->clearPaused();
            pThis->getTask()->markFinished();
        }
    }
    else
    {
        pThis->mC_drawSubState++;
    }
}

// 0605ea46
static void a7DialogChoice_Delete_0605ea46(sA7DialogChoiceTask* pThis)
{
    clearBlueBox(3, 4, 0x28, 0x10);
}

// 0605ea52
void a7DialogChoice_spawn_0605ea52(p_workArea parent, s32 choiceIdx)
{
    static const sA7DialogChoiceTask::TypedTaskDefinition definition = {
        nullptr,
        &a7DialogChoice_Update_0605e97c,
        &a7DialogChoice_Draw_0605e9d6,
        &a7DialogChoice_Delete_0605ea46,
    };

    sA7DialogChoiceTask* pTask = createSiblingTask<sA7DialogChoiceTask>(parent, &definition);
    if (pTask == nullptr)
    {
        return;
    }

    // Resolve dialog script pointer from the dispatch table at 060861b0,
    // entry [choiceIdx + 0x21]. The table is a sequence of big-endian u32
    // script addresses.
    sSaturnPtr entryPtr = gFLD_A7->getSaturnPtr(0x060861b0) + ((u32)choiceIdx + 0x21) * 4;
    pTask->m4_dialogScriptEA = readSaturnU32(entryPtr);
    pTask->m0_parent = parent;
    pTask->mD_state = 0;
    // Saturn createSiblingTask zeroed the heap; mirror that for mC/m8.
    pTask->mC_drawSubState = 0;
    pTask->m8_drawnNameEA = 0;

    parent->getTask()->markPaused();
}
