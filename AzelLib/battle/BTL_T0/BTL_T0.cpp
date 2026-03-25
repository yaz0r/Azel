#include "PDS.h"
#include "BTL_T0.h"
#include "battle/battleMainTask.h"
#include "battle/battleOverlay.h"
#include "battle/battleManager.h"
#include "battle/battleEngine.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "battle/battleGrid.h"
#include "kernel/grid.h"
#include "field/field_a3/o_fld_a3.h"

static p_workArea Create_BTL_T0_env(p_workArea parent);
static p_workArea Create_BTL_T0_Formation(p_workArea parent, u32 existingFormation);
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "kernel/graphicalObject.h"

struct BTL_T0_data : public battleOverlay
{
    BTL_T0_data() : battleOverlay("BTL_T0.PRG")
    {
        m_grid = readGrid(getSaturnPtr(0x060aa548), 2, 2);
    }

    const sGrid* m_grid;

    sSaturnPtr getEncounterDataTable() override
    {
        return getSaturnPtr(0x060ace6c);
    }

    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) override
    {
        switch (Func.m_offset)
        {
        case 0x060585f0:
            Create_BTL_T0_env(pParent);
            break;
        default:
            Unimplemented();
            break;
        }
    }

    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1) override
    {
        switch (Func.m_offset)
        {
        case 0x060540fc:
            Create_BTL_T0_Formation(pParent, arg0);
            break;
        case 0x060549b4:
            // Secondary formation/entity — stub for now
            Unimplemented();
            break;
        default:
            Unimplemented();
            break;
        }
    }

    p_workArea invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent) override
    {
        Unimplemented();
        return nullptr;
    }
};

static BTL_T0_data* g_BTL_T0 = nullptr;

static const char* BTL_T0_fileList[] = {
    "BATTLE.MCB", "BATTLE.CGB",        // 0, 1
    "TUP.MCB", "TUP.CGB",              // 2, 3
    "ENCAM.BDB",                         // 4
    (const char*)-1,                     // 5
    "A3CMN.MCB", "A3CMN.CGB",           // 6, 7
    "HEBI_TUR.MCB", "HEBI_TUR.CGB",    // 8, 9
    "BENITATE.MCB", "BENITATE.CGB",     // 10, 11
    "FLD_A3.MCB", "FLD_A3.CGB",         // 12, 13
    (const char*)-1,                     // 14
    "TUTORIAL.CGB",                      // 15
    "KARAA72.MCB", "KARAA72.CGB",       // 16, 17
    "KEIKOKU.MCB", "KEIKOKU.CGB",       // 18, 19
    "STK01.MCB", "STK01.CGB",           // 20, 21
    nullptr
};

// 0605400a
static void BTL_T0_initMusic(p_workArea pThis)
{
    s16 subBattleId = gBattleManager->m6_subBattleId;

    s8 soundBank;
    switch (subBattleId)
    {
    case 0:
    case 1:
        soundBank = 4;
        break;
    case 2:
        soundBank = 5;
        break;
    case 3:
        soundBank = 8;
        break;
    default:
        soundBank = 0x21;
        break;
    }

    loadSoundBanks(soundBank, 0);
    gBattleManager->m10_battleOverlay->m3 = 1;
    playPCM(pThis, 100);
}

// 0605409c
static void BTL_T0_initBattle(p_workArea pThis)
{
    initMemoryForBattle(pThis, BTL_T0_fileList);
    allocateNPC(pThis, 6);
    allocateNPC(pThis, 0xE);
    createBattleEngineTask(pThis, g_BTL_T0->getSaturnPtr(0x060a989c));
    // TODO: tutorial controller task (FUN_BTL_T0__0605ac56)
    Unimplemented();
}

