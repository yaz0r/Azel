#include "PDS.h"
#include "o_fld_c8.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "processModel.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/fieldDragonMovement.h"

s32 func3dModelSub0(s_3dModel* r4);
void update3dModelDrawFunctionForVertexAnimation(s_3dModel* r4, u8* pData);

// Forward declarations for shared sub-structures
struct s_fieldLCSSubStruct
{
    p_workArea m0_owner;
    void* m4_callback;
    sVec3_FP* m8_positionPtr;
    s32 mC;
    s16 m10;
    s16 m12;
    s32 m14;
    u8 m16;
    u8 m17;
    u8 m18;
    u8 m19;
    u8 m1A;
    u8 m1B;
    s32 m1C;
    s32 m20;
    // size 0x24
};

static void initFieldLCSSubStruct(s_fieldLCSSubStruct* pLCS, p_workArea owner, void* callback,
    sVec3_FP* posPtr, s32 param5, s16 param6, s16 param7, s16 param8, u8 param9, u8 param10);
static s32 clipCheck_C8_frustum(const sVec3_FP* pPos, s32 farClip);
static void readSaturnVec3Into(sSaturnPtr src, sVec3_FP* dst);
static void FUN_FLD_C8_0607d81a(sVec3_FP* pOut);
struct s_C8_cutsceneCameraTask;
static void FUN_FLD_C8_0605c87c(s_C8_cutsceneCameraTask* pTask, u8* pArgData);
static void FUN_FLD_C8_0607d600(sVec3_FP* pPos, sVec3_FP* pAngle);

// Shared dragon update functions (declared in o_fld_a3.cpp)
void dragonIdleUpdate(s_dragonTaskWorkArea*);
void dragonCutsceneUpdate(s_dragonTaskWorkArea*);
void dragonFlightUpdate(s_dragonTaskWorkArea*);
void updateCutsceneCameraInterpolation(sFieldCameraManager* r4, sFieldCameraStatus* r5);

// 0608948a = dragonCutsceneUpdate
// 0608930e = dragonScriptMovement

// 060876d8 — keyboard input handler (C8-specific, A3 asserts on keyboard)
static void FUN_FLD_C8_060876d8(s_dragonTaskWorkArea*);  // forward decl

// 06088f70 = dragonFlightUpdate (shared, with keyboard fallback in C8)
// Used directly via dragonFlightUpdate function pointer

// 0608904c = floaterFlightUpdate (shared, in fieldDragonMovement.cpp)


// 060876d8 — keyboard input (C8-specific, A3 asserts on keyboard)
static void FUN_FLD_C8_060876d8(s_dragonTaskWorkArea* r14) { Unimplemented(); }

// --- Minimal task structs for field infrastructure ---

struct s_C8_lightTask : public s_workAreaTemplate<s_C8_lightTask>
{
    s8 m0_phase;
    // size 0x01

    // 06060d90
    static void setLightPhase0() {
        mainGameState.setPackedBits(0x595, 2, 1);
        mainGameState.setPackedBits(0x597, 2, 0);
    }
    // 06060dac
    static void setLightPhase4(s_C8_lightTask* pThis) {
        mainGameState.setPackedBits(0x595, 2, 0);
        mainGameState.setPackedBits(0x597, 2, 1);
        pThis->m0_phase = 4;
    }
    // 06060dd8
    static void Init(s_C8_lightTask* pThis) {
        pThis->m0_phase = (s8)mainGameState.readPackedBits(0x591, 4);
        if (getFieldTaskPtr()->m2C_currentFieldIndex == 0x12 && pThis->m0_phase < 4) {
            setLightPhase4(pThis);
        }
    }
    // 06060e0c
    static void Update(s_C8_lightTask* pThis) {
        if (pThis->m0_phase == 0) {
            setLightPhase0();
            pThis->m0_phase++;
        }
        else if (pThis->m0_phase == 2) {
            if (getFieldTaskPtr()->m2C_currentFieldIndex == 0x12) {
                pThis->m0_phase = 3;
            }
        }
        else if (pThis->m0_phase == 3) {
            setLightPhase4(pThis);
        }
        mainGameState.setPackedBits(0x591, 4, (u32)pThis->m0_phase);
    }
    // 06060e7c — debug text only
    static void Draw(s_C8_lightTask*) {}

    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { &Init, &Update, &Draw, nullptr };
        return &td;
    }
};

struct s_C8_paletteAnimTask : public s_workAreaTemplate<s_C8_paletteAnimTask>
{
    sSaturnPtr m0_colorTableEA;
    u16 m4_palette[16];
    s16 m24_cramOffset;
    s16 m26_counter;
    s16 m28_speed;
    s8 m2A_currentIndex;
    // size 0x2C

    // 0605d59a — read 16 colors from table starting at current index
    static void readPaletteColors(s_C8_paletteAnimTask* pThis)
    {
        s32 idx = pThis->m2A_currentIndex;
        for (s32 i = 0; i < 16; i += 4)
        {
            pThis->m4_palette[i] = readSaturnU16(pThis->m0_colorTableEA + idx * 2);
            idx++; if (idx > 15) idx = 0;
            pThis->m4_palette[i + 1] = readSaturnU16(pThis->m0_colorTableEA + idx * 2);
            idx++; if (idx > 15) idx = 0;
            pThis->m4_palette[i + 2] = readSaturnU16(pThis->m0_colorTableEA + idx * 2);
            idx++; if (idx > 15) idx = 0;
            pThis->m4_palette[i + 3] = readSaturnU16(pThis->m0_colorTableEA + idx * 2);
            idx++; if (idx > 15) idx = 0;
        }
    }

    // 0605d580
    static void Init(s_C8_paletteAnimTask* pThis) {
        pThis->m0_colorTableEA = gFLD_C8->getSaturnPtr(0x060B27E8);
        pThis->m24_cramOffset = 0x20;
        pThis->m28_speed = 0x10;
        readPaletteColors(pThis);
    }
    // 0605d6a2 / 0605d62c
    static void Update(s_C8_paletteAnimTask* pThis) {
        pThis->m26_counter += pThis->m28_speed;
        if (pThis->m26_counter > 0xF)
        {
            s16 shift = pThis->m26_counter / 0x10;
            pThis->m2A_currentIndex -= (s8)shift;
            pThis->m26_counter = pThis->m26_counter % 0x10;
            if (pThis->m2A_currentIndex < 0) pThis->m2A_currentIndex = 0xF;
            readPaletteColors(pThis);
            asyncDmaCopy(&pThis->m4_palette, getVdp2Cram(pThis->m24_cramOffset * 0x20), 0x20, 0);
        }
    }

    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { &Init, &Update, nullptr, nullptr };
        return &td;
    }
};

struct s_C8_soundEmitterTask : public s_workAreaTemplateWithArg<s_C8_soundEmitterTask, s32>
{
    // 0x70 bytes of sound source data
    s16 m6C_soundId;
    // size 0x70

    // 06064144
    static void Init(s_C8_soundEmitterTask* pThis, s32 soundId) {
        s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
        pFieldData->mD8_soundEmitter = pThis;
        pThis->m6C_soundId = (s16)soundId;
    }
    // 0606416a
    static void Update(s_C8_soundEmitterTask*) {
        Unimplemented(); // complex 3D sound positioning with findSound/playBattleSoundEffect
    }
    // 060642a2 — empty
    static void Draw(s_C8_soundEmitterTask*) {}
    // 060642a6
    static void Delete(s_C8_soundEmitterTask* pThis) {
        if (pThis->m6C_soundId > 0) {
            playSystemSoundEffect(pThis->m6C_soundId);
        }
    }
    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { &Init, &Update, &Draw, &Delete };
        return &td;
    }
};

struct s_C8_effectManagerTask : public s_workAreaTemplateWithArg<s_C8_effectManagerTask, s32>
{
    u8* m0_indexTable;
    u8* m4_activeList;
    u8* m8_effectPool;
    s16 mC_activeCount;
    s16 mE_maxCount;
    // size 0x10

    // 06080ffc
    static void Init(s_C8_effectManagerTask* pThis, s32 maxCount) {
        pThis->m0_indexTable = (u8*)allocateHeapForTask(pThis, maxCount);
        pThis->m4_activeList = (u8*)allocateHeapForTask(pThis, maxCount);
        pThis->m8_effectPool = (u8*)allocateHeapForTask(pThis, maxCount * 0x30);
        pThis->mC_activeCount = 0;
        pThis->mE_maxCount = (s16)maxCount;
        for (s32 i = 0; i < maxCount; i++) {
            pThis->m0_indexTable[i] = (u8)i;
            *(s32*)(pThis->m8_effectPool + i * 0x30 + 0x28) = 0;
        }
    }
    // 060810ea — per-particle update: position += velocity, velocity decays, gravity, lifetime
    static bool updateParticle(s32* p) {
        p[0] += p[6]; p[1] += p[7]; p[2] += p[8];
        p[3] = p[0]; p[4] = p[1]; p[5] = p[2];
        p[6] -= p[6] >> 3;
        p[8] -= p[8] >> 3;
        p[7] -= p[7] >> 4;
        p[7] += -0xB0; // gravity
        p[3] -= p[6]; p[4] -= p[7]; p[5] -= p[8];
        *(s16*)(p + 0xB) = *(s16*)(p + 0xB) - *(s16*)((u8*)p + 0x2E);
        s32 life = p[10];
        p[10] = life - 1;
        return (life - 1) == 0;
    }
    // 06080f7a — remove particle from active list
    static void removeParticle(s_C8_effectManagerTask* pThis, s32 index) {
        if (pThis->mC_activeCount > 0) {
            pThis->mC_activeCount--;
            pThis->m0_indexTable[pThis->mC_activeCount] = pThis->m4_activeList[index];
            pThis->m4_activeList[index] = pThis->m4_activeList[pThis->mC_activeCount];
        }
    }
    // 06081184
    static void Update(s_C8_effectManagerTask* pThis) {
        s32 i = 0;
        while (i < pThis->mC_activeCount) {
            s32 entryOffset = pThis->m4_activeList[i] * 0x30;
            bool finished = updateParticle((s32*)(pThis->m8_effectPool + entryOffset));
            s32 next = i;
            if (finished) {
                removeParticle(pThis, i);
                next = i - 1;
            }
            i = next + 1;
        }
    }
    // 060811cc
    static void Draw(s_C8_effectManagerTask* pThis) {
        Unimplemented(); // per-particle draw (06081224 has incomplete Ghidra decompilation)
    }
    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { &Init, &Update, &Draw, nullptr };
        return &td;
    }
};

// 0607df7c — set dragon speed index and speed from table
static void FUN_FLD_C8_0607df7c(s32 speedIndex)
{
    if (speedIndex >= 0 && speedIndex < 5)
    {
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        pDragon->m235_dragonSpeedIndex = (s8)speedIndex;
        pDragon->m154_dragonSpeed = pDragon->m21C_DragonSpeedValues[pDragon->m235_dragonSpeedIndex];
        s32 midSpeed = ((s32)pDragon->m21C_DragonSpeedValues[0] + (s32)pDragon->m21C_DragonSpeedValues[1] + 1) >> 1;
        if ((s32)pDragon->m154_dragonSpeed < midSpeed)
        {
            pDragon->m238 = 4;
            pDragon->m237 = 4;
        }
        else
        {
            pDragon->m238 = 0;
            pDragon->m237 = 0;
        }
    }
}

// 0607d9c2 = initDragonMovementMode (same shared function)

// 0606a488 — check if camera slot is initialized (reads m8C at offset 0x470 per slot)
static s32 isCameraSlotActive(s16 slotIndex)
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    return (s32)pOverlay->m3E4_cameraSlots[slotIndex].m8C_isActive;
}

// 0606a4d4 — clear camera status fields
static void resetCameraStatus(sFieldCameraStatus* pStatus)
{
    pStatus->m5C_rotationSpring = {};
    pStatus->m68_rotationImpulse = {};
    pStatus->m0_position = {};
    pStatus->mC_rotation = {};
    pStatus->m28 = fixedPoint(0);
    pStatus->m18 = fixedPoint(0);
    pStatus->m2C = 0;
    pStatus->m1C = fixedPoint(0);
    pStatus->m30 = 0;
    pStatus->m24_distanceToDestination = 0xF000;
    pStatus->m40 = fixedPoint(0xF000);
    pStatus->m20 = fixedPoint(0);
    pStatus->m34 = fixedPoint(0);
    pStatus->m80_frameCounter = 0;
    pStatus->m84 = 0;
    pStatus->m8D_followState = 0;
    pStatus->m8E_followSubState = 0;
}

// 0606a512
static void initCameraSlot(s16 slotIndex, void(*param2)(sFieldCameraStatus*), void(*param3)(sFieldCameraStatus*))
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    sFieldCameraStatus* pStatus = &pOverlay->m3E4_cameraSlots[slotIndex];
    resetCameraStatus(pStatus);
    pStatus->m74_updateFunc = param2;
    pStatus->m78_drawFunc = param3;
    pStatus->m8C_isActive = 1;
}

