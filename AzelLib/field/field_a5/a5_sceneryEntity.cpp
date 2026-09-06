#include "PDS.h"
#include "o_fld_a5.h"
#include "kernel/fileBundle.h"
#include "field/fieldModelRender.h"
#include "field/fieldCutsceneTask.h"
#include "field/exitField.h"
#include "3dEngine.h"
#include "field/fieldDebrisScatter.h"

// 3D scenery entity (Saturn size 0x58). A static positioned/rotated
// model with a render-context callback that triggers a visual effect
// when the dragon enters the grid cell.
struct sA5SceneryEntity : public s_workAreaTemplate<sA5SceneryEntity>
{
    s_memoryAreaOutput m0_memoryArea;          // 0x00
    sVec3_FP           m8_position;            // 0x08
    sVec3_FP           m14_rotation;           // 0x14
    s32                m20_triggerState;        // 0x20 — 0 = idle, 1 = triggered by callback
    sFieldModelRenderContext m24_modelCtx;      // 0x24
    // Saturn size 0x58
};

// Lookup tables at FLD_A5::0609900c and 06099010
static const s16 kParticleFileBundleOffsets[] = { 0x0020, 0x0024 };
static const s16 kParticleConfigOffsets[] = { (s16)0x09A4, (s16)0x09A8 };

// 06058330
static void a5SceneryEntity_setupParticlesSub(sA5SceneryEntity* pThis, sSaturnPtr pConfig)
{
    s32 index = readSaturnS32(pConfig);
    s16 bundleOffset = kParticleFileBundleOffsets[index];
    s16 configOffset = kParticleConfigOffsets[index];

    sDebrisScatterParams config = {};
    initDebrisScatterConfig(&config, bundleOffset, configOffset);

    // Read max scalar from file bundle tree at the given offset
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootAddr = READ_BE_U32(pRaw + bundleOffset);
    config.m8_spread = MTH_Mul(readMaxScalarFromBundleTree(pRaw, treeRootAddr), 0x20000);

    config.m10_pPosition = (sVec3_FP*)getSaturnPtr(readSaturnEA(pConfig + 4));
    config.m14_pRotation = (sVec3_FP*)getSaturnPtr(readSaturnEA(pConfig + 16));
    config.m38 = 0xa000;
    config.m_pBundle = pBundle;

    createDebrisScatterTask(pThis, &config, false);
}

// 060583a8
static void a5SceneryEntity_setupParticles(sA5SceneryEntity* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 7);
    a5SceneryEntity_setupParticlesSub(pThis, gFLD_A5->getSaturnPtr(0x06098fd4));
    a5SceneryEntity_setupParticlesSub(pThis, gFLD_A5->getSaturnPtr(0x06098ff0));
}

// 0605ec0a — render context callback. Sets visibility and triggers
// the effect on the next update frame.
static void a5SceneryEntity_contextCallback(sA5SceneryEntity* pThis)
{
    pThis->m24_modelCtx.m18_visibilityFlags |= 1;
    pThis->m20_triggerState = 1;
}

// 0605ebd0 — update: runs the model render context, and when the
// callback has set m20 to 1, dispatches the trigger effect.
static void a5SceneryEntity_Update(sA5SceneryEntity* pThis)
{
    updateFieldModelRenderContext(&pThis->m24_modelCtx);
    if (pThis->m20_triggerState == 1)
    {
        startExitCutsceneCached(gFLD_A5->getSaturnPtr(0x0609c604), 0x11, 0, 9, -0x8000);
        pThis->m20_triggerState = 0;
    }
}

// 0605ec60 — create the scenery entity for subfield 9.
p_workArea createA5_sceneryEntity(p_workArea parent)
{
    sA5SceneryEntity* pThis = createSubTaskFromFunction<sA5SceneryEntity>(parent, &a5SceneryEntity_Update);
    if (pThis == nullptr)
        return nullptr;

    getMemoryArea(&pThis->m0_memoryArea, 7);

    pThis->m8_position.m0_X = fixedPoint(0x480000);
    pThis->m8_position.m4_Y = fixedPoint(0x5000);
    pThis->m8_position.m8_Z = fixedPoint(-0xA00000);

    // Saturn reads rotation from VDP2 rotation coefficient RAM:
    // X = DAT_20218e80 >> 4, Y = 0, Z = DAT_20219e80 >> 4
    Unimplemented();
    pThis->m14_rotation.m0_X = 0;
    pThis->m14_rotation.m4_Y = 0;
    pThis->m14_rotation.m8_Z = 0;

    pThis->m20_triggerState = 0;

    initFieldModelRenderContext(
        &pThis->m24_modelCtx,
        pThis,
        (void*)&a5SceneryEntity_contextCallback,
        &pThis->m8_position,
        &pThis->m14_rotation,
        3, 0, -1, 0, 0);

    a5SceneryEntity_setupParticles(pThis);

    return (p_workArea)pThis;
}
