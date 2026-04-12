#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "3dEngine.h"
#include "3dModels.h"

// Sky entity (size 0x120, with arg)
struct sA5SkyEntity : public s_workAreaTemplateWithArg<sA5SkyEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sFieldModelRenderContext m8_modelCtx;
    sVec3_FP m3C_renderPosition;
    s_3dModel m48_model1;
    s_3dModel m98_model2;
    sVec3_FP mE8_position;
    s32 mF4;
    sVec3_FP mF8_rotation;
    s32 m104;
    s32 m108;
    s32 m10C;
    s32 m110;
    s32 m114;
    u8 m118_pad[4];
    u8 m11C_flag;
    // Saturn size 0x120
};

// 0605ABD4
static void a5SkyEntity_Init(sA5SkyEntity* p, sSaturnPtr arg)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    s_fileBundle* pBundle = p->m0_memoryArea.m0_mainMemoryBundle;

    // Copy position and rotation from Saturn arg
    p->mE8_position = readSaturnVec3(arg);
    p->mF8_rotation.m0_X = fixedPoint(readSaturnS32(arg + 12));
    p->mF8_rotation.m4_Y = fixedPoint(readSaturnS32(arg + 16));
    p->mF8_rotation.m8_Z = fixedPoint(readSaturnS32(arg + 20));

    p->mF4 = 0;
    p->m104 = 0;
    p->m108 = 0;
    p->m10C = 0;
    p->m110 = 0;
    p->m114 = 0;
    p->m11C_flag = 0;

    // Init primary 3D model at model offset 0x1C
    sStaticPoseData* pPose1 = pBundle->getStaticPose(0xB54, pBundle->getModelHierarchy(0x1C)->countNumberOfBones());
    init3DModelRawData(p, &p->m48_model1, 0, pBundle, 0x1C, nullptr, pPose1, nullptr, nullptr);
    stepAnimation(&p->m48_model1);

    if ((mainGameState.bitField[0x96] & 0x20) == 0)
    {
        // Init secondary 3D model at model offset 0x14
        sStaticPoseData* pPose2 = pBundle->getStaticPose(0xB4C, pBundle->getModelHierarchy(0x14)->countNumberOfBones());
        init3DModelRawData(p, &p->m98_model2, 0, pBundle, 0x14, nullptr, pPose2, nullptr, nullptr);
        stepAnimation(&p->m98_model2);
    }
    else
    {
        Unimplemented(); // FUN_FLD_A5__0605aef4 alternate setup + change update method
    }

    // Render position = world position + Y offset
    p->m3C_renderPosition.m0_X = p->mE8_position.m0_X;
    p->m3C_renderPosition.m4_Y = fixedPoint(p->mE8_position.m4_Y.asS32() + 0x21000);
    p->m3C_renderPosition.m8_Z = p->mE8_position.m8_Z;

    initFieldModelRenderContext(&p->m8_modelCtx, p, nullptr,
        &p->m3C_renderPosition, nullptr, 2, 0, -1, 0, 0);
}

// 0605b32c — sky entity rotation animation state machine. Manages a
// random acceleration/deceleration cycle for the floating sky object's
// Y rotation. Three states: 0 = accelerate from stop, 1 = direction
// switch, 2 = cruise/decelerate.
static void a5SkyEntity_animateRotation(sA5SkyEntity* p)
{
    u32 rng = randomNumber() & 0xFFF;

    s32 state = p->m110;
    if (state == 0)
    {
        // State 0: accelerating from stop
        p->m114++;
        if (p->m114 == 1)
        {
            p->m108 = 0x5B05B0;
            p->m10C = 0;
        }
        else if (p->m114 > 5)
        {
            p->m108 = 0x38E38E;
            p->m114 = 0;
            p->m110 = (rng < 0xB33) ? 2 : 1;
        }
    }
    else if (state == 1)
    {
        // State 1: direction switch
        p->m10C = (s32)0xFFFEDCBB; // -0x12345
        if (p->m108 < 0x2468B)
        {
            p->m10C = 0x12345;
            p->m110 = 2;
        }
        if (rng < 0x7A)
        {
            p->m110 = 0;
        }
        else if (rng < 0x199)
        {
            p->m110 = 2;
        }
    }
    else if (state == 2)
    {
        // State 2: cruise/decelerate
        if (p->m10C >= 1)
        {
            p->m10C -= 0x369D0;
            if (p->m10C < 0)
            {
                p->m108 += p->m10C;
                p->m10C = 0;
            }
        }
        else
        {
            p->m10C = 0;
        }

        p->m114++;
        if (p->m114 < 300)
        {
            if (p->m114 > 0x1D)
            {
                if (p->m108 < 0x2468B)
                {
                    if (rng < 0x732)
                    {
                        p->m110 = 0;
                    }
                }
                else
                {
                    if (rng < 0x7A)
                    {
                        p->m110 = 0;
                    }
                    else if (rng < 0x4CC)
                    {
                        p->m110 = 1;
                    }
                }
            }
        }
        else
        {
            if (p->m108 < 0x1C71C8)
            {
                p->m110 = 0;
            }
            else
            {
                p->m110 = 1;
            }
        }

        if (p->m108 < 0x2468B && rng < 0x333)
        {
            p->m110 = 0;
        }

        if (p->m110 != 2)
        {
            p->m114 = 0;
        }
    }

    p->m108 += p->m10C;
    p->m104 += p->m108;
}

// 0605ADB6
static void a5SkyEntity_Update(sA5SkyEntity* p)
{
    if ((mainGameState.bitField[0x96] & 0x40) == 0)
    {
        p->m8_modelCtx.m18_visibilityFlags |= 1;
    }
    else
    {
        a5SkyEntity_animateRotation(p);
        p->m8_modelCtx.m18_visibilityFlags = 0;
    }

    updateFieldModelRenderContext(&p->m8_modelCtx);

    s32 culled = checkPositionVisibilityAgainstFarPlane(&p->mE8_position);
    p->m11C_flag = (culled == 0) ? 1 : 0;

    pushCurrentMatrix();
    translateCurrentMatrix(&p->mE8_position);

    sVec3_FP rotation;
    rotation.m0_X = p->mF8_rotation.m0_X;
    rotation.m4_Y = fixedPoint(p->mF8_rotation.m4_Y.m_value + p->m104);
    rotation.m8_Z = p->mF8_rotation.m8_Z;
    rotateCurrentMatrixZYX(&rotation);

    callGridCellDraw_normalSub2(p->m0_memoryArea.m0_mainMemoryBundle, 0x568);
    popMatrix();
}

// 0605AD24
static void a5SkyEntity_Draw(sA5SkyEntity* p)
{
    if (p->m11C_flag == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(&p->mE8_position);
    rotateCurrentMatrixZYX(&p->mF8_rotation);
    p->m48_model1.m18_drawFunction(&p->m48_model1);
    adjustMatrixTranslation(fixedPoint(p->mF4));
    rotateCurrentMatrixShiftedY(fixedPoint(p->m104));
    p->m98_model2.m18_drawFunction(&p->m98_model2);
    popMatrix();
}

// 0605abc4
void createA5_envObjects_sub2_skyEntity(p_workArea parent, sSaturnPtr arg)
{
    static sA5SkyEntity::TypedTaskDefinition td = { &a5SkyEntity_Init, &a5SkyEntity_Update, &a5SkyEntity_Draw, nullptr };
    createSubTaskWithArg<sA5SkyEntity>(parent, arg, &td);
}
