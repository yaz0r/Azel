#include "PDS.h"
#include "itemVisualEffect.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleGrid.h"
#include "battleIntro.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"

struct sItemVisualEffectTask : public s_workAreaTemplateWithCopy<sItemVisualEffectTask>
{
    s16 m8_currentFrame;
    s16 mA_totalFrames;
    sVec3_FP* mC_pPosition1;  // camera source position
    sVec3_FP* m10_pPosition2; // camera target position
    u8* m14_cameraAnimData;   // keyframe data for camera offset (if flag 0x10)
    u8* m18_targetAnimData;   // keyframe data for target offset (if flag 0x20)
    u8* m1C_rotationAnimData; // keyframe data for rotation (if flag 0x40)
    u8 m20_pos1Mode;
    u8 m21_pos2Mode;
    u8 m22_dataType;          // 0=s16*0x10, 1=s16*0x100, 2=s32>>4, 3=s32
    // size 0x24
};

// helper: read 3-component keyframe from big-endian animation data
static void readAnimKeyframe(u8* animData, s16 frame, u8 dataType, sVec3_FP& out)
{
    if (!animData) return;
    if (dataType == 0)
    {
        u8* p = animData + frame * 6;
        out.m0_X += (s32)READ_BE_S16(p + 0) << 4;
        out.m4_Y += (s32)READ_BE_S16(p + 2) << 4;
        out.m8_Z += (s32)READ_BE_S16(p + 4) << 4;
    }
    else if (dataType == 1)
    {
        u8* p = animData + frame * 6;
        out.m0_X += (s32)READ_BE_S16(p + 0) << 8;
        out.m4_Y += (s32)READ_BE_S16(p + 2) << 8;
        out.m8_Z += (s32)READ_BE_S16(p + 4) << 8;
    }
    else if (dataType == 2)
    {
        u8* p = animData + frame * 12;
        out.m0_X += READ_BE_S32(p + 0) >> 4;
        out.m4_Y += READ_BE_S32(p + 4) >> 4;
        out.m8_Z += READ_BE_S32(p + 8) >> 4;
    }
    else if (dataType == 3)
    {
        u8* p = animData + frame * 12;
        out.m0_X += READ_BE_S32(p + 0);
        out.m4_Y += READ_BE_S32(p + 4);
        out.m8_Z += READ_BE_S32(p + 8);
    }
}

// BTL_A3::0605f630 — delete handler for item visual effect task
static void sItemVisualEffectTask_Delete(s_workAreaTemplate<s_workArea>*)
{
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;
    battleEngine_resetBattleCameraPreset();
    pGrid->mE4_currentCameraReferenceCenter = pGrid->m134_desiredCameraPosition;
    pGrid->mF0_currentCameraReferenceForward = pGrid->m140_desiredCameraTarget;
    pGrid->m108_deltaCameraPosition = {};
    pGrid->m114_deltaCameraTarget = {};
    gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m2000000 = 1;
    battleGrid_setCameraFov(0x238e38e);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m3D0 = nullptr;
}

// BTL_A3::0605f046
static void sItemVisualEffectTask_Update(sItemVisualEffectTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s_battleGrid* pGrid = gBattleManager->m10_battleOverlay->m8_gridTask;

    // Compute camera position (from pPosition1 + camera anim offset)
    sVec3_FP camPos;
    if ((pThis->m20_pos1Mode & 1) == 0 || (pThis->m20_pos1Mode & 2) == 0)
    {
        camPos = *pThis->mC_pPosition1;
    }
    else
    {
        transformAndAddVec(*pThis->mC_pPosition1, camPos, *(cameraProperties2.m28 + 1));
    }
    u8 keyframeType = pThis->m22_dataType & 3; // Ghidra: *(byte*)(param_1+0x22) & 3
    readAnimKeyframe(pThis->m14_cameraAnimData, pThis->m8_currentFrame, keyframeType, camPos);

    // Compute target position (from pPosition2 + target anim offset)
    sVec3_FP targetPos;
    if ((pThis->m21_pos2Mode & 1) == 0 || (pThis->m21_pos2Mode & 2) == 0)
    {
        targetPos = *pThis->m10_pPosition2;
    }
    else
    {
        transformAndAddVec(*pThis->m10_pPosition2, targetPos, *(cameraProperties2.m28 + 1));
    }
    readAnimKeyframe(pThis->m18_targetAnimData, pThis->m8_currentFrame, keyframeType, targetPos);

    // Write to engine camera fields
    pEngine->m3E8 = camPos - sVec3_FP(
        pGrid->m1C[0] + pGrid->m28[0],
        pGrid->m1C[1] + pGrid->m28[1],
        pGrid->m1C[2] + pGrid->m28[2]);
    pEngine->m3F4_cameraPositionWhileShooting = targetPos;

    pEngine->m3E8 += pEngine->m1A0_battleAutoScrollDelta;
    pEngine->m3F4_cameraPositionWhileShooting += pEngine->m1A0_battleAutoScrollDelta;

    // Apply rotation anim to grid if present
    if (pThis->m1C_rotationAnimData)
    {
        u8* p = pThis->m1C_rotationAnimData + pThis->m8_currentFrame * 2;
        pGrid->mB4_cameraRotation.m8_Z = (s32)READ_BE_S16(p) << 16;
    }

    battleEngine_resetCameraInterpolation();
    battleEngine_restoreCameraDefaultSub0();

    pThis->m8_currentFrame++;
    if (pThis->m8_currentFrame >= pThis->mA_totalFrames)
    {
        pThis->getTask()->markFinished();
    }
}

