#include "PDS.h"
#include "o_fld_a5.h"
#include "kernel/fileBundle.h"
#include "field/fieldModelRender.h"
#include "field/fieldCutsceneTask.h"
#include "field/exitField.h"
#include "3dEngine.h"

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

// Config struct passed to a5_spawnParticleEmitter (06085e46).
// Built on the stack by a5DecorEmitter_loadUnlockEffectConfig + setupParticlesSub.
struct sParticleEmitterConfig
{
    s32 m0;            // 0x00 — particle count/type (0x6e)
    s32 m4;            // 0x04 — spread parameter (0x4ccc)
    s32 m8_scalar;     // 0x08 — max tree scalar * 0x20000
    s32 mC;            // 0x0C — range/limit (0x1fffff)
    u32 m10_positionEA; // 0x10 — Saturn address of position data
    u32 m14_rotationEA; // 0x14 — Saturn address of rotation data
    s32 m18;           // 0x18
    s32 m1C;           // 0x1C
    s32 m20;           // 0x20
    s32 m24;           // 0x24
    s32 m28;           // 0x28
    s32 m2C;           // 0x2C
    s32 m30;           // 0x30
    u16 m34_bundleOffset; // 0x34
    u16 m36_configOffset; // 0x36
    u32 m38_scale;     // 0x38 — scale (0xa000)
    s32 m3C;           // 0x3C
    s32 m40;           // 0x40
    u16 m44;           // 0x44 — (0xffff)
    u16 m46;           // 0x46
    // size 0x48
};

// 06085e46 — particle emitter spawner (complex system, many sub-dependencies)
static void a5_spawnParticleEmitter(sA5SceneryEntity* /*pEntity*/, sParticleEmitterConfig* /*pConfig*/)
{
    Unimplemented();
}

// Lookup tables at FLD_A5::0609900c and 06099010
static const s16 kParticleFileBundleOffsets[] = { 0x0020, 0x0024 };
static const s16 kParticleConfigOffsets[] = { (s16)0x09A4, (s16)0x09A8 };

// 060583d4 — initialize the emitter config struct
static void a5DecorEmitter_loadUnlockEffectConfig(sParticleEmitterConfig* pConfig, u16 bundleOffset, u16 configOffset)
{
    memset(pConfig, 0, sizeof(*pConfig));
    pConfig->m0 = 0x6e;
    pConfig->m4 = 0x4ccc;
    pConfig->mC = 0x1fffff;
    pConfig->m34_bundleOffset = bundleOffset;
    pConfig->m36_configOffset = configOffset;
    pConfig->m44 = 0xffff;
}

// 06058420 — recursively find max scalar in a file bundle tree node.
// Each node in the raw bundle data has 3 BE u32s: {valueOffset, leftOffset, rightOffset}.
// valueOffset points to a BE s32 value. Returns the max value across the tree.
static s32 a5DecorEmitter_readUnlockEffectScalar(u8* pRawBundle, u32 nodeOffset)
{
    s32 result;
    u32 valueOffset = READ_BE_U32(pRawBundle + nodeOffset);
    if (valueOffset == 0)
    {
        result = 0;
    }
    else
    {
        result = READ_BE_S32(pRawBundle + valueOffset);
    }

    u32 leftOffset = READ_BE_U32(pRawBundle + nodeOffset + 4);
    if (leftOffset != 0)
    {
        s32 leftVal = a5DecorEmitter_readUnlockEffectScalar(pRawBundle, leftOffset);
        if (result < leftVal)
            result = leftVal;
    }

    u32 rightOffset = READ_BE_U32(pRawBundle + nodeOffset + 8);
    if (rightOffset != 0)
    {
        s32 rightVal = a5DecorEmitter_readUnlockEffectScalar(pRawBundle, rightOffset);
        if (result < rightVal)
            result = rightVal;
    }

    return result;
}

// 06058330
static void a5SceneryEntity_setupParticlesSub(sA5SceneryEntity* pThis, sSaturnPtr pConfig)
{
    s32 index = readSaturnS32(pConfig);
    s16 bundleOffset = kParticleFileBundleOffsets[index];
    s16 configOffset = kParticleConfigOffsets[index];

    sParticleEmitterConfig config = {};
    a5DecorEmitter_loadUnlockEffectConfig(&config, bundleOffset, configOffset);

    // Read max scalar from file bundle tree at the given offset
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootAddr = READ_BE_U32(pRaw + bundleOffset);
    config.m8_scalar = MTH_Mul(a5DecorEmitter_readUnlockEffectScalar(pRaw, treeRootAddr), 0x20000).m_value;

    config.m10_positionEA = readSaturnU32(pConfig + 4);
    config.m14_rotationEA = readSaturnU32(pConfig + 16);
    config.m38_scale = 0xa000;

    a5_spawnParticleEmitter(pThis, &config);
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