// 0606a558
static void deactivateCameraSlot(s16 slotIndex)
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    sFieldCameraStatus* pStatus = &pOverlay->m3E4_cameraSlots[slotIndex];
    pStatus->m74_updateFunc = nullptr;
    pStatus->m78_drawFunc = nullptr;
    pStatus->m8C_isActive = 0;
}

// 0606a3ec
static s32 selectCameraSlot(s16 slotIndex)
{
    s32 check = isCameraSlotActive(slotIndex);
    if (check != 0)
    {
        sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
        pOverlay->m50C_activeCameraSlot = (u8)slotIndex;
        pOverlay->m3E4_cameraSlots[slotIndex].m80_frameCounter = 0;
        return 1;
    }
    return 0;
}

// 0606a590 — get current camera status pointer
static sFieldCameraStatus* getCurrentCameraStatus()
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    return &pOverlay->m3E4_cameraSlots[pOverlay->m50C_activeCameraSlot];
}

// 0606941e — angle smoothing with clamped step (28-bit normalized)
static s32 FUN_FLD_C8_0606941e(s32 current, s32 target, s32 speed, s32 maxStep, s32 minStep)
{
    s32 diff = fixedPoint(target - current).normalized();
    s32 step = (s32)MTH_Mul(fixedPoint(speed), fixedPoint(diff));
    if (diff < 0) {
        minStep = -minStep; maxStep = -maxStep;
        s32 clamped = (step < minStep) ? step : minStep;
        if (maxStep < clamped) { step = (minStep <= step) ? minStep : step; }
        else { step = maxStep; }
        diff = fixedPoint(diff - step).normalized();
        if (diff >= 0) target = current + step;
    } else {
        s32 clamped = (minStep <= step) ? step : minStep;
        if (clamped <= maxStep) { step = (step < minStep) ? minStep : step; }
        else { step = minStep; }
        diff = fixedPoint(diff - step).normalized();
        if (diff < 0) target = current + step;
    }
    return target;
}

// 06069398 — linear smoothing with clamped step
static s32 FUN_FLD_C8_06069398(s32 current, s32 target, s32 speed, s32 maxStep, s32 minStep)
{
    s32 diff = target - current;
    s32 step = (s32)MTH_Mul(fixedPoint(speed), fixedPoint(diff));
    if (diff < 0) {
        minStep = -minStep; maxStep = -maxStep;
        s32 clamped = (step < minStep) ? step : minStep;
        if (maxStep < clamped) { step = (minStep <= step) ? minStep : step; }
        else { step = maxStep; }
        if (diff - step > 0) return target;
    } else {
        s32 clamped = (minStep <= step) ? step : minStep;
        if (clamped <= maxStep) { step = (step < minStep) ? minStep : step; }
        else { step = minStep; }
        if (diff - step < 0) return target;
    }
    return current + step;
}

// 0606b2cc — same as updateCutsceneCameraInterpolation (060625d8 in A3)
// Camera interpolation using m370/m374 pointers. Shared across all field overlays.

// 0607da24
static void FUN_FLD_C8_0607da24()
{
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF8_Flags &= ~0x10000;
}

// 0607d98e = dragonFieldTaskInitSub4Sub4 (same shared function)

// 0606a42e — set camera tracking for slot
static s32 setCameraSlotFunctions(s16 slotIndex, void(*param2)(sFieldCameraStatus*), void(*param3)(sFieldCameraStatus*))
{
    s32 check = isCameraSlotActive(slotIndex);
    if (check != 0)
    {
        sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
        sFieldCameraStatus* pStatus = &pOverlay->m3E4_cameraSlots[slotIndex];
        pStatus->m74_updateFunc = param2;
        pStatus->m78_drawFunc = param3;
        pStatus->m8D_followState = 0;
        pStatus->m8E_followSubState = 0;
        return 1;
    }
    return 0;
}

// 0606a608 = activateCameraFollowMode (same shared function)
void activateCameraFollowMode(u32 r4); // declared in o_fld_a3.cpp

// 0606b532 — restore camera from cutscene
static void restoreCameraFromCutscene()
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    pOverlay->m378_cutsceneFrameCounter = 0;
    pOverlay->m37C_isCutsceneCameraActive = 0;
    activateDragonFlight();
    deactivateCameraSlot(1);
    // Copy only position/rotation/params from slot 1 to slot 0 (first 40 bytes, NOT the whole struct)
    sFieldCameraStatus& dst = pOverlay->m3E4_cameraSlots[0];
    sFieldCameraStatus& src = pOverlay->m3E4_cameraSlots[1];
    dst.m0_position = src.m0_position;
    dst.mC_rotation = src.mC_rotation;
    dst.m18 = src.m18;
    dst.m1C = src.m1C;
    dst.m20 = src.m20;
    dst.m24_distanceToDestination = src.m24_distanceToDestination;
    selectCameraSlot(0);
    activateCameraFollowMode((s32)pOverlay->m50E_followModeIndex);
}

// 0606b4a0 — setup camera parameters on overlay subtask
static void setupCutsceneCamera(sVec3_FP* param1, sVec3_FP* param2)
{
    sFieldCameraManager* pOverlay = getFieldTaskPtr()->m8_pSubFieldData->m334;
    pOverlay->m370_cutsceneLookAtPtr = param1;
    pOverlay->m374_cutsceneCameraPos = param2;
    pOverlay->m378_cutsceneFrameCounter = 0;
    pOverlay->m37C_isCutsceneCameraActive = 1;
    initDragonMovementMode();
    initCameraSlot(1, nullptr, nullptr);
    selectCameraSlot(1);
    sFieldCameraStatus* pCamStatus = getCurrentCameraStatus();
    updateCutsceneCameraInterpolation(pOverlay, pCamStatus);
}

// 06073e80
static void FUN_FLD_C8_06073e80()
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m64 = 1;
}

// 0607a118
static void FUN_FLD_C8_0607a118(s32 param1, s32 param2, s16 param3)
{
    FUN_FLD_C8_06073e80();
    exitCutsceneTaskUpdateSub0Sub1(
        getFieldTaskPtr()->m2C_currentFieldIndex, param1, param2, (s32)param3);
}

struct s_C8_cutsceneCameraArg
{
    sVec3_FP m0_position;
    s16 mC_rotation;
    s16 mE_pad;
    s32 m10_distance;
    s32 m14_speed;
    s32 m18[3];
    s32 m24_flags;
    s16 m28_param;
};

struct s_C8_cutsceneCameraTask : public s_workAreaTemplateWithArg<s_C8_cutsceneCameraTask, s_C8_cutsceneCameraArg*>
{
    s_scriptData3* m0_pathData;
    s32 m4_frameCounter;
    s32 m8_maxFrames;
    s32 mC_exitParam1;
    s32 m10_exitParam2;
    s32 m14_flags;
    s16 m18_exitParam3;
    s8 m1A_state;
    // size 0x1C

    // 0605ca20
    static void Init(s_C8_cutsceneCameraTask* pThis, s_C8_cutsceneCameraArg* pArg) {
        pThis->m4_frameCounter = 0;
        pThis->m8_maxFrames = pArg->m18[0];
        pThis->mC_exitParam1 = pArg->m18[1];
        pThis->m10_exitParam2 = pArg->m18[2];
        pThis->m14_flags = pArg->m24_flags;
        pThis->m18_exitParam3 = pArg->m28_param;
        // 0605c87c
        FUN_FLD_C8_0605c87c(pThis, (u8*)pArg);
        // 0606b4a0
        s_fieldSpecificData_C8* pFD = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
        setupCutsceneCamera(&pFD->m64_cameraTarget, &pFD->m4C_dragonPos);
        pThis->m1A_state = 0;
    }
    // 0607e03a
    static void FUN_FLD_C8_0607e03a(s_scriptData3* pData) {
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        pDragon->m1E4_cutsceneKeyFrame = pData;
        pDragon->m104_dragonScriptStatus = 0;
        pDragon->mF0 = dragonCutsceneUpdate;
        pDragon->mF8_Flags &= ~0x400;
    }

    // 0607e088
    static bool FUN_FLD_C8_0607e088() {
        return getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m1E4_cutsceneKeyFrame == nullptr;
    }

    // 0605ca6a
    static void Update(s_C8_cutsceneCameraTask* pThis) {
        s32 state = pThis->m1A_state;
        if (state == 0) {
            FUN_FLD_C8_0607e03a(pThis->m0_pathData);
            pThis->m1A_state++;
        }
        else if (state != 1) {
            return;
        }

        // 0607d81a — read dragon position into field_C + 0x4C
        s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
        FUN_FLD_C8_0607d81a(&pFieldData->m4C_dragonPos);

        bool done = false;
        if ((pThis->m14_flags & 1) != 0)
        {
            pThis->m4_frameCounter++;
            if (pThis->m4_frameCounter >= pThis->m8_maxFrames)
                done = true;
        }

        if (FUN_FLD_C8_0607e088())
            done = true;

        if (done)
        {
            if ((pThis->m14_flags & 1) == 0)
            {
                restoreCameraFromCutscene();
                if (pThis)
                    pThis->getTask()->m14_flags |= 1;
            }
            else
            {
                playSystemSoundEffect(0x6A);
                playSystemSoundEffect(0x72);
                FUN_FLD_C8_0607a118(pThis->mC_exitParam1, pThis->m10_exitParam2, pThis->m18_exitParam3);
                pThis->m1A_state++;
            }
        }
    }
    // 0605cb44 — empty
    static void Draw(s_C8_cutsceneCameraTask*) {}

    static const TypedTaskDefinition* getTypedTaskDefinition() {
        static const TypedTaskDefinition td = { &Init, &Update, &Draw, nullptr };
        return &td;
    }
};

// 0607d600 — set dragon position and angle
static void FUN_FLD_C8_0607d600(sVec3_FP* pPos, sVec3_FP* pAngle)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    getCurrentCameraStatus();
    if (pPos)
        pDragon->m8_pos = *pPos;
    if (pAngle)
    {
        pDragon->m20_angle = *pAngle;
        Unimplemented(); // 06080c0c — blocked
    }
    Unimplemented(); // 060899dc — blocked
    updateCameraScriptSub0(pDragon->mB8_lightWingEffect);
}

// 0605c87c — camera movement setup
static void FUN_FLD_C8_0605c87c(s_C8_cutsceneCameraTask* pTask, u8* pArgData)
{
    sVec3_FP offset = { fixedPoint(0), fixedPoint(0), fixedPoint(*(s32*)(pArgData + 0x10)) };
    s_scriptData3* pPath = (s_scriptData3*)allocateHeapForTask(pTask, sizeof(s_scriptData3));
    pTask->m0_pathData = pPath;
    pPath->m0_duration = *(s32*)(pArgData + 0x14); // speed
    pPath->m10_rotationDuration = *(s32*)(pArgData + 0x14) >> 4;
    pPath->m14_rot = {};

    sVec3_FP camPos;
    s32 camAngle;
    sMatrix4x3 mat;

    if ((pTask->m14_flags & 1) == 0)
    {
        initMatrixToIdentity(&mat);
        sVec3_FP entryPos = { *(fixedPoint*)(pArgData), *(fixedPoint*)(pArgData + 4), *(fixedPoint*)(pArgData + 8) };
        translateMatrix(entryPos, &mat);
        rotateMatrixY((s32)*(s16*)(pArgData + 0xC), &mat);
        transformAndAddVec(offset, camPos, mat);
        camAngle = (s32)*(s16*)(pArgData + 0xC) * 0x10000 + 0x8000000;
    }
    else
    {
        camPos = { *(fixedPoint*)(pArgData), *(fixedPoint*)(pArgData + 4), *(fixedPoint*)(pArgData + 8) };
        camAngle = (s32)*(s16*)(pArgData + 0xC) << 16;
    }
    pPath->m4_pos = camPos;
    pPath->m14_rot.m4_Y = fixedPoint(camAngle);

    initMatrixToIdentity(&mat);
    if ((pTask->m14_flags & 1) == 0)
    {
        translateMatrix(pPath->m4_pos, &mat);
        rotateMatrixShiftedY((s32)pPath->m14_rot.m4_Y + 0x8000000, &mat);
    }
    else
    {
        sVec3_FP entryPos = { *(fixedPoint*)(pArgData), *(fixedPoint*)(pArgData + 4), *(fixedPoint*)(pArgData + 8) };
        translateMatrix(entryPos, &mat);
        rotateMatrixShiftedY(pPath->m14_rot.m4_Y, &mat);
        translateMatrix(offset, &mat);
    }

    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    sVec3_FP targetOffset = readSaturnVec3(gFLD_C8->getSaturnPtr(0x060B27BC));
    transformAndAddVec(targetOffset, pFieldData->m64_cameraTarget, mat);

    if ((pTask->m14_flags & 1) != 0)
    {
        initMatrixToIdentity(&mat);
        sVec3_FP entryPos = { *(fixedPoint*)(pArgData), *(fixedPoint*)(pArgData + 4), *(fixedPoint*)(pArgData + 8) };
        translateMatrix(entryPos, &mat);
        rotateMatrixY((s32)*(s16*)(pArgData + 0xC), &mat);
        sVec3_FP computedPos;
        transformAndAddVec(offset, computedPos, mat);
        FUN_FLD_C8_0607d600(&computedPos, &pPath->m14_rot);
    }
    else
    {
        sVec3_FP entryPos = { *(fixedPoint*)(pArgData), *(fixedPoint*)(pArgData + 4), *(fixedPoint*)(pArgData + 8) };
        FUN_FLD_C8_0607d600(&entryPos, &pPath->m14_rot);
    }
}