// BTL_A3::0605f6ec
p_workArea createItemVisualEffect(npcFileDeleter* pFileBundle, u8 effectIndex, sVec3_FP* pPosition1, sVec3_FP* pPosition2, u8 param5, u8 param6)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    sItemVisualEffectTask* pNewTask = createSubTaskFromFunctionWithCopy<sItemVisualEffectTask>(pFileBundle, &sItemVisualEffectTask_Update);
    if (pNewTask == nullptr)
    {
        return nullptr;
    }
    pNewTask->m_DeleteMethod = (void(*)(sItemVisualEffectTask*))&sItemVisualEffectTask_Delete;

    // Read animation header from file bundle at effectIndex offset
    // The raw buffer is Saturn big-endian data; offsets within are file-relative
    u8* rawBuffer = pFileBundle->m0_fileBundle->getRawBuffer();
    u32 headerOffset = READ_BE_U32(rawBuffer + effectIndex);
    u8* animHeader = rawBuffer + headerOffset;

    pNewTask->m8_currentFrame = 0;
    pNewTask->mA_totalFrames = READ_BE_S16(animHeader + 0xC);
    pNewTask->m22_dataType = animHeader[0xF] & 0xF;

    // Conditionally read camera/target/rotation animation data pointers
    // flags are in byte 0xF: bit4=camera, bit5=target, bit6=rotation
    u8 flags = animHeader[0xF];
    pNewTask->m14_cameraAnimData = (flags & 0x10) ? rawBuffer + READ_BE_U32(animHeader + 0) : nullptr;
    pNewTask->m18_targetAnimData = (flags & 0x20) ? rawBuffer + READ_BE_U32(animHeader + 4) : nullptr;
    pNewTask->m1C_rotationAnimData = (flags & 0x40) ? rawBuffer + READ_BE_U32(animHeader + 8) : nullptr;

    // Set up position pointers based on mode flags
    pNewTask->m20_pos1Mode = param5;
    if (param5 & 1)
    {
        pNewTask->mC_pPosition1 = pPosition1; // store pointer directly
    }
    else
    {
        sVec3_FP* pCopy = (sVec3_FP*)allocateHeapForTask(pNewTask, sizeof(sVec3_FP));
        if (param5 & 2)
        {
            transformAndAddVec(*pPosition1, *pCopy, *(cameraProperties2.m28 + 1));
        }
        else
        {
            *pCopy = *pPosition1;
        }
        pNewTask->mC_pPosition1 = pCopy;
    }

    pNewTask->m21_pos2Mode = param6;
    if (param6 & 1)
    {
        pNewTask->m10_pPosition2 = pPosition2;
    }
    else
    {
        sVec3_FP* pCopy = (sVec3_FP*)allocateHeapForTask(pNewTask, sizeof(sVec3_FP));
        if (param6 & 2)
        {
            transformAndAddVec(*pPosition2, *pCopy, *(cameraProperties2.m28 + 1));
        }
        else
        {
            *pCopy = *pPosition2;
        }
        pNewTask->m10_pPosition2 = pCopy;
    }

    battleEngine_setDesiredCameraPositionPointer(&pEngine->m3F4_cameraPositionWhileShooting);
    battleEngine_setCurrentCameraPositionPointer(&pEngine->m3E8);
    battleEngine_enableAttackCamera();
    sEnemyAttackCamera_updateSub0(1);

    pEngine->m3D0 = pNewTask;

    return pNewTask;
}
