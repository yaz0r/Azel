#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "3dEngine.h"

// Sandfall entity (Saturn size 0x70)
struct sA5SandfallEntity : public s_workAreaTemplate<sA5SandfallEntity>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    sFieldModelRenderContext m14_modelCtx;
    sVec3_FP m48_renderPosition;
    s32 m54;
    s32 m58;
    s32 m5C_animState;
    s32 m60;
    s32 m64;
    s32 m68;
    u8 m6C_flag0;
    u8 m6D_flag1;
    u8 m6E_state;
    // Saturn size 0x70
};

// Sandfall subtask (size 0xC)
struct sA5SandfallSubtask : public s_workAreaTemplate<sA5SandfallSubtask>
{
    s32 m0;
    s32 m4;
    sVec3_FP* m8_pPosition;
    // size 0xC
};

// 0605A1D8
static void a5SandfallSubtask_Update(sA5SandfallSubtask* p)
{
    if ((mainGameState.bitField[0x96] & 0x10) == 0)
    {
        getFieldTaskPtr();
        // a5IsNearPosition @ FLD_A5::060790e4 — transform world pos to view and
        // test against screen bounds. Inlined to match the field_a7 pattern.
        sVec3_FP viewPos;
        transformAndAddVecByCurrentMatrix(p->m8_pPosition, &viewPos);
        if (isPointOnScreen(&viewPos, graphicEngineStatus.m405C.m14_farClipDistance))
        {
            mainGameState.bitField[0x96] |= 0x10;
        }
    }
    else
    {
        s_fieldTaskWorkArea* pField = getFieldTaskPtr();
        playPCM(pField->m8_pSubFieldData, 100);
        if (p != nullptr)
        {
            p->getTask()->markFinished();
        }
    }
}

// Animation frame count table (indexed by m60, 7 entries)
static const s32 a5Sandfall_animFrameCounts[] = { 2, 2, 2, 3, 2, 2, 3 }; // 06099fb8

// LOD0 main waterfall model offsets (indexed by m54, 26 entries)
static const s32 a5Sandfall_mainModelOffsets[] = { // 06099e88
    0x934, 0x938, 0x93C, 0x940, 0x944, 0x948, 0x94C, 0x950,
    0x954, 0x958, 0x95C, 0x960, 0x964, 0x968, 0x96C, 0x970,
    0x974, 0x978, 0x97C, 0x980, 0x984, 0x988, 0x98C, 0x990,
    0x994, 0x998
};

// LOD0 secondary model offsets (indexed by m54, 26 entries)
static const s32 a5Sandfall_secondaryModelOffsets[] = { // 06099ef0
    0x9A8, 0x9AC, 0x9B0, 0x9B4, 0x9B8, 0x9BC, 0x9C0, 0x9C4,
    0x9C8, 0x9CC, 0x9D0, 0x9D4, 0x9D8, 0x9DC, 0x9E0, 0x9E4,
    0x9E8, 0x9EC, 0x9F0, 0x9F4, 0x9F8, 0x9FC, 0xA00, 0xA04,
    0xA08, 0xA0C
};

// LOD0 instanced draw model offsets (indexed by m64, 12 entries)
static const s32 a5Sandfall_instanceOffsets_lod0[] = { // 06099f58
    0x1064, 0x1068, 0x106C, 0x1070, 0x1074, 0x1078,
    0x107C, 0x1080, 0x1084, 0x1088, 0x108C, 0x1090
};

// LOD1 instanced draw model offsets (indexed by m64, 12 entries)
static const s32 a5Sandfall_instanceOffsets_lod1[] = { // 06099f88
    0x1094, 0x1098, 0x109C, 0x10A0, 0x10A4, 0x10A8,
    0x10AC, 0x10B0, 0x10B4, 0x10B8, 0x10BC, 0x10C0
};

// 0605a390
static void a5SandfallEntity_checkCameraBounds(sA5SandfallEntity* p)
{
    bool outOfBounds = false;
    if (cameraProperties2.m0_position.m0_X < fixedPoint(0x33E000))
    {
        outOfBounds = true;
    }
    else if (cameraProperties2.m0_position.m0_X > fixedPoint(0x672000))
    {
        outOfBounds = true;
    }
    if (cameraProperties2.m0_position.m8_Z < fixedPoint(-0x76C000))
    {
        outOfBounds = true;
    }
    else if (cameraProperties2.m0_position.m8_Z > fixedPoint(-0x4B0000))
    {
        outOfBounds = true;
    }

    if (outOfBounds)
    {
        if (p->m6C_flag0 == 0)
        {
            setFieldExitConfig_A5(0, nullptr);
            p->m6C_flag0 = 1;
        }
    }
    else if (p->m6C_flag0 == 1)
    {
        setFieldExitConfig_A5(2, nullptr);
        p->m6C_flag0 = 0;
    }
}

