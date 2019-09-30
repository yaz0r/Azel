#include "PDS.h"
#include "BTL_A3.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleMainTask.h"
#include "town/town.h"
#include "kernel/vdp1Allocator.h"
#include "battle/battleEngine.h"

BTL_A3_data* g_BTL_A3 = nullptr;

void battle_A3_initMusic(p_workArea pThis)
{
    if ((getBattleManager()->m6_subBattleId == 8) || (getBattleManager()->m6_subBattleId == 9))
    {
        playMusic(6, 0);
        getBattleManager()->m10_battleOverlay->m3 = 1;
    }
    else
    {
        playMusic(4, 0);
        getBattleManager()->m10_battleOverlay->m3 = 1;
    }
    playPCM(pThis, 100);
}

static const char* assetList[] = {
    "BATTLE.MCB",
    "BATTLE.CGB",
    "TUP.MCB",
    "TUP.CGB",
    "ENCAM.BDB",
    (char*)-1,
    (char*)-1,
    "A3CMN.CGB",
    "KEIKOKU.MCB",
    "KEIKOKU.CGB",
    "HEBIMUSI.MCB",
    "HEBIMUSI.CGB",
    "KURAGE.MCB",
    "KURAGE.CGB",
    "BENITATE.MCB",
    "BENITATE.CGB",
    "HEBISU.MCB",
    "HEBISU.CGB",
    "FLD_A3.MCB",
    "FLD_A3.CGB",
    "BTFALLA3.MCB",
    "BTFALLA3.CGB",
    nullptr,
    nullptr,
};

npcFileDeleter* initMemoryForBattleSub0(p_workArea pThis, s32 fileIndex)
{
    s_fileEntry& fileEntry = dramAllocatorEnd[fileIndex];
    fileEntry.m8_refcount++;
    if (fileEntry.m8_refcount != 1)
    {
        return fileEntry.mC_buffer;
    }

    fileEntry.mC_buffer = loadNPCFile2(pThis, fileEntry.mFileName, fileEntry.m4_fileSize, fileIndex);
}

void initMemoryForBattle(p_workArea pThis, const char** assetList)
{
    if (getBattleManager()->mC == 0)
    {
        resetTempAllocators();
        initDramAllocator(getBattleManager()->m10_battleOverlay, townBuffer, sizeof(townBuffer), assetList);
        if (getBattleManager()->mD == 0)
        {
            loadDragon(getBattleManager());
            mainGameState.gameStats.m2_rider1 = 1;
            mainGameState.gameStats.m3_rider2 = 2;
            loadCurrentRider(getBattleManager());
            loadCurrentRider2(getBattleManager());
            getBattleManager()->mD = 1;
            updateDragonIfCursorChanged(0);
        }
    }
    initDramAllocator(getBattleManager()->m10_battleOverlay, townBuffer, sizeof(townBuffer), assetList);
    initVdp1Ram(getBattleManager()->m10_battleOverlay, 0x25C18800, 0x63800);
    allocateNPC(pThis, 0);
    allocateNPC(pThis, 2);
    initMemoryForBattleSub0(pThis, 4);
}

void battle_A3_func0(p_workArea pThis)
{
    initMemoryForBattle(pThis, assetList);
    allocateNPC(pThis, 6);

    sSaturnPtr BTL_A3_Data = g_BTL_A3->getSaturnPtr(0x60a5688);

    createBattleEngineTask(pThis, BTL_A3_Data);
}

p_workArea overlayStart_BTL_A3(p_workArea parent)
{
    if (g_BTL_A3 == NULL)
    {
        FILE* fHandle = fopen("BTL_A3.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        g_BTL_A3 = new BTL_A3_data();
        g_BTL_A3->m_name = "BTL_A3.PRG";
        g_BTL_A3->m_data = fileData;
        g_BTL_A3->m_dataSize = fileSize;
        g_BTL_A3->m_base = 0x6054000;
        g_BTL_A3->init();
    }

    return createBattleMainTask(parent, &battle_A3_initMusic, battle_A3_func0);
}
