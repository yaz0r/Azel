#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
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

// 060583a8 — particle effect setup helper. Creates two effect descriptors
// from Saturn data tables. Depends on the A5 particle pool system.
static void a5SceneryEntity_setupParticles(sA5SceneryEntity* /*pThis*/)
{
    Unimplemented();
}

// 0607cdcc — effect dispatcher triggered when the dragon enters the
// entity's grid cell. Spawns a visual effect from the entity's data.
static void a5SceneryEntity_triggerEffect(sA5SceneryEntity* /*pThis*/)
{
    Unimplemented();
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
        a5SceneryEntity_triggerEffect(pThis);
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

    // Saturn reads initial rotation from VDP2 rotation coefficient RAM
    // (DAT_20218e80 >> 4, 0, DAT_20219e80 >> 4). These are runtime values
    // from the background rotation system; default to 0 until the VDP2
    // coefficient table is fully mapped.
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