// 0605A2A8
static void a5SandfallEntity_Init(sA5SandfallEntity* p)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    p->m8_position.m0_X = fixedPoint(0x500000);
    p->m8_position.m4_Y = 0;
    p->m8_position.m8_Z = fixedPoint(-0x600000);
    p->m6C_flag0 = 1;
    p->m54 = 0;
    p->m58 = 0;
    p->m5C_animState = 2;
    p->m60 = 0;
    p->m64 = 0;
    p->m68 = 0;

    sA5SandfallSubtask* pSub = createSubTaskFromFunction<sA5SandfallSubtask>((p_workArea)p, &a5SandfallSubtask_Update);
    if (pSub)
        pSub->m8_pPosition = &p->m8_position;

    if ((mainGameState.bitField[0x95] & 2) == 0)
    {
        p->m6D_flag1 = 1;
    }

    p->m48_renderPosition = p->m8_position;
    initFieldModelRenderContext(&p->m14_modelCtx, p, nullptr,
        &p->m48_renderPosition, nullptr, 0, 0, -1, 0xFF, 0);
    p->m14_modelCtx.m18_visibilityFlags |= 1;

    if ((mainGameState.bitField[0x96] & 0x40) == 0)
    {
        p->m14_modelCtx.m18_visibilityFlags = 0;
        p->m6E_state = 0;
    }
    else if ((mainGameState.bitField[0x96] & 0x20) == 0)
    {
        p->m6E_state = 1;
    }
    else
    {
        mainGameState.bitField[0x95] |= 2;
    }
}

// 0605A476
static void a5SandfallEntity_Update(sA5SandfallEntity* p)
{
    if ((mainGameState.bitField[0x95] & 2) == 0)
    {
        p->m6D_flag1 = 1;
        p->m58++;
        if (p->m58 >= p->m5C_animState)
        {
            p->m58 = 0;
            p->m54++;
            if (p->m54 > 0x19)
            {
                p->m54 = 0;
                p->m60++;
                if (p->m60 > 6)
                {
                    p->m60 = 0;
                }
                p->m5C_animState = a5Sandfall_animFrameCounts[p->m60];
            }
        }
    }
    else if (p->m6D_flag1 != 0)
    {
        playSystemSoundEffect(0x70);
        p->m6D_flag1 = 0;
    }

    if ((mainGameState.bitField[0x70] & 0x20) == 0)
    {
        p->m14_modelCtx.m18_visibilityFlags |= 2;
    }
    else
    {
        if ((mainGameState.bitField[0x70] & 0x10) == 0)
        {
            p->m14_modelCtx.m18_visibilityFlags = 0;
            goto done;
        }
        p->m14_modelCtx.m18_visibilityFlags |= 1;
    }
done:
    updateFieldModelRenderContext(&p->m14_modelCtx);
    a5SandfallEntity_checkCameraBounds(p);

    p->m68++;
    if (p->m68 > 1)
    {
        p->m68 = 0;
        p->m64++;
        if (p->m64 > 0xB)
        {
            p->m64 = 0;
        }
    }
}

// 0605A52E
static void a5SandfallEntity_Draw(sA5SandfallEntity* p)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&p->m8_position);

    fixedPoint depth = pCurrentMatrix->m[2][3];
    s32 lod = gridCellDraw_GetDepthRange(depth);
    s_fileBundle* pBundle = p->m0_memoryArea.m0_mainMemoryBundle;

    if ((mainGameState.bitField[0x95] & 2) == 0)
    {
        // Not stopped — animated waterfall
        if (lod == 0)
        {
            addObjectToDrawList(pBundle->get3DModel(a5Sandfall_mainModelOffsets[p->m54]));
            if (p->m6E_state == 0)
            {
                addObjectToDrawList(pBundle->get3DModel(a5Sandfall_secondaryModelOffsets[p->m54]));
            }
            else
            {
                drawCurrentObjectInstanced(
                    pBundle->get3DModel(a5Sandfall_secondaryModelOffsets[p->m54]),
                    pBundle->get3DModel(a5Sandfall_instanceOffsets_lod0[p->m64]));
            }
        }
        else if (lod == 1)
        {
            addObjectToDrawList(pBundle->get3DModel(0x9A0));
            if (p->m6E_state == 0)
            {
                addObjectToDrawList(pBundle->get3DModel(0xA14));
            }
            else
            {
                drawCurrentObjectInstanced(
                    pBundle->get3DModel(0xA14),
                    pBundle->get3DModel(a5Sandfall_instanceOffsets_lod1[p->m64]));
            }
        }
    }
    else
    {
        // Stopped — static waterfall
        if (lod == 0)
        {
            addObjectToDrawList(pBundle->get3DModel(0x99C));
        }
        else if (lod == 1)
        {
            addObjectToDrawList(pBundle->get3DModel(0x9A4));
        }

        if ((s32)depth < 0x12C001)
        {
            addObjectToDrawList(pBundle->get3DModel(0xA10));
        }
        else if ((s32)depth < 0x200000)
        {
            addObjectToDrawList(pBundle->get3DModel(0xA18));
        }
    }

    popMatrix();
}

// 0605a68a
void createA5_envObjects_sub2_sandfall(p_workArea parent)
{
    static sA5SandfallEntity::TypedTaskDefinition td = { &a5SandfallEntity_Init, &a5SandfallEntity_Update, &a5SandfallEntity_Draw, nullptr };
    createSubTask<sA5SandfallEntity>(parent, &td);
}