// BTL_T0 formation task
struct sBTL_T0_Formation : public s_workAreaTemplateWithArgWithCopy<sBTL_T0_Formation, sSaturnPtr>
{
    static void Init(sBTL_T0_Formation* pThis, sSaturnPtr arg)
    {
        Unimplemented();
    }
    static void Update(sBTL_T0_Formation* pThis)
    {
        Unimplemented();
    }
    static void Delete(sBTL_T0_Formation* pThis)
    {
        Unimplemented();
    }

    u8 m_pad[0x4C]; // size 0x4C
};

// 0607bafa
static p_workArea Create_BTL_T0_FormationSub(p_workArea parent, sSaturnPtr formationData)
{
    static const sBTL_T0_Formation::TypedTaskDefinition definition = {
        &sBTL_T0_Formation::Init,
        &sBTL_T0_Formation::Update,
        nullptr,
        &sBTL_T0_Formation::Delete,
    };
    return createSubTaskWithArg<sBTL_T0_Formation, sSaturnPtr>(parent, formationData, &definition);
}

// 060540fc
static p_workArea Create_BTL_T0_Formation(p_workArea parent, u32 existingFormation)
{
    // On Saturn, existingFormation is a pointer — if non-null, return it as-is
    // In C++, the value comes from the battle engine's formation slot
    if (existingFormation != 0)
    {
        return reinterpret_cast<p_workArea>(static_cast<uintptr_t>(existingFormation));
    }
    return Create_BTL_T0_FormationSub(parent, g_BTL_T0->getSaturnPtr(0x060a76f4));
}

// 06058580
static void BTL_T0_env_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = -0x80000;

    // 06057f34 — BTL_T0 VDP2 init
    gBattleManager->m10_battleOverlay->m1C_envTask = (p_workArea)pThis;
    reinitVdp2();
    initNBG1Layer();
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa76c), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa56c), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa96c), vdp2Palette, 0x200, 0);

    static const sLayerConfig rgb0Setup[] = {
        m2_CHCN, 1, m5_CHSZ, 1, m6_PNB, 1, m7_CNSM, 0,
        m27_RPMD, 2, m11_SCN, 8, m34_W0E, 1, m37_W0A, 1,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = { m31_RxKTE, 1, m13, 1, m0_END };
    setupRotationParams(rotParamsSetup);
    static const sLayerConfig rotParams2Setup[] = { m0_END };
    setupRotationParams2(rotParams2Setup);

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
    regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_T0->getSaturnPtr(0x060aabac));
    setupRotationMapPlanes(1, g_BTL_T0->getSaturnPtr(0x060aab6c));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));

    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    *(u16*)getVdp2Vram(0x2A400) = 0x0700;
    *(u16*)getVdp2Vram(0x2A600) = 0x38E5;
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    regs->mF0_PRISA = 0x405;
    regs->mF2_PRISB = 0x507;
    regs->mF4_PRISC = 0x505;
    regs->mF6_PRISD = 0x505;
    regs->mF8_PRINA = 0x600;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x10000;

    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x400;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    // TODO: applyLayerDisplayConfig, color offset setup from Ghidra (0x060aac80)
    // TODO: FUN_BTL_T0__0607ea38 (additional VDP2 setup)

    // Grid setup
    npcFileDeleter* envFile = (npcFileDeleter*)allocateNPC(pThis, 0x12);
    pThis->m58 = envFile;
    initGridForBattle(envFile, g_BTL_T0->m_grid, 2, 2, 0x400000);

    // 060544a2 — water/overlay effect sub-task
    Unimplemented(); // TODO: create water effect sub-task on envFile

    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;
}

// 060585f0
static p_workArea Create_BTL_T0_env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_T0_env_Init,
        &BTL_A3_Env_Update,
        &BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}

// 06054000
p_workArea overlayStart_BTL_T0(p_workArea parent)
{
    if (g_BTL_T0 == nullptr)
    {
        g_BTL_T0 = new BTL_T0_data();
    }

    gCurrentBattleOverlay = g_BTL_T0;
    return createBattleMainTask(parent, &BTL_T0_initMusic, BTL_T0_initBattle);
}
