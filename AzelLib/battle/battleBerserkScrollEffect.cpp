#include "PDS.h"
#include "battleBerserkScrollEffect.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "BTL_A3/BTL_A3_map6.h"

// Forward declarations
static void findAndLoadTownFile(const char* filename, u8* destination, u16 vdp1TextureLocation);
bool isSoundFileLoaded(sSaturnPtr pConfig);

struct sBattleBerserkScrollTask : public s_workAreaTemplate<sBattleBerserkScrollTask>
{
    s8 m0_state;            // 0x00
    s8 m1_fadeFrames;       // 0x01
    s8 m2_totalFrames;      // 0x02
    s8 m3_paletteIndex;     // 0x03
    s8 m4_numCycles;        // 0x04
    s8 m5_palettesPerCycle; // 0x05
    s8 m6_scrollId;         // 0x06
    s8 m7_pad;              // 0x07
    s32 m8_pad;             // 0x08
    s32 mC_scrollX;         // 0x0C
    s32 m10_scrollY;        // 0x10
    s32 m14_scaleX;         // 0x14
    s32 m18_scaleY;         // 0x18
    s32 m1C_currentScale;   // 0x1C
    s32 m20_scaleIncrement; // 0x20
    s_VDP2Regs m24_savedVdp2Regs; // 0x24
    // size 0x144
};

// Config table entry for scroll effects (Saturn data at 060b15d0)
struct sBerserkScrollConfig
{
    sSaturnPtr m0_scbFile;      // Saturn ptr to SCB filename
    sSaturnPtr m4_pnbFile;      // Saturn ptr to PNB filename
    s8 m8_numCycles;            // byte at offset 0x08
    sSaturnPtr mC_paletteData;  // Saturn ptr to palette data array
    s8 m10_palettesPerCycle;    // byte at offset 0x10
    s32 m14_initialScale;       // initial scale (fixedPoint)
    s32 m18_targetScale;        // target scale (fixedPoint)
    s8 m1C_fadeFrames2;         // byte at offset 0x1C
    s8 m1D_fadeFrames;          // byte at offset 0x1D
    s8 m1E_texSizeMode;         // 0=0x200000, 1=0x400000
};

static sBerserkScrollConfig readScrollConfig(sSaturnPtr tableBase, int index)
{
    sSaturnPtr entry = tableBase + index * 0x20;
    sBerserkScrollConfig cfg;
    cfg.m0_scbFile = readSaturnEA(entry);
    cfg.m4_pnbFile = readSaturnEA(entry + 4);
    cfg.m8_numCycles = (s8)readSaturnU8(entry + 8);
    cfg.mC_paletteData = readSaturnEA(entry + 0xC);
    cfg.m10_palettesPerCycle = (s8)readSaturnU8(entry + 0x10);
    cfg.m14_initialScale = readSaturnS32(entry + 0x14);
    cfg.m18_targetScale = readSaturnS32(entry + 0x18);
    cfg.m1C_fadeFrames2 = (s8)readSaturnU8(entry + 0x1C);
    cfg.m1D_fadeFrames = (s8)readSaturnU8(entry + 0x1D);
    cfg.m1E_texSizeMode = (s8)readSaturnU8(entry + 0x1E);
    return cfg;
}