// 0605cb58
static void FUN_FLD_C8_0605cb58(p_workArea parent, s_C8_cutsceneCameraArg* pArg)
{
    createSubTaskWithArg<s_C8_cutsceneCameraTask>(parent, pArg);
}

// 06060f4c
static void FUN_FLD_C8_06060f4c(p_workArea workArea)
{
    createSubTask<s_C8_lightTask>(workArea);
}

// 0605fe10
static void FUN_FLD_C8_0605fe10()
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    pFieldData->m38 = 0;
    pFieldData->m3C = 0;
}

// 06056510
static void FUN_FLD_C8_06056510()
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    pFieldData->m28 = 0;
    pFieldData->m2C = 0;
}

// 0605f030 — empty function
static void FUN_FLD_C8_0605f030() {}

// --- Minimal task structs for createSubTaskFromFunction ---

struct s_C8_poolTask : public s_workAreaTemplate<s_C8_poolTask>
{
    s_memoryAreaOutput m0;
    u8* m8_indexTable;
    u8* mC_activeList;
    u8* m10_pool;
    s16 m14_activeCount;
    s16 m16_maxCount;
    s32 m18;
    s32 m1C;
    // size 0x20
};

struct s_C8_eventTriggerTask : public s_workAreaTemplate<s_C8_eventTriggerTask>
{
    sSaturnPtr m0_data;
    // size 4
};

struct s_C8_boundsCheckTask : public s_workAreaTemplate<s_C8_boundsCheckTask>
{
    sSaturnPtr m0_data;
    // size 4
};

struct s_C8_zoneTask : public s_workAreaTemplate<s_C8_zoneTask>
{
    sSaturnPtr m0_table0;
    sSaturnPtr m4_table1;
    // size 8
};

// 060815d0 — create particle/effect pool (size 0x20)
static p_workArea FUN_FLD_C8_060815d0(p_workArea workArea, s32 param2, s32 param3)
{
    s_C8_poolTask* pTask = createSubTaskFromFunction<s_C8_poolTask>(workArea, nullptr);
    if (pTask)
    {
        getMemoryArea(&pTask->m0, param2);
        pTask->m16_maxCount = (s16)param3;
        s32 poolSize = param3 * 0x34;
        pTask->m8_indexTable = (u8*)allocateHeapForTask(pTask, param3);
        pTask->mC_activeList = (u8*)allocateHeapForTask(pTask, param3);
        pTask->m10_pool = (u8*)allocateHeapForTask(pTask, poolSize);
        pTask->m14_activeCount = 0;
        for (s32 i = 0; i < param3; i++)
        {
            pTask->m8_indexTable[i] = (u8)i;
            *(s32*)(pTask->m10_pool + i * 0x34 + 0x28) = 0;
        }
        pTask->mC_activeList = pTask->m8_indexTable; // m_C = m8 initially
        pTask->m18 = 0;
        pTask->m1C = 0;
        Unimplemented(); // set Update/Draw to 060814d8/06081584
    }
    return pTask;
}

// 0605960c — event trigger update
static void FUN_FLD_C8_0605960c(s_C8_eventTriggerTask* pThis)
{
    // Check if dragon is within trigger bounds, call callback, kill task
    Unimplemented(); // needs FUN_FLD_C8_0607d81a + bounds check + callback dispatch
}

// 06059666 — create event trigger subtask per subfield
static void FUN_FLD_C8_06059666(p_workArea workArea)
{
    if ((mainGameState.bitField[0xA6] & 0x10) == 0)
    {
        s_C8_eventTriggerTask* pTask = createSubTaskFromFunction<s_C8_eventTriggerTask>(workArea, &FUN_FLD_C8_0605960c);
        if (pTask)
        {
            s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
            if (subfield == 0)
                pTask->m0_data = gFLD_C8->getSaturnPtr(0x0608E4F0);
            else if (subfield == 0x1A)
                pTask->m0_data = gFLD_C8->getSaturnPtr(0x0608E510);
            else
            {
                // Kill task for unsupported subfields
                pTask->getTask()->m14_flags |= 1;
            }

            // Check game state bit — if already triggered, kill task
            if (!pTask->m0_data.isNull())
            {
                s32 bitIndex = readSaturnS32(pTask->m0_data + 0x1C);
                if (bitIndex > 0 && mainGameState.getBit566(bitIndex))
                {
                    pTask->getTask()->m14_flags |= 1;
                }
            }
        }
    }
    else
    {
        // D4 variant — different subfield mapping
        Unimplemented();
    }
}

// 0605d6c4
static void FUN_FLD_C8_0605d6c4(p_workArea workArea)
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    pFieldData->mD4_subtask = createSubTask<s_C8_paletteAnimTask>(workArea);
}

// 0605cb90
static void FUN_FLD_C8_0605cb90(p_workArea workArea)
{
    FUN_FLD_C8_06060f4c(workArea);
    FUN_FLD_C8_0605fe10();
    FUN_FLD_C8_06056510();
    FUN_FLD_C8_0605f030();
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    pFieldData->mA0_particlePool = FUN_FLD_C8_060815d0(workArea, 1, 0x80);
    FUN_FLD_C8_06059666(workArea);
    FUN_FLD_C8_0605d6c4(workArea);
}

// 060642c4
static void FUN_FLD_C8_060642c4(p_workArea workArea, s32 param)
{
    createSubTaskWithArg<s_C8_soundEmitterTask>(workArea, param);
}

// 0608120a
static p_workArea FUN_FLD_C8_0608120a(p_workArea workArea, s32 param)
{
    return createSubTaskWithArg<s_C8_effectManagerTask>(workArea, param);
}

// 0607d81a — read dragon position
static void FUN_FLD_C8_0607d81a(sVec3_FP* pOut)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pOut->m0_X = pDragon->m8_pos.m0_X;
    pOut->m4_Y = pDragon->m8_pos.m4_Y;
    pOut->m8_Z = pDragon->m8_pos.m8_Z;
}

// 0605b0a0 — vertical bounds check update
static void FUN_FLD_C8_0605b0a0(s_C8_boundsCheckTask* pThis)
{
    if ((mainGameState.bitField[0xA6] & 4) == 0)
        return;

    // Read dragon position
    sVec3_FP dragonPos;
    FUN_FLD_C8_0607d81a(&dragonPos);

    // Scan table: entries are {bitIndex, minY, maxY} triplets (12 bytes each)
    // Find upper bound based on dragon Y
    sSaturnPtr tableEA = pThis->m0_data;
    sSaturnPtr upper = tableEA;
    while ((s32)dragonPos.m4_Y < (readSaturnS32(tableEA + 4) + readSaturnS32(tableEA + 8)) >> 1)
    {
        tableEA = tableEA + 12;
    }

    // Find first entry from top where bit is NOT set
    sSaturnPtr lower = tableEA;
    while (readSaturnS32(upper) != (s32)0xFFFFFFFF)
    {
        s32 bitIndex = readSaturnS32(upper);
        if (!mainGameState.getBit566(bitIndex))
            break;
        upper = upper + 12;
    }

    // Find last entry going down where bit is NOT set
    sSaturnPtr prev = tableEA;
    while (true)
    {
        sSaturnPtr check = prev - 12;
        if (readSaturnS32(check) == (s32)0xFFFFFFFF)
            break;
        s32 bitIndex = readSaturnS32(check);
        if (mainGameState.getBit566(bitIndex))
            break;
        prev = check;
    }

    adjustVerticalLimits(readSaturnS32(upper + 4), readSaturnS32(prev - 4));
}

// 0605b17c — create per-subfield vertical bounds check subtask
static void FUN_FLD_C8_0605b17c(p_workArea workArea)
{
    s_C8_boundsCheckTask* pTask = createSubTaskFromFunction<s_C8_boundsCheckTask>(workArea, &FUN_FLD_C8_0605b0a0);
    if (pTask)
    {
        s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
        if (subfield == 0)
            pTask->m0_data = gFLD_C8->getSaturnPtr(0x0609C45C);
        else if (subfield == 9)
            pTask->m0_data = gFLD_C8->getSaturnPtr(0x0609C438);
        else if (subfield == 0x1A)
            pTask->m0_data = gFLD_C8->getSaturnPtr(0x0609C408);
        else if (pTask)
            pTask->getTask()->m14_flags |= 1;
    }
}

// 06085ee2 — create 0xF0 subtask under grid cell
static void FUN_FLD_C8_06085ee2(sSaturnPtr dataEA)
{
    // 0607a48c — find grid cell for current position
    Unimplemented(); // needs grid cell lookup + createSubTaskWithArg with task def at 060b7eac
}

// 06064d8c
static void FUN_FLD_C8_06064d8c()
{
    FUN_FLD_C8_06085ee2(gFLD_C8->getSaturnPtr(0x060B4788));
}

// 0605b70a — zone collision update
static void FUN_FLD_C8_0605b70a(s_C8_zoneTask* pThis)
{
    s32 cameraY = (s32)cameraProperties2.m0_position.m4_Y;
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;

    // 0605b65e — search zone table 0
    {
        s32 idx = 0;
        while (readSaturnS32(pThis->m0_table0 + idx * 4) <= cameraY)
            idx++;
        pFieldData->mE0_zoneIndex0 = idx;
    }
    // 0605b68a — search zone table 1
    {
        s32 idx = 0;
        while (readSaturnS32(pThis->m4_table1 + idx * 4) <= cameraY)
            idx++;
        pFieldData->mE4_zoneIndex1 = idx;
    }
}

// 0605b75c — create zone collision subtask
static void FUN_FLD_C8_0605b75c(p_workArea workArea)
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    s_C8_zoneTask* pTask = createSubTaskFromFunction<s_C8_zoneTask>(workArea, &FUN_FLD_C8_0605b70a);
    pFieldData->mDC_zoneCollision = pTask;
    if (pTask)
    {
        s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
        if (subfield == 0)
        {
            pTask->m0_table0 = gFLD_C8->getSaturnPtr(0x0609CA54);
            pTask->m4_table1 = gFLD_C8->getSaturnPtr(0x0609CA94);
        }
        else if (subfield == 6)
        {
            pTask->m0_table0 = gFLD_C8->getSaturnPtr(0x0609CA64);
            pTask->m4_table1 = gFLD_C8->getSaturnPtr(0x0609CAA0);
        }
        else if (subfield == 9)
        {
            pTask->m0_table0 = gFLD_C8->getSaturnPtr(0x0609CA6C);
            pTask->m4_table1 = gFLD_C8->getSaturnPtr(0x0609CAA4);
        }
        else if (subfield == 0x17)
        {
            pTask->m0_table0 = gFLD_C8->getSaturnPtr(0x0609CA7C);
            pTask->m4_table1 = gFLD_C8->getSaturnPtr(0x0609CAB0);
        }
        else if (subfield == 0x1A)
        {
            pTask->m0_table0 = gFLD_C8->getSaturnPtr(0x0609CA84);
            pTask->m4_table1 = gFLD_C8->getSaturnPtr(0x0609CAB4);
        }
        else
        {
            pTask->getTask()->m14_flags |= 1;
        }
    }
}

// 0605cbf0
static void fieldC8_0_startTasks(p_workArea workArea)
{
    FUN_FLD_C8_0605cb90(workArea);
    FUN_FLD_C8_060642c4(workArea, 0x6A);
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    pFieldData->mB4_effectManager = FUN_FLD_C8_0608120a(workArea, 0x3F);
    FUN_FLD_C8_0605b17c(workArea);
    FUN_FLD_C8_06064d8c();
    FUN_FLD_C8_0605b75c(workArea);
}

// 0605f794 — create VDP2 task
static void createVdp2Task_C8_0(p_workArea workArea)
{
    createC8Vdp2Task(workArea, gFLD_C8->getSaturnPtr(0x060b2d74));
}

// --- Environment object task (0x24 bytes, task definition at 0609cabc) ---

struct s_envObjectC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_modelFrameDataEA;
    s32 mC_speed;
    s32 m10_groupCount;
};

