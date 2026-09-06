#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldModelRender.h"
#include "field/fieldDragonInput.h"
#include "field/fieldVisibilityGrid.h"
#include "field/fieldDebrisScatter.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "3dEngine.h"
#include "3dModels.h"
#include "a5_gridDeferredDraw.h"
#include "field/field_a3/o_fld_a3_1.h"
#include "field/fieldDragonMovement.h"

extern void dispatchTutorialMultiChoiceSub2();

static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

struct sA5WormPassageEntrance : public s_workAreaTemplateWithArg<sA5WormPassageEntrance, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_dataPtr;
    sFieldModelRenderContext mC_modelCtx;
    s_3dModel m40_3dModel;
    sVec3_FP m90_normal;
    s32 m9C_config;
    s32 mA0_state;
    s32 mA4_delay;
    std::array<s16, 2> mA8_modelIdx;
    // Saturn size 0xAC
};

// 06077304
static void a5WormPassageEntrance_dispatchCollision(sA5WormPassageEntrance* pThis, s16 entryKey)
{
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey);
}

// 06056EFC
static void a5WormPassageEntrance_Draw(sA5WormPassageEntrance* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    if (readSaturnS16(data + 0x18) == 0)
        return;

    pushCurrentMatrix();

    sVec3_FP pos = readSaturnVec3(data);
    translateCurrentMatrix(&pos);

    sVec3_FP rot = { fixedPoint(readSaturnS32(data + 0xC)),
                     fixedPoint(readSaturnS32(data + 0x10)),
                     fixedPoint(readSaturnS32(data + 0x14)) };
    rotateCurrentMatrixZYX(&rot);

    // FUN_FLD_A5__06078fa6 is an overlay-local copy of gridCellDraw_GetDepthRange.
    u32 lod = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if (lod < 2)
    {
        // data[0x18] or data[0x1A] depending on LOD — a s16 bundle offset
        // to the sProcessed3dModel that should be drawn at this range.
        s16 modelOffset = readSaturnS16(data + 0x18 + lod * 2);
        sProcessed3dModel* pModel = pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset);
        addObjectToDrawList(pModel);
        a5WormPassageEntrance_dispatchCollision(pThis, readSaturnS16(data + 0x1C));
    }

    popMatrix();
}

// 06056a22
static void a5WormPassageEntrance_runUnlockTransition(sA5WormPassageEntrance* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;

    // Read model/pose offsets from Saturn data at +0x5A and +0x5C
    u16 bundleOffset = readSaturnU16(data + 0x5A);
    u16 poseOffset = readSaturnU16(data + 0x5C);

    sDebrisScatterParams params;
    initDebrisScatterConfig(&params, bundleOffset, poseOffset);

    // Read max scalar from the bundle tree
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootOffset = READ_BE_U32(pRaw + bundleOffset);
    params.m8_spread = fixedPoint(readMaxScalarFromBundleTree(pRaw, treeRootOffset));

    params.m0_gravity = fixedPoint(0x14a);
    params.m4_bounce = fixedPoint((s32)0xFFFFE667);
    params.mC_randomMask = fixedPoint(0x3FFFFF);

    // Position from Saturn data at +0xC (the entity's world position)
    sVec3_FP pos = readSaturnVec3(data + 0xC);
    params.m10_pPosition = &pos;
    params.m14_pRotation = nullptr;

    params.m18_velX = 0;
    params.m1C_velY = 0;
    params.m20_velZ = fixedPoint((s32)0xFFFFF800);

    params.m_pBundle = pBundle;

    createDebrisScatterTask((p_workArea)pThis, &params, false);

    // Play the unlock sound effect from Saturn data at +0x54
    s16 soundId = readSaturnS16(data + 0x54);
    playSystemSoundEffect((s32)soundId);
}

// 06056F6A
static void a5WormPassageEntrance_DrawWithModel(sA5WormPassageEntrance* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;

    if (pThis->mA0_state == 1)
    {
        u32 bitIdx = (u32)readSaturnS16(data + 0x5E);
        u32 adjIdx = bitIdx;
        if ((s32)bitIdx > 999)
        {
            adjIdx = bitIdx - 0x236;
            bitIdx = bitIdx - 0x236;
        }
        if ((mainGameState.bitField[adjIdx >> 3] & bitMasks[bitIdx & 7]) != 0)
        {
            a5WormPassageEntrance_runUnlockTransition(pThis);
            pThis->mA8_modelIdx[0] = readSaturnS16(data + 0x56);
            pThis->mA8_modelIdx[1] = readSaturnS16(data + 0x58);
            pThis->mA0_state++;
        }
    }

    pushCurrentMatrix();
    sVec3_FP pos = readSaturnVec3(data);
    translateCurrentMatrix(&pos);
    sVec3_FP rot = { fixedPoint(readSaturnS32(data + 0xC)),
                     fixedPoint(readSaturnS32(data + 0x10)),
                     fixedPoint(readSaturnS32(data + 0x14)) };
    rotateCurrentMatrixZYX(&rot);

    u32 lod = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if (lod < 2)
    {
        // LOD switch
        s16 modelOffset = pThis->mA8_modelIdx[lod];
        sProcessed3dModel* pModel = pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset);
        addObjectToDrawList(pModel);
        a5WormPassageEntrance_dispatchCollision(pThis, readSaturnS16(data + 0x1C));
    }
    popMatrix();
}