// BTL_A3::060a28b8
static void battleBerserkScrollEffect_update(sBattleBerserkScrollTask* pThis)
{
    s_BTL_A3_Env* envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
    s8 envScrollCount = envTask->m5C;

    sBerserkScrollConfig cfg = {};
    sSaturnMemoryFile* overlayFile = gBattleManager->m10_battleOverlay->m4_battleEngine->m3A8_overlayBattledata.m_file;
    if (envScrollCount > 0)
    {
        pThis->m6_scrollId = envScrollCount - 1;
        cfg = readScrollConfig(overlayFile->getSaturnPtr(0x060b15d0), pThis->m6_scrollId);
    }

    switch ((u8)pThis->m0_state)
    {
    case 0:
    {
        if (envScrollCount < 1)
            return;

        // Backup VDP2 registers
        memcpy_dma((void*)vdp2Controls.m4_pendingVdp2Regs, &pThis->m24_savedVdp2Regs, sizeof(s_VDP2Regs));
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
        vdp2Controls.m_isDirty = 1;

        // Setup NBG0 (data from 060b16f0)
        {
            static const sLayerConfig nbg0Setup[] = {
                {m1_TPEN, 0}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1},
                {m7_CNSM, 0}, {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1},
                {m10_SPN, 3}, {(eVdp2LayerConfig)22, 0}, {(eVdp2LayerConfig)21, 0},
                {(eVdp2LayerConfig)20, 0},
                {m0_END},
            };
            setupNBG0(nbg0Setup);
        }

        pThis->m3_paletteIndex = 0;
        pThis->m1_fadeFrames = cfg.m1D_fadeFrames;

        // Create palette fade sub-task
        Unimplemented(); // FUN_BTL_A3__0607a434 — palette fade subtask

        // Load scroll tile data
        {
            std::string scbName = readSaturnString(cfg.m0_scbFile);
            findAndLoadTownFile(scbName.c_str(), getVdp2Vram(0x1B000), 0);
        }
        {
            std::string pnbName = readSaturnString(cfg.m4_pnbFile);
            findAndLoadTownFile(pnbName.c_str(), getVdp2Vram(0x1F000), 0);
        }

        initLayerMap(0, 0x1F000, 0x1F000, 0x1F000, 0x1F000);

        // Set VDP2 back color
        *(u16*)getVdp2Vram(0x2A600) = 0x8000;

        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;
        vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF) | 0x100;
        vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x407;
        vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
        vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x605;
        vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
        vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
        vdp2Controls.m_isDirty = 1;

        pThis->mC_scrollX = 0;
        pThis->m10_scrollY = 0;
        pThis->m14_scaleX = 0;
        pThis->m18_scaleY = 0;

        pThis->m1C_currentScale = cfg.m14_initialScale;
        pThis->m14_scaleX = cfg.m14_initialScale;
        pThis->m18_scaleY = cfg.m14_initialScale;

        pThis->m5_palettesPerCycle = cfg.m10_palettesPerCycle;
        pThis->m4_numCycles = cfg.m8_numCycles;
        pThis->m2_totalFrames = pThis->m5_palettesPerCycle * pThis->m4_numCycles;
        pThis->m20_scaleIncrement = (s32)FP_Div(cfg.m18_targetScale - cfg.m14_initialScale, fixedPoint((s32)pThis->m2_totalFrames << 16));

        pThis->m0_state++;
        break;
    }
    case 1:
    {
        bool scbLoaded = isSoundFileLoaded(cfg.m0_scbFile);
        if (!scbLoaded)
            return;
        bool pnbLoaded = isSoundFileLoaded(cfg.m4_pnbFile);
        if (!pnbLoaded)
            return;
        pThis->m0_state++;
        break;
    }
    case 2:
        pThis->m0_state++;
        break;
    case 3:
    {
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
        vdp2Controls.m_isDirty = 1;
        pThis->m1_fadeFrames--;
        if (pThis->m1_fadeFrames >= 0)
            return;
        pThis->m0_state++;
        break;
    }
    case 4:
    {
        pThis->m1C_currentScale += pThis->m20_scaleIncrement;
        if (pThis->m1C_currentScale > 0x10000)
        {
            pThis->m1C_currentScale = 0x10000;
        }
        else if (pThis->m1C_currentScale < 0x4000)
        {
            pThis->m1C_currentScale = 0x4000;
        }

        pThis->m3_paletteIndex++;
        if (pThis->m3_paletteIndex < pThis->m5_palettesPerCycle)
        {
            // DMA copy next palette
            sSaturnPtr paletteEntry = cfg.mC_paletteData + pThis->m3_paletteIndex * 4;
            sSaturnPtr paletteAddr = readSaturnEA(paletteEntry);
            asyncDmaCopy(paletteAddr, getVdp2Cram(0x600), 0x20, 0);
        }
        else
        {
            pThis->m4_numCycles--;
            if (pThis->m4_numCycles < 1)
            {
                // Last cycle done — set final palette and start fade out
                pThis->m3_paletteIndex = pThis->m5_palettesPerCycle - 1;
                pThis->m1_fadeFrames = cfg.m1D_fadeFrames;
                Unimplemented(); // FUN_BTL_A3__0607a434 — final palette fade
                pThis->m0_state++;
                return;
            }
            pThis->m3_paletteIndex = 0;
            sSaturnPtr paletteEntry = cfg.mC_paletteData + pThis->m3_paletteIndex * 4;
            sSaturnPtr paletteAddr = readSaturnEA(paletteEntry);
            asyncDmaCopy(paletteAddr, getVdp2Cram(0x600), 0x20, 0);
        }
        break;
    }
    case 5:
    {
        pThis->m1_fadeFrames--;
        if (pThis->m1_fadeFrames >= 0)
            return;

        // Restore VDP2 registers
        memcpy_dma(&pThis->m24_savedVdp2Regs, (void*)vdp2Controls.m4_pendingVdp2Regs, sizeof(s_VDP2Regs));
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~1;
        vdp2Controls.m_isDirty = 1;

        *(u16*)getVdp2Vram(0x2A600) = 0xCDAB;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

        pThis->m0_state++;
        break;
    }
    case 6:
        pThis->m0_state++;
        break;
    case 7:
    {
        // Clear env task flag and restore BGON
        envTask->m70_flags &= ~1;
        vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
        vdp2Controls.m_isDirty = 1;
        envTask->m5C = 0;
        pThis->m0_state = 0;
        break;
    }
    default:
        break;
    }
}