struct s_envObjectC8 : public s_workAreaTemplateWithArg<s_envObjectC8, s_envObjectC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    sSaturnPtr mC_modelFrameDataEA;
    s32 m10_fraction;
    s32 m14_speed;
    s32 m18_frameIndex;
    s32 m1C_groupCount;
    s32 m20_currentGroup;
    // size 0x24

    static void Init(s_envObjectC8* pThis, s_envObjectC8_arg* pArg);
    static void Update(s_envObjectC8* pThis);
    static void Draw(s_envObjectC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 0605b824
void s_envObjectC8::Init(s_envObjectC8* pThis, s_envObjectC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mC_modelFrameDataEA = pArg->m8_modelFrameDataEA;
    pThis->m10_fraction = 0;
    pThis->m18_frameIndex = 0;
    pThis->m14_speed = pArg->mC_speed;
    pThis->m1C_groupCount = pArg->m10_groupCount;
    pThis->m20_currentGroup = 0;
}

// 0605b854
void s_envObjectC8::Update(s_envObjectC8* pThis)
{
    sSaturnPtr frameArrayEA = readSaturnEA(pThis->mC_modelFrameDataEA);

    s32 fraction = pThis->m10_fraction + pThis->m14_speed;
    pThis->m10_fraction = fraction;

    if (fraction > 0xFFFF)
    {
        pThis->m10_fraction = pThis->m10_fraction - 0x10000;
        s32 frameIndex = pThis->m18_frameIndex + 1;
        pThis->m18_frameIndex = frameIndex;

        if (readSaturnS32(frameArrayEA + pThis->m20_currentGroup * 8) <= frameIndex)
        {
            pThis->m18_frameIndex = 0;
            s32 group = pThis->m20_currentGroup + 1;
            pThis->m20_currentGroup = group;

            if (pThis->m1C_groupCount <= group)
            {
                pThis->m20_currentGroup = 0;
            }
        }
    }
}

// 06078044 — queue model for deferred rendering in visibility grid
static void envObjectC8_QueueModel(s_visibilityGridWorkArea* pGrid, sProcessed3dModel* pModel, fixedPoint param)
{
    s16 count = pGrid->m12E4_numCollisionGeometries + 1;
    pGrid->m12E4_numCollisionGeometries = count;
    if (count < 0x18)
    {
        pGrid->m44->m0_model = pModel;
        pGrid->m44->m34 = param;
        pGrid->m44++;
    }
}

// 0607840a — distance-culled per-node rendering
static void envObjectC8_DrawNode(sProcessed3dModel* pModel, fixedPoint param)
{
    if (pModel != nullptr)
    {
        fixedPoint radius = pModel->m0_radius + fixedPoint(0x8000);
        s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        fixedPoint dx = pCurrentMatrix->m[0][3] - pDragon->m8_pos.m0_X;
        if ((s32)dx < 0) dx = -dx;
        fixedPoint dy = pCurrentMatrix->m[1][3] - pDragon->m8_pos.m4_Y;
        if ((s32)dy < 0) dy = -dy;
        fixedPoint dz = pCurrentMatrix->m[2][3] - pDragon->m8_pos.m8_Z;
        if ((s32)dz < 0) dz = -dz;

        if ((s32)dx <= (s32)radius && (s32)dy <= (s32)radius && (s32)dz <= (s32)radius)
        {
            copyMatrix(pCurrentMatrix, &pGrid->m44->m4_matrix);
            if (pGrid->m12F2_renderMode == 1)
            {
                Unimplemented(); // debug collision rendering
            }
            envObjectC8_QueueModel(pGrid, pModel, param);
        }
    }
}

// 060785cc — recursive model hierarchy tree walker
static void envObjectC8_DrawHierarchy(sModelHierarchy* pNode, std::vector<sStaticPoseData::sBonePoseData>::const_iterator& pBone, fixedPoint param)
{
    do
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pBone->m0_translation);
        rotateCurrentMatrixZYX(&pBone->mC_rotation);

        if (pNode->m0_3dModel)
        {
            envObjectC8_DrawNode(pNode->m0_3dModel, param);
        }
        if (pNode->m4_subNode)
        {
            pBone++;
            envObjectC8_DrawHierarchy(pNode->m4_subNode, pBone, param);
        }

        popMatrix();

        if (pNode->m8_nextNode == nullptr)
            break;

        pBone++;
        pNode = pNode->m8_nextNode;
    } while (true);
}

// 06078726 — draw model hierarchy with billboard matrix
static void envObjectC8_DrawWithBillboard(s_fileBundle* pBundle, s16 hierarchyOffset, s16 poseOffset)
{
    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(hierarchyOffset);
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pHierarchy->countNumberOfBones());

    sMatrix4x3 savedMatrix;
    copyMatrix(pCurrentMatrix, &savedMatrix);
    pushCurrentMatrix();
    copyToCurrentMatrix(&cameraProperties2.m88_billboardViewMatrix);
    multiplyCurrentMatrix(&savedMatrix);

    std::vector<sStaticPoseData::sBonePoseData>::const_iterator bones = pPose->m0_bones.begin();
    envObjectC8_DrawHierarchy(pHierarchy, bones, fixedPoint(0));

    popMatrix();
}

// 0607877e
static void envObjectC8_DrawSub(s_memoryAreaOutput* pMemArea, s16 param1, s16 param2)
{
    envObjectC8_DrawWithBillboard(pMemArea->m0_mainMemoryBundle, param1, param2);
}

// 0605b898
void s_envObjectC8::Draw(s_envObjectC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;
    sSaturnPtr frameArrayEA = readSaturnEA(pThis->mC_modelFrameDataEA);
    sSaturnPtr frameEntryEA = frameArrayEA + pThis->m20_currentGroup * 8;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(entryEA + 4));

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(frameEntryEA + 4),
        readSaturnS16(frameEntryEA + 6));

    s16 extraParam1 = readSaturnS16(pThis->mC_modelFrameDataEA + 4);
    if (extraParam1 != 0)
    {
        envObjectC8_DrawSub(&pThis->m0_memoryArea, extraParam1, readSaturnS16(pThis->mC_modelFrameDataEA + 6));
    }

    popMatrix();
}

// 0605b8fe — create environment object task from entry data
static void createEnvObjectTask_C8(p_workArea gridCell, s_envObjectC8_arg* pArg)
{
    createSubTaskWithArg<s_envObjectC8>(gridCell, pArg);
}

// 0605b270
static void createEnvObject_C8_type0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C52C);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b294
static void createEnvObject_C8_type0b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C564);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 6;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b2b8
static void createEnvObject_C8_type1(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C614);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b2dc
static void createEnvObject_C8_type1b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C61C);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b300
static void createEnvObject_C8_type2(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C624);
    arg.mC_speed = 0x8000;
    arg.m10_groupCount = 7;
    createEnvObjectTask_C8(gridCell, &arg);
}

// 0605b324
static void createEnvObject_C8_type2b(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_envObjectC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_modelFrameDataEA = gFLD_C8->getSaturnPtr(0x0609C65C);
    arg.mC_speed = 0x5555;
    arg.m10_groupCount = 6;
    createEnvObjectTask_C8(gridCell, &arg);
}

// --- Static draw entity (0x10 bytes, task definition at 0608b434) ---

struct s_staticDrawC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    s16 m8_drawParam1;
    s16 mA_drawParam2;
};

struct s_staticDrawC8 : public s_workAreaTemplateWithArg<s_staticDrawC8, s_staticDrawC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s16 mC_drawParam1;
    s16 mE_drawParam2;
    // size 0x10

    static void Init(s_staticDrawC8* pThis, s_staticDrawC8_arg* pArg);
    static void Draw(s_staticDrawC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 06055c7c
void s_staticDrawC8::Init(s_staticDrawC8* pThis, s_staticDrawC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mC_drawParam1 = pArg->m8_drawParam1;
    pThis->mE_drawParam2 = pArg->mA_drawParam2;
}

// 06055c30
void s_staticDrawC8::Draw(s_staticDrawC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(entryEA + 4));

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(modelDataEA),
        readSaturnS16(modelDataEA + 2));

    envObjectC8_DrawSub(&pThis->m0_memoryArea, pThis->mC_drawParam1, pThis->mE_drawParam2);

    popMatrix();
}

// 06055cf4
static void createCellEntity_C8_06055cf4(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x28;
    arg.mA_drawParam2 = 0x5d4;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// 06055d14
static void createCellEntity_C8_06055d14(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x30;
    arg.mA_drawParam2 = 0x5dc;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// --- Simple model entity (0xC bytes, task definitions at 060b63d8/060b63e8) ---

struct s_simpleModelC8 : public s_workAreaTemplate<s_simpleModelC8>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    // size 0xC

    static void Draw0(s_simpleModelC8* pThis);
    static void Draw1(s_simpleModelC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, nullptr, nullptr, nullptr };
        return &taskDefinition;
    }
};

// 0607a2fc
void s_simpleModelC8::Draw0(s_simpleModelC8* pThis)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sSaturnPtr entryEA = pThis->m8_entryEA;

    sVec3_FP pos = readSaturnVec3(entryEA + 4);
    if (pGrid->m12FC_isObjectClipped && pGrid->m12FC_isObjectClipped(&pos, graphicEngineStatus.m405C.m14_farClipDistance))
    {
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pos);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(modelDataEA),
        readSaturnS16(modelDataEA + 2));

    popMatrix();
}

// 0607a398
void s_simpleModelC8::Draw1(s_simpleModelC8* pThis)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    sSaturnPtr entryEA = pThis->m8_entryEA;

    sVec3_FP pos = readSaturnVec3(entryEA + 4);
    if (pGrid->m12FC_isObjectClipped && pGrid->m12FC_isObjectClipped(&pos, graphicEngineStatus.m405C.m14_farClipDistance))
    {
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pos);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    if ((s32)depthRangeIndex <= pGrid->m1300)
    {
        sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
        s16 modelOffset = readSaturnS16(modelDataEA);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(modelOffset));
    }

    popMatrix();
}

// 0607a414
static void createCellEntity_C8_0607a414(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    s16 selector = readSaturnS16(modelDataEA + 2);

    static const s_simpleModelC8::TypedTaskDefinition taskDef0 = { nullptr, nullptr, &s_simpleModelC8::Draw1, nullptr };
    static const s_simpleModelC8::TypedTaskDefinition taskDef1 = { nullptr, nullptr, &s_simpleModelC8::Draw0, nullptr };

    const s_simpleModelC8::TypedTaskDefinition* pDef = (selector == 0) ? &taskDef0 : &taskDef1;

    s_simpleModelC8* pTask = createSubTask<s_simpleModelC8>(gridCell, pDef);
    if (pTask)
    {
        getMemoryArea(&pTask->m0_memoryArea, value);
        pTask->m8_entryEA = entryEA;
    }
}

// --- Interactive pickup entity (0xA0 bytes, task definition at 0609c074) ---

struct s_interactiveEntityC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_dataPtr;
    sSaturnPtr mC_tablePtr;
};

struct s_interactiveEntityC8 : public s_workAreaTemplateWithArg<s_interactiveEntityC8, s_interactiveEntityC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s_fieldLCSSubStruct mC_lcs;
    u8 m24_flags;
    s_3dModel m40_3dModel;
    sSaturnPtr m90_dataPtr;
    sSaturnPtr m94_tablePtr;
    fixedPoint m98_depth;
    s8 m9C_state;
    s8 m9D_subIndex;
    s8 m9E_counter;
    u8 m9F_modelMode;
    // size 0xA0

    static void Init(s_interactiveEntityC8* pThis, s_interactiveEntityC8_arg* pArg);
    static void Update(s_interactiveEntityC8* pThis);
    static void Draw(s_interactiveEntityC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 0605a0e4 — init 3D model with animation from entry data
static void interactiveEntityC8_initModel(s_interactiveEntityC8* pThis)
{
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr entryEA = pThis->m8_entryEA;
    sSaturnPtr tablePtr = pThis->m94_tablePtr;

    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    u16 modelIndex = readSaturnU16(modelDataEA);
    u16 poseOffset = readSaturnU16(modelDataEA + 2);
    u16 animOffset = readSaturnU16(tablePtr + 4);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    u32 numBones = pHierarchy->countNumberOfBones();
    sAnimationData* pAnim = animOffset ? pBundle->getAnimation(animOffset) : nullptr;
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, numBones);

    init3DModelRawData(pThis, &pThis->m40_3dModel, 0, pBundle, modelIndex, pAnim, pPose, nullptr, nullptr);
    stepAnimation(&pThis->m40_3dModel);
    pThis->m9F_modelMode = 1;
}

// 0605a3a4 — increment animation counter
static void interactiveEntityC8_tickCounter(s_interactiveEntityC8* pThis)
{
    pThis->m9E_counter++;
    if (pThis->m9E_counter > 1)
    {
        pThis->m9E_counter = 0;
        pThis->m9D_subIndex++;
        if (pThis->m9D_subIndex > 5)
        {
            pThis->m9D_subIndex = 0;
        }
    }
}

