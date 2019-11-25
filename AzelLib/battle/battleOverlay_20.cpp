#include "PDS.h"
#include "battleOverlay_20.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "kernel/vdp1Allocator.h"
#include "town/town.h" // todo: for npcFileDeleter
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"

void s_battleOverlay_20_update(s_battleOverlay_20* pThis)
{
    pThis->m2C++;
    switch (pThis->m10)
    {
    case 0:
        if (BattleEngineSub0_UpdateSub0())
        {
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 2)
                return;
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 0xE)
                return;
        }
        if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x80000)
        {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }
}

void s_battleOverlay_20_drawSub0(s_battleOverlay_20* pThis)
{
    {
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1001); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, pThis->m14_vdp1Memory + 0x2ebc); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, pThis->m14_vdp1Memory + 0x278); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x108); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m1A_X - 0x82); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x5C - pThis->m1C_Y)); // CMDYA
        setVdp1VramU16(vdp1WriteEA + 0x14, pThis->m1A_X + 0x87); // CMDXC
        setVdp1VramU16(vdp1WriteEA + 0x16, -(-0x73 - pThis->m1C_Y)); // CMDYC

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    sSaturnPtr spriteDataTable = gCurrentBattleOverlay->getSaturnPtr(0x60b1a40);
    sSaturnPtr currentSprite = gCurrentBattleOverlay->getSaturnPtr(0x60b19b8);
    for(int i=0; i<4; i++)
    {
        sSaturnPtr spriteData = readSaturnEA(spriteDataTable + 4 * i);

        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, readSaturnS16(spriteData + 10)); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, readSaturnS16(spriteData + 6) + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, readSaturnS16(spriteData + 2) + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, readSaturnS16(spriteData + 8)); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, readSaturnS16(currentSprite) + pThis->m1A_X - 0xb0); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-pThis->m1C_Y - readSaturnS16(currentSprite + 2)) + 0x70); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;

        currentSprite += 2;
    }

    {
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, 0x2ED0 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, 0x554 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x252e); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m16 - 0x8E); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x33 - pThis->m18)); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    {
        u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0
        setVdp1VramU16(vdp1WriteEA + 0x04, 0x88); // CMDPMOD
        setVdp1VramU16(vdp1WriteEA + 0x06, 0x2ED0 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDCOLR
        setVdp1VramU16(vdp1WriteEA + 0x08, 0x4d4 + dramAllocatorEnd[0].mC_buffer->m4_vd1Allocation->m4_vdp1Memory); // CMDSRCA
        setVdp1VramU16(vdp1WriteEA + 0x0A, 0x553); // CMDSIZE
        setVdp1VramU16(vdp1WriteEA + 0x0C, pThis->m16 - 0xB0); // CMDXA
        setVdp1VramU16(vdp1WriteEA + 0x0E, -(-0x31 - pThis->m18)); // CMDYA

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

}

void s_battleOverlay_20_drawSub1(s_battleOverlay_20* pThis)
{
    FunctionUnimplemented();
}


void s_battleOverlay_20_draw(s_battleOverlay_20* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 8)
    {
        if (BattleEngineSub0_UpdateSub0())
        {
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 2)
                return;
            if (getBattleManager()->m10_battleOverlay->m4_battleEngine->m38C_battleIntroType == 0xE)
                return;
        }

        s_battleOverlay_20_drawSub0(pThis);
        s_battleOverlay_20_drawSub1(pThis);

        if(getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x21] == 0)
        {
            pThis->m0 = 0;
        }
        else
        {
            pThis->m0 = 1;
        }
    }
}

void s_battleOverlay_20_delete(s_battleOverlay_20*)
{
    FunctionUnimplemented();
}

void createBattleHPOrBpDisplayTask(npcFileDeleter* parent, u16*, u16*, u16*, s32)
{
    FunctionUnimplemented();
}

void createPGTask(npcFileDeleter* parent, u16*, s32)
{
    FunctionUnimplemented();
}

void createBattleOverlay_task20(npcFileDeleter* parent)
{
    static const s_battleOverlay_20::TypedTaskDefinition definition = {
        nullptr,
        &s_battleOverlay_20_update,
        &s_battleOverlay_20_draw,
        &s_battleOverlay_20_delete,
    };

    s_battleOverlay_20* pNewTask = createSubTask<s_battleOverlay_20>(parent, &definition);

    getBattleManager()->m10_battleOverlay->m20 = pNewTask;

    pNewTask->m14_vdp1Memory = parent->m4_vd1Allocation->m4_vdp1Memory;
    pNewTask->m16 = 0;
    pNewTask->m18 = 0x47;
    pNewTask->m1A_X = 0;
    pNewTask->m1C_Y = 0x47;
    pNewTask->m1E = 0x31;
    pNewTask->m20 = 0xb3;
    pNewTask->m22 = 0x66;
    pNewTask->m24 = 0xb3;

    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.m10_currentHP, &mainGameState.gameStats.maxHP, &pNewTask->m1E, 1);
    createBattleHPOrBpDisplayTask(parent, &mainGameState.gameStats.currentBP, &mainGameState.gameStats.maxBP, &pNewTask->m22, 0);
    createPGTask(parent, &pNewTask->m16, 0);

    pNewTask->m0 = 0;
    pNewTask->m28 = 0;
    pNewTask->m10 = 0;
}