// 06056A9A
static void a5WormPassageEntrance_Init(sA5WormPassageEntrance* pThis, sSaturnPtr arg)
{
    s16 areaIdx = readSaturnS16(arg + 0x1E);
    getMemoryArea(&pThis->m0_memoryArea, areaIdx);
    pThis->m8_dataPtr = arg;
    pThis->m9C_config = readSaturnS16(arg + 0x20);

    // If data has a 3D model index
    s16 modelOffset = readSaturnS16(arg + 0x5A);
    if (modelOffset != 0)
    {
        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnS16(arg + 0x5C), pBundle->getModelHierarchy(modelOffset)->countNumberOfBones());
        init3DModelRawData(pThis, &pThis->m40_3dModel, 0, pBundle, modelOffset, nullptr, pPose, nullptr, nullptr);
        stepAnimation(&pThis->m40_3dModel);
        pThis->m_DrawMethod = &a5WormPassageEntrance_DrawWithModel;

        // Check game state bit from data
        u32 bitIdx = (u32)readSaturnS16(arg + 0x5E);
        u32 adjIdx = (bitIdx < 1000) ? bitIdx : (bitIdx - 0x236);
        if ((mainGameState.bitField[adjIdx >> 3] & bitMasks[bitIdx & 7]) == 0)
        {
            pThis->mA8_modelIdx[0] = readSaturnS16(arg + 0x18);
            pThis->mA8_modelIdx[1] = readSaturnS16(arg + 0x1A);
            pThis->mA0_state = 1;
        }
        else
        {
            pThis->mA8_modelIdx[0] = readSaturnS16(arg + 0x56);
            pThis->mA8_modelIdx[1] = readSaturnS16(arg + 0x58);
            pThis->mA0_state = 2;
        }
    }

    // Set up model render context
    s16 rotFlag = readSaturnS16(arg + 0x26);
    if (rotFlag == 0)
    {
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), nullptr, 3, 0, -1, 0, 0);
    }
    else
    {
        // Compute rotated normal vector
        sMatrix4x3 rotMatrix;
        initMatrixToIdentity(&rotMatrix);
        rotateMatrixShiftedY(fixedPoint(readSaturnS32(arg + 0x10)), &rotMatrix);
        transformVec(*(sVec3_FP*)getSaturnPtr(gFLD_A5->getSaturnPtr(0x06098AE4)), pThis->m90_normal, rotMatrix);
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), &pThis->m90_normal, 3, 0, -1, 0, 0);
    }
}