// 0605a140 — LCS callback for interactive entity (opens door/elevator)
static void interactiveEntityC8_lcsCallback(s_interactiveEntityC8* pThis)
{
    sSaturnPtr dataPtr = pThis->m90_dataPtr;
    s16 subFieldIndex = getFieldTaskPtr()->m2E_currentSubFieldIndex;

    sSaturnPtr tableBase;
    switch (subFieldIndex)
    {
    case 0:  tableBase = { 0x0608a880, gFLD_C8 }; break;
    case 6:  tableBase = { 0x0608a8d0, gFLD_C8 }; break;
    case 9:  tableBase = { 0x0608a8f0, gFLD_C8 }; break;
    case 0x17: tableBase = { 0x0608a970, gFLD_C8 }; break;
    case 0x1a: tableBase = { 0x0608a990, gFLD_C8 }; break;
    default: assert(0); return;
    }

    s8 entryIndex = readSaturnS8(dataPtr + 0xC);
    sSaturnPtr entryEA = tableBase + entryIndex * 0x10;

    s_C8_cutsceneCameraArg camArg = {};
    camArg.m0_position = readSaturnVec3(entryEA);
    camArg.mC_rotation = readSaturnS16(entryEA + 0xC);
    camArg.m10_distance = 0x3C000;
    camArg.m14_speed = 0x3C;
    camArg.m18[0] = (s32)readSaturnS8(dataPtr + 0xD);
    camArg.m18[1] = (s32)readSaturnS8(dataPtr + 0xE);
    camArg.m18[2] = 1;
    camArg.m24_flags = 1;
    camArg.m28_param = (s16)0x8000;

    FUN_FLD_C8_0605cb58(pThis, &camArg);

    s32 bitIndex = readSaturnS32(dataPtr + 4);
    if (bitIndex > 0)
    {
        mainGameState.setBit566(bitIndex);
    }

    playSystemSoundEffect(0x65);
    interactiveEntityC8_initModel(pThis);
    pThis->m9C_state = 4;
}

// 0605a270
void s_interactiveEntityC8::Init(s_interactiveEntityC8* pThis, s_interactiveEntityC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->m90_dataPtr = pArg->m8_dataPtr;
    pThis->m94_tablePtr = pArg->mC_tablePtr;

    interactiveEntityC8_initModel(pThis);

    sVec3_FP entryPos = readSaturnVec3(pThis->m8_entryEA + 4);
    initFieldLCSSubStruct(&pThis->mC_lcs, pThis, (void*)interactiveEntityC8_lcsCallback,
        &entryPos, 0, 3, 0, -1, -1, 0);

    s8 entryPointInData = readSaturnS8(pThis->m90_dataPtr + 0xC);
    s16 fieldEntryPoint = getFieldTaskPtr()->m30_fieldEntryPoint;

    if ((s32)entryPointInData == (s32)fieldEntryPoint)
    {
        pThis->m9C_state = 2;
        playSystemSoundEffect(0x65);
        s32 bitIndex = readSaturnS32(pThis->m90_dataPtr + 4);
        if (bitIndex > 0)
        {
            mainGameState.setBit566(bitIndex);
        }
    }
    else
    {
        pThis->m9F_modelMode = 0;
        pThis->m9C_state = 1;
        s32 bitIndex = readSaturnS32(pThis->m90_dataPtr + 8);
        if (bitIndex > 0)
        {
            if (!mainGameState.getBit566(bitIndex))
            {
                pThis->m24_flags |= 1;
                pThis->m9C_state = 0;
            }
        }
    }
}

// 0605a3d2
void s_interactiveEntityC8::Update(s_interactiveEntityC8* pThis)
{
    // LCS collision check/update (TODO)

    if (readSaturnS32(pThis->m90_dataPtr) != 0)
    {
        interactiveEntityC8_tickCounter(pThis);
    }

    switch (pThis->m9C_state)
    {
    case 0:
    {
        s32 bitIndex = readSaturnS32(pThis->m90_dataPtr + 8);
        if (bitIndex > 0)
        {
            if (mainGameState.getBit566(bitIndex))
            {
                pThis->m24_flags &= ~1;
                pThis->m9C_state = 1;
            }
        }
        break;
    }
    case 1:
        break;
    case 2:
    {
        s16 frame = stepAnimation(&pThis->m40_3dModel);
        s16 numFrames = pThis->m40_3dModel.m30_pCurrentAnimation ?
            pThis->m40_3dModel.m30_pCurrentAnimation->m4_numFrames : 0;
        if (frame == numFrames - 1)
        {
            u16 animOffset = readSaturnU16(pThis->m94_tablePtr + 6);
            initAnimation(&pThis->m40_3dModel,
                pThis->m0_memoryArea.m0_mainMemoryBundle->getAnimation(animOffset));
            playSystemSoundEffect(0x65);
            pThis->m9C_state = 3;
        }
        break;
    }
    case 3:
    {
        s16 frame = stepAnimation(&pThis->m40_3dModel);
        s16 numFrames = pThis->m40_3dModel.m30_pCurrentAnimation ?
            pThis->m40_3dModel.m30_pCurrentAnimation->m4_numFrames : 0;
        if (frame == numFrames - 1)
        {
            // FUN_06020024 — frees VDP1 blocks for 3D model (no-op in C++, handled by destructors)
            pThis->m9F_modelMode = 0;
            pThis->m9C_state = 1;
        }
        break;
    }
    case 4:
    {
        s16 frame = stepAnimation(&pThis->m40_3dModel);
        s16 numFrames = pThis->m40_3dModel.m30_pCurrentAnimation ?
            pThis->m40_3dModel.m30_pCurrentAnimation->m4_numFrames : 0;
        if (frame == numFrames - 1)
        {
            pThis->m9C_state = 5;
        }
        break;
    }
    }
}

// 0605a5e8
void s_interactiveEntityC8::Draw(s_interactiveEntityC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;

    sVec3_FP pos = readSaturnVec3(entryEA + 4);
    if (clipCheck_C8_frustum(&pos, graphicEngineStatus.m405C.m14_farClipDistance) != 0)
    {
        pThis->m98_depth = fixedPoint(0x270F000);
        return;
    }

    pushCurrentMatrix();
    translateCurrentMatrix(pos);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    pThis->m98_depth = pCurrentMatrix->m[2][3];

    if (pThis->m9F_modelMode == 0)
    {
        sSaturnPtr modelDataEA;
        sSaturnPtr tableBase = readSaturnEA(pThis->m94_tablePtr);
        if (tableBase.isNull())
        {
            modelDataEA = readSaturnEA(entryEA + 0x1C);
        }
        else
        {
            modelDataEA = tableBase + pThis->m9D_subIndex * 4;
        }
        LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
            readSaturnS16(modelDataEA),
            readSaturnS16(modelDataEA + 2));
    }
    else
    {
        pThis->m40_3dModel.m18_drawFunction(&pThis->m40_3dModel);
    }

    popMatrix();
}

// 0605a680
static void createInteractiveEntityC8(p_workArea gridCell, s_interactiveEntityC8_arg* pArg)
{
    createSubTaskWithArg<s_interactiveEntityC8>(gridCell, pArg);
}

// 0605a6c0
static void createCellEntity_C8_0605a6c0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C134);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C0FC);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605a6e0
static void createCellEntity_C8_0605a6e0(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C124);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C104);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605d6fc — linear search in array of 0x14-byte entries
static s32 searchEntryArray(sSaturnPtr arrayEA, s32 key, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        if (readSaturnS32(arrayEA + i * 0x14) == key)
        {
            return i;
        }
    }
    return 0;
}

// 0605a740
static void createCellEntity_C8_0605a740(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C164);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C11C);
    createInteractiveEntityC8(gridCell, &arg);
}

// --- 0x1E0-byte tower creature entity (task def at 060b28d8) ---

struct s_towerCreatureBone
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    u16 m18_modelOffset;
    u16 m1A_pad;
    // size 0x1C
};

struct s_towerCreatureC8_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_dataPtr;
    sSaturnPtr mC_tablePtr;
};

struct s_towerCreatureC8 : public s_workAreaTemplateWithArg<s_towerCreatureC8, s_towerCreatureC8_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s_fieldLCSSubStruct mC_lcs;
    sVec3_FP m40_position;
    s_3dModel m4C_3dModel;
    s_towerCreatureBone m9C_bones[4];
    // m10C: animation sub-structure data (0xA8 bytes, managed by subtasks)
    u8 m10C_animSubData[0xA8];
    sSaturnPtr m1B4_dataPtr;
    sSaturnPtr m1B8_tablePtr;
    fixedPoint m1BC_yRotation;
    fixedPoint m1C0_yRotSpeed;
    fixedPoint m1C4;
    fixedPoint m1C8_xRotSpeed;
    fixedPoint m1CC_xRotation;
    fixedPoint m1D0_velocity;
    s32 m1D4;
    u8 m1D8_counter;
    s8 m1D9_state;
    s8 m1DA_yRotSubState;
    s8 m1DB_xRotSubState;
    u8 m1DC_flags;
    // size 0x1E0

    static void Init(s_towerCreatureC8* pThis, s_towerCreatureC8_arg* pArg);
    static void Update(s_towerCreatureC8* pThis);
    static void Draw(s_towerCreatureC8* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 0605d772 — read bone transform data from model hierarchy
static void towerCreatureC8_readBoneData(s_towerCreatureC8* pThis, s_towerCreatureC8_arg* pArg, sModelHierarchy* pNode, s32* pBoneIndex)
{
    sSaturnPtr tablePtr = pArg->mC_tablePtr;
    sSaturnPtr modelOffsetTableEA = readSaturnEA(tablePtr);
    u16 poseOffset = readSaturnU16(tablePtr + 6);
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;

    do
    {
        s_towerCreatureBone* pBone = &pThis->m9C_bones[*pBoneIndex];
        pBone->m18_modelOffset = readSaturnU16(modelOffsetTableEA + *pBoneIndex * 2);

        // Read position/rotation from raw pose data in bundle (0x24 bytes per bone)
        u8* pRawPose = pBundle->getRawFileAtOffset(poseOffset) + *pBoneIndex * 0x24;
        pBone->m0_position.m0_X = READ_BE_S32(pRawPose + 0);
        pBone->m0_position.m4_Y = READ_BE_S32(pRawPose + 4);
        pBone->m0_position.m8_Z = READ_BE_S32(pRawPose + 8);
        pBone->mC_rotation.m0_X = READ_BE_S32(pRawPose + 0xC);
        pBone->mC_rotation.m4_Y = READ_BE_S32(pRawPose + 0x10);
        pBone->mC_rotation.m8_Z = READ_BE_S32(pRawPose + 0x14);
        (*pBoneIndex)++;

        if (pNode->m4_subNode)
        {
            towerCreatureC8_readBoneData(pThis, pArg, pNode->m4_subNode, pBoneIndex);
        }

        if (pNode->m8_nextNode == nullptr)
            break;

        pNode = pNode->m8_nextNode;
    } while (true);
}

// 0605de64 — init 3D model with animation for tower creature
static void towerCreatureC8_initModel(s_towerCreatureC8* pThis)
{
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr tablePtr = pThis->m1B8_tablePtr;

    u16 modelIndex = readSaturnU16(tablePtr + 4);
    u16 poseOffset = readSaturnU16(tablePtr + 6);
    u16 colorAnimOffset = readSaturnU16(tablePtr + 8);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    u32 numBones = pHierarchy->countNumberOfBones();
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, numBones);
    u8* pColorAnim = colorAnimOffset ? pBundle->getRawFileAtOffset(colorAnimOffset) : nullptr;

    init3DModelRawData(pThis, &pThis->m4C_3dModel, 0, pBundle, modelIndex, nullptr, pPose, pColorAnim, nullptr);
    stepAnimation(&pThis->m4C_3dModel);
    pThis->m1DC_flags |= 1;
}

// 0605e118 — Y rotation state machine
static void towerCreatureC8_updateYRotation(s_towerCreatureC8* pThis)
{
    pThis->m1BC_yRotation = fixedPoint((s32)pThis->m1BC_yRotation + (s32)pThis->m1C0_yRotSpeed);
    pThis->m1BC_yRotation = fixedPoint((u32)(s32)pThis->m1BC_yRotation & 0xFFFFFFF);

    switch (pThis->m1DA_yRotSubState)
    {
    case 0: break;
    case 1:
        pThis->m1C0_yRotSpeed = fixedPoint(0xC22E);
        pThis->m1DA_yRotSubState++;
        break;
    case 2:
        pThis->m1C0_yRotSpeed = fixedPoint((s32)pThis->m1C0_yRotSpeed + 1);
        pThis->m1C0_yRotSpeed = MTH_Mul(fixedPoint(0x105E3), pThis->m1C0_yRotSpeed);
        if ((s32)pThis->m1C0_yRotSpeed > 0x444444)
        {
            pThis->m1C0_yRotSpeed = fixedPoint(0x444444);
            pThis->m1DA_yRotSubState = 0;
        }
        break;
    case 3:
        if ((s32)pThis->m1BC_yRotation < 0x8000000)
            pThis->m1DA_yRotSubState++;
        break;
    case 4:
        if ((s32)pThis->m1BC_yRotation > 0x8000000)
            pThis->m1DA_yRotSubState++;
        break;
    case 5:
        pThis->m1C0_yRotSpeed = fixedPoint((s32)pThis->m1C0_yRotSpeed - ((s32)pThis->m1C0_yRotSpeed >> 5));
        if ((s32)pThis->m1C0_yRotSpeed < 0x12345)
            pThis->m1C0_yRotSpeed = fixedPoint(0x12345);
        if ((s32)pThis->m1BC_yRotation < 0x8000000)
        {
            pThis->m1BC_yRotation = fixedPoint(0);
            pThis->m1C0_yRotSpeed = fixedPoint(0);
            pThis->m1DA_yRotSubState = 0;
        }
        break;
    }
}