// BTL_A3::060a2c72
static void battleBerserkScrollEffect_draw(sBattleBerserkScrollTask* pThis)
{
    s_BTL_A3_Env* envTask = gBattleManager->m10_battleOverlay->m1C_envTask;
    if ((envTask->m70_flags & 1) == 0)
        return;

    // Project the desired camera position to screen space
    sVec3_FP viewPos;
    transformAndAddVecByCurrentMatrix(gBattleManager->m10_battleOverlay->m4_battleEngine->m3D8_pDesiredCameraPosition, &viewPos);

    s16 xProj, yProj;
    getVdp1ProjectionParams(&xProj, &yProj);
    s32 projX = setDividend(xProj, (s32)viewPos.m0_X, (s32)viewPos.m8_Z);
    s32 projY = setDividend(yProj, (s32)viewPos.m4_Y, (s32)viewPos.m8_Z);

    // Copy current scale to both axes
    pThis->m14_scaleX = pThis->m1C_currentScale;
    pThis->m18_scaleY = pThis->m1C_currentScale;

    // Compute inverse scale
    s32 invScaleX = (s32)FP_Div(0x10000, fixedPoint(pThis->m14_scaleX));
    s32 invScaleY = (s32)FP_Div(0x10000, fixedPoint(pThis->m18_scaleY));

    // Determine texture size from config
    sSaturnMemoryFile* overlayFile = gBattleManager->m10_battleOverlay->m4_battleEngine->m3A8_overlayBattledata.m_file;
    sBerserkScrollConfig cfg = readScrollConfig(overlayFile->getSaturnPtr(0x060b15d0), pThis->m6_scrollId);
    s32 texSize;
    if (cfg.m1E_texSizeMode == 0)
        texSize = 0x200000;
    else if (cfg.m1E_texSizeMode == 1)
        texSize = 0x400000;
    else
        texSize = 0x200000;

    // Compute scaled texture offset
    s32 scaledTexX = MTH_Mul(fixedPoint(texSize), fixedPoint(invScaleX));
    s32 scaledTexY = MTH_Mul(fixedPoint(texSize), fixedPoint(invScaleY));

    // Compute scroll position from projected screen coordinates
    s32 tempScrollX = scaledTexX - (projX + 0xB0) * 0x10000;
    s32 tempScrollY = scaledTexY - (0x70 - projY) * 0x10000;

    pThis->mC_scrollX = (s32)MTH_Mul(fixedPoint(tempScrollX), fixedPoint(pThis->m14_scaleX));
    pThis->m10_scrollY = (s32)MTH_Mul(fixedPoint(tempScrollY), fixedPoint(pThis->m18_scaleY));

    // Apply to VDP2 scroll registers
    pauseEngine[4] = 0;
    updateVDP2CoordinatesIncrement(pThis->m14_scaleX, pThis->m18_scaleY);
    setupVDP2CoordinatesIncrement2(pThis->mC_scrollX, pThis->m10_scrollY);
    pauseEngine[4] = 4;
}

static const sBattleBerserkScrollTask::TypedTaskDefinition battleBerserkScrollTaskDefinition = {
    nullptr,
    battleBerserkScrollEffect_update,
    battleBerserkScrollEffect_draw,
    nullptr,
};

// 060a2e78
void createBattleBerserkScrollEffect(p_workArea parent)
{
    createSubTask<sBattleBerserkScrollTask>(parent, &battleBerserkScrollTaskDefinition);
}

// Local copy of findAndLoadTownFile (also in battleEngine.cpp)
static void findAndLoadTownFile(const char* filename, u8* destination, u16 vdp1TextureLocation)
{
    findMandatoryFileOnDisc(filename);
    loadFile(filename, destination, vdp1TextureLocation);
}