// 06056C16
static void a5WormPassageEntrance_Update(sA5WormPassageEntrance* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    s32 state = pThis->m9C_config;

    switch (pThis->m9C_config) {
    case -1:
        if (readSaturnS16(data + 0x24)) {
            fieldNoop();
        }
        break;
    case 0:
        {
            // Check activation game state bits
            bool activated = true;
            s16 bit1 = readSaturnS16(data + 0x22);
            if (bit1 != 0)
            {
                u32 adj1 = (bit1 < 1000) ? (u32)bit1 : (u32)(bit1 - 0x236);
                if ((mainGameState.bitField[adj1 >> 3] & bitMasks[bit1 & 7]) == 0)
                    activated = false;
            }
            if (activated)
            {
                s16 bit2 = readSaturnS16(data + 0x5E);
                if (bit2 != 0)
                {
                    u32 adj2 = (bit2 < 1000) ? (u32)bit2 : (u32)(bit2 - 0x236);
                    if ((mainGameState.bitField[adj2 >> 3] & bitMasks[bit2 & 7]) == 0)
                        activated = false;
                }
            }
            if (activated)
                pThis->m9C_config++;
        }

        if (readSaturnS16(data + 0x24)) {
            fieldNoop();
        }
        break;
    case 1:
        updateFieldModelRenderContext(&pThis->mC_modelCtx);
        if (readSaturnS16(data + 0x24)) {
            fieldNoop();
        }
        break;
    case 2: // autopilot in tunnel
        {
            sFieldCameraStatus* pActiveSlot = getFieldCameraStatus();
            pActiveSlot->m0_position.m0_X = fixedPoint(readSaturnS32(data + 0x2C));
            pActiveSlot->m0_position.m4_Y = fixedPoint(readSaturnS32(data + 0x30));
            pActiveSlot->m0_position.m8_Z = fixedPoint(readSaturnS32(data + 0x34));
            setCameraFollowMode_cut(eCameraFollowMode_scriptTarget);

            pThis->mA4_delay = 90;

            s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
            pDragon->mF0 = &DragonUpdateCutscene;
            pDragon->m104_dragonScriptStatus = 0;
            pDragon->mF8_Flags &= ~0x400;
            pDragon->m8_pos.m4_Y = fixedPoint(0x14000);
            pDragon->m20_angle.m0_X = fixedPoint(0);
            pDragon->m20_angle.m4_Y = fixedPoint(readSaturnS32(data + 0x48));
            pDragon->m20_angle.m8_Z = fixedPoint(0);

            s32 dragonSpeed = readSaturnS32(data + 0x50);
            u16 yawIdx = (u16)((u32)pDragon->m20_angle.m4_Y.m_value >> 16) & 0xFFF;
            pDragon->m160_deltaTranslation.m0_X = MTH_Mul(fixedPoint(-dragonSpeed), getSin(yawIdx));
            pDragon->m160_deltaTranslation.m4_Y = fixedPoint(performDivision(
                pThis->mA4_delay, readSaturnS32(data + 0x3C) - pDragon->m8_pos.m4_Y.m_value));
            pDragon->m160_deltaTranslation.m8_Z = MTH_Mul(fixedPoint(-dragonSpeed), getCos(yawIdx));

            pDragon->m8_pos.m0_X = fixedPoint(readSaturnS32(data + 0x38)
                - pDragon->m160_deltaTranslation.m0_X.m_value * (pThis->mA4_delay + 5));
            pDragon->m8_pos.m4_Y = fixedPoint(readSaturnS32(data + 0x3C)
                - pDragon->m160_deltaTranslation.m4_Y.m_value * (pThis->mA4_delay + 5));
            pDragon->m8_pos.m8_Z = fixedPoint(readSaturnS32(data + 0x40)
                - pDragon->m160_deltaTranslation.m8_Z.m_value * (pThis->mA4_delay + 5));

            pThis->m9C_config++;
        }
        break;
    case 3:
        {
            pThis->mA4_delay--;
            if (pThis->mA4_delay > 0)
                break;

            s16 dest = readSaturnS16(data + 0x28);
            if (dest < 0)
            {
                if ((mainGameState.bitField[0xB] & 0x80) == 0)
                {
                    fieldA3_1_checkExitsTaskUpdate2Sub1(0xB);
                }
                else
                {
                    dispatchTutorialMultiChoiceSub2();
                }
            }
            else
            {
                triggerSubfieldChange(dest, -0x8000);
                playBattleSoundEffect(100);
                playBattleSoundEffect(0x65);
            }
            pThis->m9C_config++;
        }
        break;
    case 4: // done
        break;
    }
}

void createA5_wormPassageEntrance(p_workArea parent, sSaturnPtr data)
{
    static sA5WormPassageEntrance::TypedTaskDefinition td = { &a5WormPassageEntrance_Init, &a5WormPassageEntrance_Update, &a5WormPassageEntrance_Draw, nullptr };
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, data, &td);
}

// 06057066
void createA5_wormPassageEntrances_day(p_workArea parent)
{
    static sA5WormPassageEntrance::TypedTaskDefinition td = { &a5WormPassageEntrance_Init, &a5WormPassageEntrance_Update, &a5WormPassageEntrance_Draw, nullptr };
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098AF0), &td);
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098B50), &td);
}

// 0605708e
void createA5_wormPassageEntrances_night(p_workArea parent)
{
    static sA5WormPassageEntrance::TypedTaskDefinition td = { &a5WormPassageEntrance_Init, &a5WormPassageEntrance_Update, &a5WormPassageEntrance_Draw, nullptr };
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098C10), &td);
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098BB0), &td);
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098C70), &td);
}

// 060570c2
void createA5_envObjects_sub4_wormPassageEntrances(p_workArea parent)
{
    static sA5WormPassageEntrance::TypedTaskDefinition td = { &a5WormPassageEntrance_Init, &a5WormPassageEntrance_Update, &a5WormPassageEntrance_Draw, nullptr };
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098CD0), &td);
    createSubTaskWithArg<sA5WormPassageEntrance>(parent, gFLD_A5->getSaturnPtr(0x06098D30), &td);
}