// 0605e222 — X rotation state machine
static void towerCreatureC8_updateXRotation(s_towerCreatureC8* pThis)
{
    pThis->m1C4 = fixedPoint((s32)pThis->m1C4 + (s32)pThis->m1C8_xRotSpeed);
    pThis->m1C4 = fixedPoint((u32)(s32)pThis->m1C4 & 0xFFFFFFF);

    switch (pThis->m1DB_xRotSubState)
    {
    case 0: break;
    case 1:
        pThis->m1C8_xRotSpeed = fixedPoint(0xFFFF3DD2);
        pThis->m1DB_xRotSubState++;
        break;
    case 2:
        pThis->m1C8_xRotSpeed = fixedPoint((s32)pThis->m1C8_xRotSpeed - 1);
        pThis->m1C8_xRotSpeed = MTH_Mul(fixedPoint(0x105E3), pThis->m1C8_xRotSpeed);
        if ((s32)pThis->m1C8_xRotSpeed < -0x444444)
        {
            pThis->m1C8_xRotSpeed = fixedPoint(0xFFBBBBBC);
            pThis->m1DB_xRotSubState = 0;
        }
        break;
    case 3:
        if ((s32)pThis->m1C4 > 0x8000000)
            pThis->m1DB_xRotSubState++;
        break;
    case 4:
        if ((s32)pThis->m1C4 < 0x8000000)
            pThis->m1DB_xRotSubState++;
        break;
    case 5:
        pThis->m1C8_xRotSpeed = fixedPoint((s32)pThis->m1C8_xRotSpeed - ((s32)pThis->m1C8_xRotSpeed >> 5));
        if ((s32)pThis->m1C8_xRotSpeed > -0x12345)
            pThis->m1C8_xRotSpeed = fixedPoint(0xFFFEDCBB);
        if ((s32)pThis->m1C4 > 0x8000000)
        {
            pThis->m1C4 = fixedPoint(0);
            pThis->m1C8_xRotSpeed = fixedPoint(0);
            pThis->m1DB_xRotSubState = 0;
        }
        break;
    }
}

// 0605e3c4 — draw sub-element bones (4 bones with individual transforms)
static void towerCreatureC8_drawBones(s_towerCreatureC8* pThis)
{
    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;

    translateCurrentMatrix(&pThis->m9C_bones[0].m0_position);
    rotateCurrentMatrixZYX(&pThis->m9C_bones[0].mC_rotation);
    addObjectToDrawList(pBundle->get3DModel(pThis->m9C_bones[0].m18_modelOffset));

    translateCurrentMatrix(&pThis->m9C_bones[1].m0_position);
    rotateCurrentMatrixZYX(&pThis->m9C_bones[1].mC_rotation);
    addObjectToDrawList(pBundle->get3DModel(pThis->m9C_bones[1].m18_modelOffset));

    translateCurrentMatrix(&pThis->m9C_bones[2].m0_position);
    rotateCurrentMatrixZYX(&pThis->m9C_bones[2].mC_rotation);
    rotateCurrentMatrixShiftedY(pThis->m1C4);
    addObjectToDrawList(pBundle->get3DModel(pThis->m9C_bones[2].m18_modelOffset));

    translateCurrentMatrix(&pThis->m9C_bones[3].m0_position);
    rotateCurrentMatrixZYX(&pThis->m9C_bones[3].mC_rotation);
    addObjectToDrawList(pBundle->get3DModel(pThis->m9C_bones[3].m18_modelOffset));
}

// 0605da3a
void s_towerCreatureC8::Init(s_towerCreatureC8* pThis, s_towerCreatureC8_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->m1B4_dataPtr = pArg->m8_dataPtr;
    pThis->m1B8_tablePtr = pArg->mC_tablePtr;
    pThis->m1BC_yRotation = fixedPoint(0);
    pThis->m1C0_yRotSpeed = fixedPoint(0);
    pThis->m1C4 = fixedPoint(0);
    pThis->m1C8_xRotSpeed = fixedPoint(0);
    pThis->m1CC_xRotation = fixedPoint(0xFFAAAAAA);
    pThis->m1D0_velocity = fixedPoint(0);
    pThis->m1D9_state = 0;
    pThis->m1DA_yRotSubState = 0;
    pThis->m1DB_xRotSubState = 0;

    // 0605d772
    {
        sModelHierarchy* pHierarchy = pThis->m0_memoryArea.m0_mainMemoryBundle->getModelHierarchy(readSaturnU16(pArg->mC_tablePtr + 4));
        s32 boneIndex = 0;
        towerCreatureC8_readBoneData(pThis, pArg, pHierarchy, &boneIndex);
    }

    // 0607a55a — read position from entry data
    readSaturnVec3Into(pThis->m8_entryEA + 4, &pThis->m40_position);

    // Init LCS sub-structure
    initFieldLCSSubStruct(&pThis->mC_lcs, pThis, nullptr /*0605db7e*/, &pThis->m40_position,
        0, 0, 0, -1, -1, 0);

    pThis->mC_lcs.m18 = 0;

    if ((mainGameState.bitField[0xA6] & 4) != 0)
    {
        s32 bitIndex = readSaturnS32(pThis->m1B4_dataPtr + 8);
        u32 idx = (bitIndex > 999) ? bitIndex - 566 : (u32)bitIndex;
        if (!mainGameState.getBit566(idx))
        {
            // 0605d830 — init animation sub-structures for wings
            Unimplemented(); // creates 9 trail subtasks via 0608a74e
            pThis->m1C8_xRotSpeed = fixedPoint(0xFFBBBBBC);
            pThis->m1DB_xRotSubState = 0;
            pThis->m1C0_yRotSpeed = fixedPoint(0x444444);
            pThis->m1DA_yRotSubState = 0;
            pThis->m1D9_state = 5;
        }
        else
        {
            pThis->mC_lcs.m18 |= 1;
            pThis->m1D9_state = 10;
        }
    }
}

// 0605dee8 — main 10-state machine
static void towerCreatureC8_mainStateMachine(s_towerCreatureC8* pThis)
{
    switch (pThis->m1D9_state)
    {
    case 0:
        if ((mainGameState.bitField[0xA6] & 4) != 0)
        {
            pThis->m1D9_state++;
        }
        break;
    case 1:
        pThis->mC_lcs.m18 |= 1;
        towerCreatureC8_initModel(pThis);
        Unimplemented(); // 0605d830 — init animation sub-structures (creates wing subtasks)
        pThis->m1D9_state++;
        break;
    case 2:
    {
        s16 result = func3dModelSub0(&pThis->m4C_3dModel);
        if (result == 0)
        {
            // FUN_06020024 — frees VDP1 blocks for 3D model (no-op in C++, handled by destructors)
            pThis->m1DC_flags &= ~1;
            pThis->m1DB_xRotSubState = 1;
            pThis->m1D9_state++;
        }
        break;
    }
    case 3:
        // 0606436e/06064444 — sound effect registration (needs field sound system at field_C + 0xD8)
        pThis->m1DC_flags &= ~4;
        pThis->m1D0_velocity = fixedPoint(0);
        pThis->m1D9_state++;
        // fall through
    case 4:
        pThis->m1D0_velocity = fixedPoint((s32)pThis->m1D0_velocity - 0x4DA);
        if ((s32)pThis->m1D0_velocity < -0x1E573)
            pThis->m1D0_velocity = fixedPoint(0xFFFE1A8D);
        pThis->m1CC_xRotation = fixedPoint((s32)pThis->m1CC_xRotation + (s32)pThis->m1D0_velocity);
        if ((s32)pThis->m1CC_xRotation >= -0x3777777)
            break;
        if ((pThis->m1DC_flags & 4) == 0)
        {
            // TODO: 0606436e/06064444 — sound effects (needs field sound system)
            pThis->m1DC_flags |= 4;
            // TODO: 0605dc7c — spawn particles (needs field particle system)
            pThis->m1DA_yRotSubState = 1;
            pThis->m1D0_velocity = -pThis->m1D0_velocity;
            pThis->m1D0_velocity = fixedPoint((s32)pThis->m1D0_velocity - ((s32)pThis->m1D0_velocity >> 1));
            pThis->m1CC_xRotation = fixedPoint((s32)pThis->m1D0_velocity + (s32)fixedPoint(0xFC888889));
        }
        else
        {
            pThis->m1D9_state++;
        }
        break;
    case 5:
        pThis->mC_lcs.m18 = 0;
        // TODO: 060642fc — register effect (needs field effect system)
        pThis->m1CC_xRotation = fixedPoint(0xFC888889);
        pThis->m1D9_state++;
        break;
    case 6:
        // TODO: 0605dc7c — spawn particles type 2 (needs field particle system)
        break;
    case 7:
        pThis->mC_lcs.m18 |= 1;
        pThis->m1DA_yRotSubState = 3;
        pThis->m1DB_xRotSubState = 3;
        pThis->m1D0_velocity = fixedPoint(0x1E573);
        // 0605d8ea — set animation flags on sub-structures
        {
            u8* pAnimSub = pThis->m10C_animSubData;
            pAnimSub[0x19] |= 2;
            pAnimSub[0x51] |= 2;
            pAnimSub[0x89] |= 2;
        }
        pThis->m1D9_state++;
        break;
    case 8:
        pThis->m1CC_xRotation = fixedPoint((s32)pThis->m1CC_xRotation + (s32)pThis->m1D0_velocity);
        if ((s32)pThis->m1CC_xRotation > -0xAAAAAB)
        {
            pThis->m1CC_xRotation = fixedPoint(0xFF555556);
            pThis->m1D9_state++;
        }
        break;
    case 9:
        if (pThis->m1DB_xRotSubState == 0)
        {
            pThis->m1D9_state++;
        }
        break;
    }

    if (pThis->m1D9_state > 2 && pThis->m1D9_state < 7)
    {
        // TODO: 0605d916 — particle animation update (needs field particle system)
    }
}

// 0605b6e0 — LCS collision check (variant for 0x1E0 entity)
static bool FUN_FLD_C8_0605b6e0(s_fieldLCSSubStruct* pLCS)
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    sVec3_FP* pPos = pLCS->m8_positionPtr;
    if (!pPos) return false;
    // 0605b68a — search zone table 1 for position Y
    // Compare with field_C + 0xE4 zone index
    Unimplemented(); // needs zone table search via 0605b68a
    return false;
}

// 0605e32a
void s_towerCreatureC8::Update(s_towerCreatureC8* pThis)
{
    towerCreatureC8_mainStateMachine(pThis);
    towerCreatureC8_updateYRotation(pThis);
    towerCreatureC8_updateXRotation(pThis);

    if (pThis->m1D9_state < 10)
    {
        if (FUN_FLD_C8_0605b6e0(&pThis->mC_lcs))
        {
            Unimplemented(); // 06075bd8 — LCS collision update
        }
    }
}

// 0605e448
void s_towerCreatureC8::Draw(s_towerCreatureC8* pThis)
{
    sSaturnPtr entryEA = pThis->m8_entryEA;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(entryEA + 4));

    sSaturnPtr tablePtr = pThis->m1B8_tablePtr;

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(entryEA + 0x10);
    rot[1] = readSaturnS16(entryEA + 0x12);
    rot[2] = readSaturnS16(entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    rotateCurrentMatrixShiftedY(pThis->m1BC_yRotation);

    // Main model
    sSaturnPtr modelDataEA = readSaturnEA(entryEA + 0x1C);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle,
        readSaturnS16(modelDataEA),
        readSaturnS16(modelDataEA + 2));

    // Billboard sub
    envObjectC8_DrawSub(&pThis->m0_memoryArea,
        readSaturnS16(tablePtr + 10),
        readSaturnS16(tablePtr + 0xC));

    // 3 appendages
    static const sSaturnPtr appendageDataEA = gFLD_C8->getSaturnPtr(0x060B28A8);
    for (s32 i = 0; i < 3; i++)
    {
        sSaturnPtr appendageEA = appendageDataEA + i * 0x10;

        pushCurrentMatrix();
        translateCurrentMatrix(readSaturnVec3(appendageEA));
        rotateCurrentMatrixY(readSaturnS16(appendageEA + 0xC));
        rotateCurrentMatrixShiftedX(pThis->m1CC_xRotation);

        pushCurrentMatrix();
        if ((pThis->m1DC_flags & 1) == 0)
        {
            towerCreatureC8_drawBones(pThis);
        }
        else
        {
            pThis->m4C_3dModel.m18_drawFunction(&pThis->m4C_3dModel);
        }
        popMatrix();

        envObjectC8_DrawSub(&pThis->m0_memoryArea,
            readSaturnS16(tablePtr + 0xE),
            readSaturnS16(tablePtr + 0x10));

        popMatrix();
    }

    popMatrix();
}

