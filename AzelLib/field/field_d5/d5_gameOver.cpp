#include "PDS.h"
#include "d5_gameOver.h"
#include "d5_particles.h"
#include "d5_nameEntry.h"
#include "field.h"
#include "field/field_a3/o_fld_a3_1.h"
#include "audio/systemSounds.h"
#include "kernel/fade.h"

extern bool hasSaveGame();

// Game over continue/end menu palette indices
static const u32 gameOverPaletteIndices[] = { 0x0C, 0x0E };

struct s_gameOverSecondaryTask : public s_workAreaTemplate<s_gameOverSecondaryTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(s_gameOverSecondaryTask* pThis);
    static void Draw(s_gameOverSecondaryTask* pThis);
    static void Update_Confirm(s_gameOverSecondaryTask* pThis);

    s_nameEntryParticleTask* m0_particleTask; // 0x00
    u8 m4_state;       // 0x04
    u8 m5_selection;   // 0x05: 0=continue, 1=end
    u8 m6_hasSaveGame; // 0x06
    u8 m7_result;      // 0x07
    // size 0x08
};

// 0605453C
void s_gameOverSecondaryTask::Init(s_gameOverSecondaryTask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();

    // Pause dragon task
    pFieldTask->m8_pSubFieldData->m338_pDragonTask->getTask()->markPaused();

    // Pause LCS task
    pFieldTask->m8_pSubFieldData->m340_pLCS->getTask()->markPaused();

    // Clear update functions
    pFieldTask->m8_pSubFieldData->pUpdateFunction2 = nullptr;
    pFieldTask->m8_pSubFieldData->m374_pUpdateFunction1 = nullptr;

    pThis->m0_particleTask = createParticleSystemTask(pThis);

    if (hasSaveGame() == 0)
    {
        pThis->m5_selection = 1; // default to "End" if no save
    }
    else
    {
        pThis->m6_hasSaveGame = 1;
    }
}

// 060546CA
void s_gameOverSecondaryTask::Draw(s_gameOverSecondaryTask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    s_dragonTaskWorkArea* pDragonTask = pFieldTask->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->mF8_Flags |= 0x20000;

    switch (pThis->m4_state)
    {
    case 0:
        if (g_fadeControls.m0_fade0.m20_stopped != 0)
        {
            pThis->m4_state++;
        }
        break;
    case 1:
    {
        u16 buttons = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown;
        if (buttons & 8) // Start
        {
            playSystemSoundEffect(0);
            pThis->m_UpdateMethod = &s_gameOverSecondaryTask::Update_Confirm;
            pThis->m4_state = 0;
            pThis->m7_result = 2;
        }
        else if (buttons & 0x20) // Down
        {
            if (pThis->m5_selection == 0)
            {
                playSystemSoundEffect(2);
            }
            pThis->m5_selection = 1;
        }
        else if (buttons & 0x10) // Up
        {
            if (pThis->m5_selection != 0)
            {
                if (pThis->m6_hasSaveGame == 0)
                {
                    playSystemSoundEffect(5);
                }
                else
                {
                    playSystemSoundEffect(2);
                    pThis->m5_selection = 0;
                }
            }
        }

        // Render menu text
        vdp2PrintStatus.m10_palette = (u16)((gameOverPaletteIndices[pThis->m5_selection] & 0xF) << 12);
        vdp2DebugPrintSetPosition(0x12, 0xB);
        drawLineLargeFont("CONTINUE");

        vdp2PrintStatus.m10_palette = (u16)((gameOverPaletteIndices[pThis->m5_selection ^ 1] & 0xF) << 12);
        vdp2DebugPrintSetPosition(0x12, 0xE);
        drawLineLargeFont("  END   ");
        break;
    }
    default:
        break;
    }
}

// 0605459c
void s_gameOverSecondaryTask::Update_Confirm(s_gameOverSecondaryTask* pThis)
{
    switch (pThis->m4_state)
    {
    case 0:
        if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        if (pThis->m5_selection == 0)
        {
            // Continue: fade out particle, white fade
            pThis->m0_particleTask->m28_state = 2;
            {
                u32 color = convertColorToU32ForFade(*(const sVec3_FP*)&g_fadeControls);
                fadePalette(&g_fadeControls.m0_fade0, color, 0xFFFF, 0x78);
            }
            pThis->m4_state = 2;
            vdp2DebugPrintSetPosition(0x12, 0xE);
            vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
            vdp2Controls.m_isDirty = 1;
        }
        else
        {
            // End: fade out particle, black fade
            pThis->m0_particleTask->m28_state = 3;
            {
                u32 color = convertColorToU32ForFade(*(const sVec3_FP*)&g_fadeControls);
                fadePalette(&g_fadeControls.m0_fade0, color, 0x8000, 0x78);
            }
            pThis->m4_state = 1;
            vdp2DebugPrintSetPosition(0x12, 0xB);
        }
        clearVdp2TextLargeFont();
        break;
    case 1:
        // End path: wait for fade, then exit to title
        if (g_fadeControls.m0_fade0.m20_stopped != 0)
        {
            fieldA3_1_checkExitsTaskUpdate2Sub1(0x74);
        }
        break;
    case 2:
        // Continue path: wait for fade, then create fade-out task
        if (g_fadeControls.m0_fade0.m20_stopped != 0)
        {
            createNameEntryFadeOutTask(pThis, 0x4A);
            pThis->getTask()->markFinished();
        }
        break;
    default:
        break;
    }
}

// 060547ce
void createGameOverSecondaryTask(p_workArea parent)
{
    createSubTask<s_gameOverSecondaryTask>(parent);
}