// 0605e362 — D4 alternate update: wait for bitfield then switch to normal update
static void towerCreatureC8_UpdateD4(s_towerCreatureC8* pThis)
{
    pThis->mC_lcs.m18 |= 2;
    // TODO: 06075bd8 collision update on mC_lcs
    if ((mainGameState.bitField[0xA6] & 4) != 0)
    {
        pThis->mC_lcs.m18 = 0;
        pThis->m_UpdateMethod = &s_towerCreatureC8::Update;
    }
}

// 0605e550 — create tower creature task
static void createTowerCreatureC8(p_workArea gridCell, s_towerCreatureC8_arg* pArg)
{
    s_towerCreatureC8* pTask = createSubTaskWithArg<s_towerCreatureC8>(gridCell, pArg);
    if (pTask && (mainGameState.bitField[0xA6] & 0x10) != 0)
    {
        pTask->m_UpdateMethod = &towerCreatureC8_UpdateD4;
    }
}

// 0605e59c
static void createCellEntity_C8_0605e59c(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s32 entryType = readSaturnS32(entryEA + 0x18);
    s8 index = (s8)searchEntryArray(gFLD_C8->getSaturnPtr(0x060B2904), entryType, 3);

    s_towerCreatureC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x060B2904) + index * 0x14;
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x060B28F0);
    createTowerCreatureC8(gridCell, &arg);
}

// 06055cd4
static void createCellEntity_C8_06055cd4(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_staticDrawC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_drawParam1 = 0x20;
    arg.mA_drawParam2 = 0x5cc;
    createSubTaskWithArg<s_staticDrawC8>(gridCell, &arg);
}

// 06059a9c — linear search in 0x10-stride entry array
static s32 searchEntryArray16(s32 key, sSaturnPtr arrayEA, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        if (readSaturnS32(arrayEA + i * 0x10) == key)
        {
            return i;
        }
    }
    return 0;
}

// --- 0xB4-byte entity (task definition at 0609bf5c) ---

struct s_entityC8_B4_arg
{
    sSaturnPtr m0_entryEA;
    s32 m4_areaIndex;
    sSaturnPtr m8_tablePtr;
    sSaturnPtr mC_dataPtr;
    sSaturnPtr m10_extraPtr;
};

// 06075b84
static void initFieldLCSSubStruct(s_fieldLCSSubStruct* pLCS, p_workArea owner, void* callback,
    sVec3_FP* posPtr, s32 param5, s16 param6, s16 param7, s16 param8, u8 param9, u8 param10)
{
    pLCS->m0_owner = owner;
    pLCS->m4_callback = callback;
    pLCS->m8_positionPtr = posPtr;
    pLCS->mC = param5;
    pLCS->m12 = param7;
    pLCS->m14 = param8;
    if (param8 < 0)
        pLCS->m16 = 0;
    else
        pLCS->m16 = param9;
    pLCS->m10 = param6;
    pLCS->m17 = param10;
    pLCS->m18 = 0;
    pLCS->m19 = 0;
    pLCS->m1A = 0;
    pLCS->m1B = 0;
    pLCS->m1C = 0;
    pLCS->m20 = 0;
}

struct s_entityC8_B4 : public s_workAreaTemplateWithArg<s_entityC8_B4, s_entityC8_B4_arg*>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_entryEA;
    s_3dModel mC_3dModel;
    p_workArea m5C_childTask;
    s_fieldLCSSubStruct m60_lcs;
    u8 m78_flags;
    sVec3_FP m94_position;
    sSaturnPtr mA0_tablePtr;
    sSaturnPtr mA4_dataPtr;
    sSaturnPtr mA8_extraPtr;
    fixedPoint mAC_depth;
    s8 mB0_state;
    // size 0xB4

    static void Init(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg);
    static void Update(s_entityC8_B4* pThis);
    static void Draw(s_entityC8_B4* pThis);

    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }
};

// 06059c38 — kill child env object task
static void entityC8_B4_killChild(s_entityC8_B4* pThis)
{
    if (pThis->m5C_childTask != nullptr)
    {
        if (pThis->m5C_childTask != nullptr)
        {
            pThis->m5C_childTask->getTask()->m14_flags |= 1;
        }
        pThis->m5C_childTask = nullptr;
    }
}

// 06059d20 — init 3D model in idle/static state
static void entityC8_B4_initIdle(s_entityC8_B4* pThis)
{
    entityC8_B4_killChild(pThis);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    u16 modelIndex = readSaturnU16(extraPtr);
    u16 poseOffset = readSaturnU16(extraPtr + 2);
    u16 colorAnimOffset = readSaturnU16(extraPtr + 12);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pHierarchy->countNumberOfBones());
    u8* pColorAnim = colorAnimOffset ? pBundle->getRawFileAtOffset(colorAnimOffset) : nullptr;

    init3DModelRawData(pThis, &pThis->mC_3dModel, 0, pBundle, modelIndex, nullptr, pPose, pColorAnim, nullptr);
    stepAnimation(&pThis->mC_3dModel);
    func3dModelSub0(&pThis->mC_3dModel);
}

// 06059d74 — activate entity (start animation)
static void entityC8_B4_activate(s_entityC8_B4* pThis)
{
    entityC8_B4_killChild(pThis);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    u16 modelIndex = readSaturnU16(extraPtr + 4);
    u16 animOffset = readSaturnU16(extraPtr + 6);
    u16 poseOffset = readSaturnU16(extraPtr + 8);
    u16 colorAnimOffset = readSaturnU16(extraPtr + 10);

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelIndex);
    u32 numBones = pHierarchy->countNumberOfBones();
    sAnimationData* pAnim = animOffset ? pBundle->getAnimation(animOffset) : nullptr;
    sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, numBones);
    u8* pColorAnim = colorAnimOffset ? pBundle->getRawFileAtOffset(colorAnimOffset) : nullptr;

    init3DModelRawData(pThis, &pThis->mC_3dModel, 0, pBundle, modelIndex, pAnim, pPose, pColorAnim, nullptr);
    stepAnimation(&pThis->mC_3dModel);
    func3dModelSub0(&pThis->mC_3dModel);
    pThis->m78_flags |= 1;
}

// 06059c88 — normal mode init (creates child env object + LCS collision)
static void entityC8_B4_initNormal(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg)
{
    entityC8_B4_killChild(pThis);

    // Create child animated env object
    s_envObjectC8_arg envArg;
    envArg.m0_entryEA = pArg->m0_entryEA;
    envArg.m4_areaIndex = pArg->m4_areaIndex;
    envArg.m8_modelFrameDataEA = pThis->mA0_tablePtr;
    envArg.mC_speed = 0x8000;
    envArg.m10_groupCount = 6;
    pThis->m5C_childTask = createSubTaskWithArg<s_envObjectC8>(pThis, &envArg);

    // Compute LCS position from entry transform + offset vector
    static const sVec3_FP offsetVec = { fixedPoint(0), fixedPoint(0), fixedPoint(0x93333) };
    sMatrix4x3 mat;
    initMatrixToIdentity(&mat);
    translateMatrix(readSaturnVec3(pThis->m8_entryEA + 4), &mat);

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(pThis->m8_entryEA + 0x10);
    rot[1] = readSaturnS16(pThis->m8_entryEA + 0x12);
    rot[2] = readSaturnS16(pThis->m8_entryEA + 0x14);
    rotateMatrixZYX_s16(rot, &mat);
    transformAndAddVec(offsetVec, pThis->m94_position, mat);

    // Init LCS sub-structure
    initFieldLCSSubStruct(&pThis->m60_lcs, pThis, nullptr /*DAT_06059ae0*/, &pThis->m94_position,
        0, 0, 0, -1, -1, 0);
    pThis->m78_flags = 0;
}

// 06059f60 — alternate update (idle animation only)
static void entityC8_B4_updateAlt(s_entityC8_B4* pThis)
{
    func3dModelSub0(&pThis->mC_3dModel);
}

// 06059e14
void s_entityC8_B4::Init(s_entityC8_B4* pThis, s_entityC8_B4_arg* pArg)
{
    getMemoryArea(&pThis->m0_memoryArea, pArg->m4_areaIndex);
    pThis->m8_entryEA = pArg->m0_entryEA;
    pThis->mA0_tablePtr = pArg->m8_tablePtr;
    pThis->mA4_dataPtr = pArg->mC_dataPtr;
    pThis->mA8_extraPtr = pArg->m10_extraPtr;
    pThis->m5C_childTask = nullptr;

    bool isAlternateMode = (mainGameState.bitField[0xA6] & 8) != 0;
    if (!isAlternateMode)
    {
        entityC8_B4_initNormal(pThis, pArg);
    }
    else
    {
        entityC8_B4_initIdle(pThis);
        pThis->m_UpdateMethod = (void(*)(s_entityC8_B4*))entityC8_B4_updateAlt;
    }
    pThis->mB0_state = isAlternateMode ? 1 : 0;
}

// 06059e70
void s_entityC8_B4::Update(s_entityC8_B4* pThis)
{
    // 0605b6b6/06075bd8 — LCS collision check/update (depends on field-specific zone data)
    // TODO: implement when field zone system (field_C + 0xDC/0xE0) is understood

    switch (pThis->mB0_state)
    {
    case 0:
        if ((mainGameState.bitField[0xA6] & 8) != 0)
        {
            entityC8_B4_activate(pThis);
            playSystemSoundEffect(0x66);
            pThis->mB0_state = 2;
        }
        break;
    case 1:
        pThis->m_UpdateMethod = (void(*)(s_entityC8_B4*))entityC8_B4_updateAlt;
        break;
    case 2:
    {
        s16 frame = stepAnimation(&pThis->mC_3dModel);
        if (frame == 0x24)
        {
            pThis->mB0_state = 3;
        }
        break;
    }
    case 3:
    {
        stepAnimation(&pThis->mC_3dModel);
        s16 result = func3dModelSub0(&pThis->mC_3dModel);
        if (result == 0)
        {
            u16 vertAnimOffset = readSaturnU16(pThis->mA8_extraPtr + 12);
            update3dModelDrawFunctionForVertexAnimation(
                &pThis->mC_3dModel,
                pThis->m0_memoryArea.m0_mainMemoryBundle->getRawFileAtOffset(vertAnimOffset));
            pThis->mB0_state = 4;
        }
        break;
    }
    case 4:
    {
        func3dModelSub0(&pThis->mC_3dModel);
        s16 frame = stepAnimation(&pThis->mC_3dModel);
        if (frame == 0)
        {
            entityC8_B4_initIdle(pThis);
            pThis->mB0_state = 1;
        }
        break;
    }
    }
}

// 06059f6c
void s_entityC8_B4::Draw(s_entityC8_B4* pThis)
{
    if (pThis->mB0_state == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(readSaturnVec3(pThis->m8_entryEA + 4));

    sSaturnPtr extraPtr = pThis->mA8_extraPtr;

    sVec3_S16_12_4 rot;
    rot[0] = readSaturnS16(pThis->m8_entryEA + 0x10);
    rot[1] = readSaturnS16(pThis->m8_entryEA + 0x12);
    rot[2] = readSaturnS16(pThis->m8_entryEA + 0x14);
    rotateCurrentMatrixZYX_s16(rot);

    pThis->mAC_depth = pCurrentMatrix->m[2][3];
    pThis->mC_3dModel.m18_drawFunction(&pThis->mC_3dModel);
    envObjectC8_DrawSub(&pThis->m0_memoryArea,
        readSaturnS16(extraPtr + 0xE),
        readSaturnS16(extraPtr + 0x10));

    popMatrix();
}

// 06059fc8
static void createEntityC8_B4(p_workArea gridCell, s_entityC8_B4_arg* pArg)
{
    createSubTaskWithArg<s_entityC8_B4>(gridCell, pArg);
}

// 0605a008
static void createCellEntity_C8_0605a008(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s32 entryType = readSaturnS32(entryEA + 0x18);
    s32 index = searchEntryArray16(entryType, gFLD_C8->getSaturnPtr(0x0609BFB8), 2);

    s_entityC8_B4_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_tablePtr = gFLD_C8->getSaturnPtr(0x0609BF9C);
    arg.mC_dataPtr = gFLD_C8->getSaturnPtr(0x0609BFB8) + index * 0x10;
    arg.m10_extraPtr = gFLD_C8->getSaturnPtr(0x0609BFA4);
    createEntityC8_B4(gridCell, &arg);
}

// 0605a700
static void createCellEntity_C8_0605a700(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C154);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C10C);
    createInteractiveEntityC8(gridCell, &arg);
}

// 0605a720
static void createCellEntity_C8_0605a720(p_workArea gridCell, sSaturnPtr entryEA, s32 value)
{
    s_interactiveEntityC8_arg arg;
    arg.m0_entryEA = entryEA;
    arg.m4_areaIndex = value;
    arg.m8_dataPtr = gFLD_C8->getSaturnPtr(0x0609C144);
    arg.mC_tablePtr = gFLD_C8->getSaturnPtr(0x0609C114);
    createInteractiveEntityC8(gridCell, &arg);
}

// 06079dfe — dispatch per-cell entity creation from function pointer list
static void dispatchCellEntities_C8(p_workArea gridCell, sSaturnPtr cellDataEA, s32 value)
{
    sSaturnPtr entry = cellDataEA;
    while (true)
    {
        u32 funcAddr = readSaturnU32(entry);
        if (funcAddr == 0) break;

        switch (funcAddr)
        {
        case 0x0605B270: createEnvObject_C8_type0(gridCell, entry, value); break;
        case 0x0605B294: createEnvObject_C8_type0b(gridCell, entry, value); break;
        case 0x0605B2B8: createEnvObject_C8_type1(gridCell, entry, value); break;
        case 0x0605B2DC: createEnvObject_C8_type1b(gridCell, entry, value); break;
        case 0x0605B300: createEnvObject_C8_type2(gridCell, entry, value); break;
        case 0x0605B324: createEnvObject_C8_type2b(gridCell, entry, value); break;
        case 0x06055CD4: createCellEntity_C8_06055cd4(gridCell, entry, value); break;
        case 0x06055CF4: createCellEntity_C8_06055cf4(gridCell, entry, value); break;
        case 0x06055D14: createCellEntity_C8_06055d14(gridCell, entry, value); break;
        case 0x0605A008: createCellEntity_C8_0605a008(gridCell, entry, value); break;
        case 0x0605A6C0: createCellEntity_C8_0605a6c0(gridCell, entry, value); break;
        case 0x0605A6E0: createCellEntity_C8_0605a6e0(gridCell, entry, value); break;
        case 0x0605A700: createCellEntity_C8_0605a700(gridCell, entry, value); break;
        case 0x0605A720: createCellEntity_C8_0605a720(gridCell, entry, value); break;
        case 0x0605A740: createCellEntity_C8_0605a740(gridCell, entry, value); break;
        case 0x0605E59C: createCellEntity_C8_0605e59c(gridCell, entry, value); break;
        case 0x0607A414: createCellEntity_C8_0607a414(gridCell, entry, value); break;
        default:
            PDS_Log("C8: unknown cell entity func 0x%08X", funcAddr);
            break;
        }

        entry = entry + 0x20; // stride 32 bytes
    }
}

// 06079e26 — setup field with data tables and per-cell visibility registration
void setupFieldC8(sSaturnPtr dataTable3EA, sSaturnPtr visibilityEA, void(*createFunc)(p_workArea))
{
    s_DataTable3* pDT3 = readDataTable3(dataTable3EA);
    setupField2(pDT3, createFunc);

    // Per-cell visibility: visibilityEA points to {ptrArray, value, width, height}
    // Iterates width*height cells, calling per-cell function pointer lists
    if (!visibilityEA.isNull())
    {
        s32 width = readSaturnS32(visibilityEA + 8);
        s32 height = readSaturnS32(visibilityEA + 12);
        sSaturnPtr ptrArray = readSaturnEA(visibilityEA);
        s32 value = readSaturnS32(visibilityEA + 4);

        s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
        s32 count = width * height;
        for (s32 i = 0; i < count; i++)
        {
            sSaturnPtr cellDataEA = readSaturnEA(ptrArray + i * 4);
            if (!cellDataEA.isNull())
            {
                // 06079dfe — iterate function pointer list for this cell
                dispatchCellEntities_C8(pGrid->m3C_cellRenderingTasks[i], cellDataEA, value);
            }
        }
    }
}

// 06055bb4 — camera config
static void setupCameraConfig_C8_0()
{
    setupFieldCameraConfigs(readCameraConfig(gFLD_C8->getSaturnPtr(0x0608AF64)), 1);
}

// 06078784 — tower clipping: always visible (returns 0)
static s32 clipCheck_C8_alwaysVisible(const sVec3_FP* r4, s32 r5)
{
    return 0;
}

// 0606a5c2 — get camera view matrix
static sMatrix4x3* getFieldCameraMatrix_C8()
{
    return &getFieldTaskPtr()->m8_pSubFieldData->m334->m384_viewMatrix;
}

// 060788dc — frustum culling check
static s32 clipCheck_C8_frustum(const sVec3_FP* pPos, s32 farClip)
{
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    // Manhattan distance check against camera
    fixedPoint dx = pPos->m0_X - cameraProperties2.m0_position.m0_X;
    if ((s32)dx < 0) dx = -dx;
    if ((s32)dx > farClip) return 1;

    fixedPoint dy = pPos->m4_Y - cameraProperties2.m0_position.m4_Y;
    if ((s32)dy < 0) dy = -dy;
    if ((s32)dy > farClip) return 1;

    fixedPoint dz = pPos->m8_Z - cameraProperties2.m0_position.m8_Z;
    if ((s32)dz < 0) dz = -dz;
    if ((s32)dz > farClip) return 1;

    // Offset camera position along view direction
    sMatrix4x3* pViewMat = getFieldCameraMatrix_C8();
    sVec3_FP camPos;
    camPos.m0_X = cameraProperties2.m0_position.m0_X + pViewMat->m[0][2] * -0x20;
    camPos.m4_Y = cameraProperties2.m0_position.m4_Y + pViewMat->m[1][2] * -0x20;
    camPos.m8_Z = cameraProperties2.m0_position.m8_Z + pViewMat->m[2][2] * -0x20;

    // Delta from offset camera to object
    sVec3_FP delta;
    delta.m0_X = pPos->m0_X - camPos.m0_X;
    delta.m4_Y = pPos->m4_Y - camPos.m4_Y;
    delta.m8_Z = pPos->m8_Z - camPos.m8_Z;

    // Check 4 frustum planes
    if (dot3_FP(&pGrid->m12AC, &delta) <= 0) return 1;
    if (dot3_FP(&pGrid->m12B8, &delta) <= 0) return 1;
    if (dot3_FP(&pGrid->m12C4, &delta) <= 0) return 1;
    if (dot3_FP(&pGrid->m12D0, &delta) <= 0) return 1;

    return 0;
}

// 06078a1c — frustum clip check wrapper
static s32 clipCheck_C8_wrapper(const sVec3_FP* pPos)
{
    return clipCheck_C8_frustum(pPos, graphicEngineStatus.m405C.m14_farClipDistance);
}

// 06078a26 — set clipping function on visibility grid
static void setClipFunction_C8()
{
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m12FC_isObjectClipped = clipCheck_C8_alwaysVisible;
}

// 06054210 — clip distances and overlay init
static void initClipAndOverlay_C8(p_workArea workArea)
{
    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);
    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;
    setClipFunction_C8();
    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 2;
    fieldRadar_setEncounterDistance(fixedPoint(0x12C000));
}

// 06057f1a — set field-specific data flags
static void setFieldDataFlags_C8()
{
    s_fieldSpecificData_C8* pFieldData = (s_fieldSpecificData_C8*)getFieldTaskPtr()->mC;
    if (pFieldData)
    {
        pFieldData->m25 = 0;
        pFieldData->m27 = 1;
    }
}

// 0605cbec — empty function
static void emptyFunc_C8() {}

// 0605e678 — push dragon away from tower center if too close
static void dragonTowerPushback(s_dragonTaskWorkArea* pDragon, sSaturnPtr towerCenterEA, fixedPoint minDistance)
{
    sVec3_FP towerCenter;
    towerCenter.m0_X = readSaturnS32(towerCenterEA);
    towerCenter.m4_Y = pDragon->m8_pos.m4_Y;
    towerCenter.m8_Z = readSaturnS32(towerCenterEA + 8);

    fixedPoint dist = vecDistance(pDragon->m8_pos, towerCenter);
    fixedPoint pushDist = dist - minDistance;

    fixedPoint pushX, pushZ;
    if ((s32)pushDist < 0)
    {
        pushX = 0;
        pushZ = 0;
    }
    else
    {
        fixedPoint diffX = pDragon->m8_pos.m0_X - towerCenter.m0_X;
        fixedPoint diffZ = pDragon->m8_pos.m8_Z - towerCenter.m8_Z;
        pushX = performDivision(dist, MTH_Mul(pushDist, diffX));
        pushZ = performDivision(dist, MTH_Mul(pushDist, diffZ));
    }

    pDragon->m160_deltaTranslation[0] = pDragon->m160_deltaTranslation[0] - pushX;
    // m164 unchanged
    pDragon->m160_deltaTranslation[2] = pDragon->m160_deltaTranslation[2] - pushZ;
    pDragon->m3C_targetAngles[0] = pDragon->m20_angle[0]; // only pitch target, NOT yaw/roll
}

// 0605e7be — tower-specific dragon update
static void dragonUpdateC8(s_dragonTaskWorkArea* pDragon)
{
    dragonTowerPushback(pDragon, gFLD_C8->getSaturnPtr(0x060B29B4), 0xA1000);
}

// 0607d684 — setupDragonPosition from Saturn data
static void setupDragonPositionFromSaturn(sSaturnPtr posEA, sSaturnPtr rotEA)
{
    sVec3_FP pos = { readSaturnS32(posEA), readSaturnS32(posEA + 4), readSaturnS32(posEA + 8) };
    sVec3_FP rot = { readSaturnS32(rotEA), readSaturnS32(rotEA + 4), readSaturnS32(rotEA + 8) };
    setupDragonPosition(&pos, &rot);
}

// 0607a55a — copy vec3 from Saturn data
static void readSaturnVec3Into(sSaturnPtr src, sVec3_FP* dst)
{
    dst->m0_X = readSaturnS32(src);
    dst->m4_Y = readSaturnS32(src + 4);
    dst->m8_Z = readSaturnS32(src + 8);
}

// 060542e6 — cutscene start based on entry point
static void startCutsceneFromEntryPoint_C8(sSaturnPtr data)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    s32 entryPoint = pFieldTask->m30_fieldEntryPoint;

    if (entryPoint == -1)
        return;

    if ((mainGameState.bitField[0xA6] & 0x10) != 0)
    {
        if ((pFieldTask->m2E_currentSubFieldIndex == 6 && (mainGameState.bitField[0x88] & 0x10) == 0) ||
            (pFieldTask->m2E_currentSubFieldIndex == 0x17 && (mainGameState.bitField[0x87] & 0x10) == 0))
        {
            return;
        }
        if (entryPoint == 0xB)
        {
            return;
        }
    }

    sSaturnPtr entryData = data + entryPoint * 0x10;

    s_C8_cutsceneCameraArg camArg = {};
    readSaturnVec3Into(entryData, &camArg.m0_position);
    camArg.mC_rotation = readSaturnS16(entryData + 0xC);
    camArg.m10_distance = 0x3C000;
    camArg.m14_speed = 0x3C;
    camArg.m24_flags = 0;
    camArg.m28_param = (s16)0x8000;

    // Subfield 9 with entry points 1-6: use closer camera
    pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2E_currentSubFieldIndex == 9 &&
        (entryPoint >= 1 && entryPoint <= 6))
    {
        camArg.m10_distance = 0x19000;
        camArg.m14_speed = 0x27;
    }

    // 0605cb58
    s_visibilityGridWorkArea* pGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;
    FUN_FLD_C8_0605cb58(pGrid->m38, &camArg);
}

// 0607baa0 — random battle init
static void initRandomBattle_C8(s32 scriptIndex)
{
    fieldRadar_initRandomBattle(scriptIndex);
}

// 06054460
void subfieldC8_0(p_workArea workArea)
{
    createVdp2Task_C8_0(workArea);
    setupFieldC8(gFLD_C8->getSaturnPtr(0x0608FDE4), gFLD_C8->getSaturnPtr(0x0608F414), fieldC8_0_startTasks);
    adjustVerticalLimits(0x6C000, 0xF91000);
    setupCameraConfig_C8_0();
    initClipAndOverlay_C8(workArea);
    setFieldDataFlags_C8();
    emptyFunc_C8();
    getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->mF4 = dragonUpdateC8;
    setupDragonPositionFromSaturn(gFLD_C8->getSaturnPtr(0x0608A868), gFLD_C8->getSaturnPtr(0x0608A874));

    if (getFieldTaskPtr()->m30_fieldEntryPoint < 0)
    {
        setupDragonPositionFromSaturn(gFLD_C8->getSaturnPtr(0x0608AA00), gFLD_C8->getSaturnPtr(0x0608AA0C));
    }
    else
    {
        startCutsceneFromEntryPoint_C8(gFLD_C8->getSaturnPtr(0x0608A880));
    }

    initRandomBattle_C8(0x32);
}
